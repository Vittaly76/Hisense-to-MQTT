// #define HOST_NAME "TestHisense"
// #define USE_MDNS true
// #define USE_ARDUINO_OTA true



//#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <MQTT.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.
#include <GParser.h>
#include <TimerMs.h>

// #ifdef USE_MDNS
// #include <DNSServer.h>
// #include <ESP8266mDNS.h>
// #endif
// #include "RemoteDebug.h"

#include "102_00_status.h"

StaticJsonDocument<200> doc;

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "HisenseTest";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "18031976";

#define STRING_LEN 250

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "HisenseAdapter v1"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN 4

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
//#define STATUS_PIN 2
#define STATUS_PIN 16

#define MQTT_TOPIC_PREFIX "devices/"

// -- Method declarations.
void handleRoot();
void mqttMessageReceived(String &topic, String &payload);
bool connectMqtt();
bool connectMqttOptions();
// -- Callback methods.
void wifiConnected();
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);

DNSServer dnsServer;
WebServer server(80);
WiFiClient net;
MQTTClient mqttClient(256);

char mqttServerValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
// -- You can also use namespace formats e.g.: iotwebconf::ParameterGroup
IotWebConfParameterGroup mqttGroup = IotWebConfParameterGroup("mqtt", "MQTT configuration");
IotWebConfTextParameter mqttServerParam = IotWebConfTextParameter("MQTT server", "mqttServer", mqttServerValue, STRING_LEN);
IotWebConfTextParameter mqttUserNameParam = IotWebConfTextParameter("MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN);
IotWebConfPasswordParameter mqttUserPasswordParam = IotWebConfPasswordParameter("MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN);

bool needMqttConnect = false;
bool needReset = false;
//int pinState = HIGH;
unsigned long lastReport = 0;
unsigned long lastMqttConnectionAttempt = 0;

char mqttActionTopic[STRING_LEN];
char mqttStatusTopic[STRING_LEN];
char mqttAliveTopic[STRING_LEN];

// RemoteDebug Debug;
// // SSID and password
// const char* ssid = "Home";
// const char* password = "0016e688";

const int BUFFER_SIZE = 170;
char buf[BUFFER_SIZE];  //буфер входящих ответов
char packet_102_00_rec[62]; //буфер входящих пакетов типа 102_00

Device_Status hisenseStatus; // экземпляр структуры ответа 102_00
PDevice_Status pData;  //указатель на структуру ответа 102_00 в буфере входящих пакетов

byte transmitt_byte[50] = { 0 }; //массив отправляемого пакета
byte request_status_byte[] = {0xF4, 0xF5, 0x00, 0x40, 0x0C, 0x00, 0x00, 0x01, 0x01, 0xFE, 0x01, 0x00, 0x00, 0x66, 0x00, 0x00, 0x00, 0x01, 0xB3, 0xF4, 0xFB}; //массив пакета-запроса статуса 21 байт

TimerMs tmr(5000, 0, 0);  //таймер отправки пакета-запроса статуса (период, мс), (0 не запущен / 1 запущен), (режим: 0 период / 1 таймер)
TimerMs tmr2(15000, 0, 0);  //таймер перевода статуса alive в offline (период, мс), (0 не запущен / 1 запущен), (режим: 0 период / 1 таймер)

int isAlive = 0;

//расчёт контрольной суммы
uint16_t getHash(byte* data, int length) {
  uint16_t hash = 0;
  int i = 0;
  length = length-6;  //длина на 6 байт меньше, потому что ещё футер, хедер и чексумма
  while (length--) {
    hash += *(data + 2 + i);  //отбрасываем футер 2 байта
    i++;
  }
  return hash;
}

void hash_adder() {
  uint16_t send_hash = getHash((byte*)&transmitt_byte, 50);
  //Serial.print("Check sum: ");
  //Serial.println(send_hash, HEX);
  byte* ptrN2 = (byte*)&send_hash;
        transmitt_byte[47]=*ptrN2;
        transmitt_byte[46]=*(ptrN2 + 1);
}

//выделяем полезный полученный пакет 102_00
void get102_00_packet() {
  for (int i=0; i<62; i++) {
    packet_102_00_rec[i] = buf[i+16];
  }
  // for (int i=0; i<62; i++) {
  // Serial.print(packet_102_00_rec[i], HEX);
  // Serial.print(" ");
  // }
  // Serial.println(" ");
}

void parser_sender_102_00() {
  String mqttOutput;
  get102_00_packet();
  pData = ( PDevice_Status ) packet_102_00_rec;
  // Serial.print("wind_status: ");
  // Serial.println(pData->wind_status);

  // Serial.print("sleep_status: ");
  // Serial.println(pData->sleep_status);

  // Serial.print("mode_status: ");
  // Serial.println(pData->mode_status, BIN);

  // Serial.print("run_status: ");
  // Serial.println(pData->run_status, BIN);

  // Serial.print("direction_status: ");
  // Serial.println(pData->direction_status);
  
  //mode_status:4; run_status:2; direction_status:2;
  // Serial.print("mode_status: ");
  // Serial.println(((pData->mode_run_direction_status) & 0b11110000)>>4); 

  // Serial.print("run_status: ");
  // Serial.println(((pData->mode_run_direction_status) & 0b00001100)>>2, BIN);

  // Serial.print("direction_status: ");
  // Serial.println(((pData->mode_run_direction_status) & 0b00000011)>>0, BIN); 

  // Serial.print("indoor_temperature_setting: ");
  // Serial.println(pData->indoor_temperature_setting);

  // Serial.print("indoor_temperature_status: ");
  // Serial.println(pData->indoor_temperature_status);

  // Serial.print("up_down: ");
  // Serial.println(pData->up_down);

  // Serial.print("left_right: ");
  // Serial.println(pData->left_right);

  // Serial.print("20th Byte: ");
  // Serial.println((pData->binary_byte_20), BIN);
  
  // Serial.print("up_down: ");
  // Serial.println(((pData->binary_byte_20) & 0b10000000)>>7);

  // Serial.print("left_right: ");
  // Serial.println(((pData->binary_byte_20) & 0b01000000)>>6);

  // Serial.print("outdoor_temperature: ");
  // Serial.println(pData->outdoor_temperature);

  // Serial.println("---------------------------------------------------");
  // Serial.println(" ");

//пакуем в JSON
  doc["wind_status"] = pData->wind_status;
  doc["sleep_status"] = pData->sleep_status;
  doc["mode_status"] = ((pData->mode_run_direction_status) & 0b11110000)>>4;
  doc["run_status"] = ((pData->mode_run_direction_status) & 0b00001100)>>2;
  doc["direction_status"] = ((pData->mode_run_direction_status) & 0b00000011)>>0;
  yield();
  doc["indoor_temperature_setting"] = pData->indoor_temperature_setting;
  doc["indoor_temperature_status"] = pData->indoor_temperature_status;
  doc["up_down"] = ((pData->binary_byte_20) & 0b10000000)>>7;
  doc["left_right"] = ((pData->binary_byte_20) & 0b01000000)>>6;
  doc["outdoor_temperature"] = pData->outdoor_temperature;
  serializeJson(doc, mqttOutput);

  //Serial.println(mqttOutput);
  yield();
  mqttClient.publish(mqttStatusTopic, mqttOutput);
  yield();
  //mqttClient.publish("MyHisenseTest/Status", "Test mqtt sender");
}

//подготовка шаблона отправляемого пакета
void send_packet_prepare () {
  transmitt_byte[0] = 0xf4; //футер f4f5
  transmitt_byte[1] = 0xf5;
  transmitt_byte[2] = 0x00; //флаг запроса
  transmitt_byte[3] = 0x40; //служебный
  transmitt_byte[4] = 0x29; //длина пакета
  transmitt_byte[5] = 0x00;  //преамбула 8 байт
  transmitt_byte[6] = 0x00;
  transmitt_byte[7] = 0x01;
  transmitt_byte[8] = 0x01;
  transmitt_byte[9] = 0xfe;
  transmitt_byte[10] = 0x01;
  transmitt_byte[11] = 0x00;
  transmitt_byte[12] = 0x00; //конец преамбулы
  transmitt_byte[13] = 0x65; //тип запроса
  transmitt_byte[14] = 0x00; //тип запроса
  transmitt_byte[15] = 0x00; //флаг запроса
  transmitt_byte[16] = 0x00; //полезный пакет 29 байт
  transmitt_byte[17] = 0x00;
  transmitt_byte[18] = 0x00;
  transmitt_byte[19] = 0x00;
  transmitt_byte[20] = 0x00;
  transmitt_byte[21] = 0x00;
  transmitt_byte[22] = 0x00;
  transmitt_byte[23] = 0x00;
  transmitt_byte[24] = 0x00;
  transmitt_byte[25] = 0x00;
  transmitt_byte[26] = 0x00;
  transmitt_byte[27] = 0x00;
  transmitt_byte[28] = 0x00;
  transmitt_byte[29] = 0x00;
  transmitt_byte[30] = 0x00;
  transmitt_byte[31] = 0x00;
  transmitt_byte[32] = 0x00;
  transmitt_byte[33] = 0x00;
  transmitt_byte[34] = 0x00;
  transmitt_byte[35] = 0x00;
  transmitt_byte[36] = 0x00;
  transmitt_byte[37] = 0x00;
  transmitt_byte[38] = 0x00;
  transmitt_byte[39] = 0x00;
  transmitt_byte[40] = 0x00;
  transmitt_byte[41] = 0x00;
  transmitt_byte[42] = 0x00;
  transmitt_byte[43] = 0x00;
  transmitt_byte[44] = 0x00;
  transmitt_byte[45] = 0x00;
  transmitt_byte[46] = 0x00; //2 байта контрольной суммы
  transmitt_byte[47] = 0x00;
  transmitt_byte[48] = 0xf4;  //окончание
  transmitt_byte[49] = 0xfb; //окончание
}



void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  Serial.setTimeout(50);

  mqttGroup.addItem(&mqttServerParam);
  mqttGroup.addItem(&mqttUserNameParam);
  mqttGroup.addItem(&mqttUserPasswordParam);

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addParameterGroup(&mqttGroup);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);

  // -- Initializing the configuration.
  bool validConfig = iotWebConf.init();
  if (!validConfig)
  {
    mqttServerValue[0] = '\0';
    mqttUserNameValue[0] = '\0';
    mqttUserPasswordValue[0] = '\0';
  }

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  // -- Prepare dynamic topic names
  String temp = String(MQTT_TOPIC_PREFIX);
  temp += iotWebConf.getThingName();
  temp += "/action";
  temp.toCharArray(mqttActionTopic, STRING_LEN);
  temp = String(MQTT_TOPIC_PREFIX);
  temp += iotWebConf.getThingName();
  temp += "/status";
  temp.toCharArray(mqttStatusTopic, STRING_LEN);
  temp = String(MQTT_TOPIC_PREFIX);
  temp += iotWebConf.getThingName();
  temp += "/alive";
  temp.toCharArray(mqttAliveTopic, STRING_LEN); 

  mqttClient.begin(mqttServerValue, net);
  mqttClient.onMessage(mqttMessageReceived);

  //void setWill(const char topic[], const char payload[], bool retained, int qos);
  mqttClient.setWill(mqttAliveTopic, "offline", true, 1);



  // WiFi.begin(ssid, password);
  
  // // Wait for connection
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  // }  
  // String hostNameWifi = HOST_NAME;
  // hostNameWifi.concat(".local");
  // WiFi.hostname(hostNameWifi);
  // #ifdef USE_MDNS  // Use the MDNS ?

  //     if (MDNS.begin(HOST_NAME)) {      
  //     }
  //     MDNS.addService("telnet", "tcp", 23);
  // #endif

  // 	// Initialize RemoteDebug
	// Debug.begin(HOST_NAME); // Initialize the WiFi server
  // Debug.setResetCmdEnabled(true); // Enable the reset command
	// Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
	// Debug.showColors(true); // Colors
}

void loop() {
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
  mqttClient.loop();
  

  if (needMqttConnect)
  {
    if (connectMqtt())
    {
      needMqttConnect = false;
    }
  }
  else if ((iotWebConf.getState() == iotwebconf::OnLine) && (!mqttClient .connected()))
  {
    //Serial.println("MQTT reconnect");
    connectMqtt();
  }

  if (needReset)
  {
    //Serial.println("Rebooting after 1 second.");
    iotWebConf.delay(1000);
    ESP.restart();
  }

  // check if data is available
  if (Serial.available() > 0) {
    // read the incoming bytes:
    int rlen = Serial.readBytes(buf, BUFFER_SIZE);    

    // prints the received data
    // Serial.print("I received: ");    
    // Serial.println(rlen);
    // debugD("I received: %d", rlen);

    //Вычисляем чексумму
    uint16_t MyHash = getHash((byte*)&buf, rlen);
    // Serial.print("Checksum: ");
    // Serial.println(MyHash, HEX);
    // debugD("Checksum: %X", MyHash);
    //ПРоверяем на соответствие полученной
    uint16_t HashR;
    byte* ptrN = (byte*)&HashR;
    *ptrN = buf[rlen-3];
    *(ptrN + 1) = buf[rlen-4];
    // Serial.print("Checksum received: ");
    // Serial.println(HashR, HEX);
    if (MyHash == HashR) {
      //Serial.println("Checksum Ok");
      // Serial.print("Direction: ");
      // Serial.println(buf[15], HEX);
      if ((buf[4] == rlen-9)&&(buf[15] == 0x01)) {
        //Serial.println("Lenght and direction is ok");
        tmr2.start();
        // if (isAlive == 0) {
        //   isAlive = 1;
        //   mqttClient.publish(mqttAliveTopic, "online");
        // }
        //Говорим в статус, что мы живы
        mqttClient.publish(mqttAliveTopic, "online");

        //выбираем тип пакета
        uint16_t PacketTypeR;
        byte* ptrN = (byte*)&PacketTypeR;
        *ptrN = buf[14];
        *(ptrN + 1) = buf[13];
        // Serial.print("Packet type: ");
        // Serial.println(PacketTypeR, HEX);
        switch (PacketTypeR) {
          case 0x6600:
            //Serial.println("Select type 6600");
            tmr.start();
            parser_sender_102_00();
          break;
          case 0x6500:
            //Serial.println("Select type 6500");
            tmr.start();
            parser_sender_102_00();
          break;
        }
      }

    }

    //проверка соответствия длины
    // Serial.print("Lenght received: ");
    // Serial.println(buf[4], HEX);
    // Serial.print("Lenght calculated: ");
    // Serial.println(rlen-9, HEX);
        
  }

  //Срабатывание таймера отправки запроса статуса 
  if (tmr.tick()) {
    //  for (int j=0; j<21; j++) {
    //    p(request_status_byte[j]);      
    //    Serial.print(" ");       
    //  }
    //  Serial.println("");
     Serial.write(request_status_byte, 21);
  }

  //Срабатывание таймера статуса offline 
  if (tmr2.tick()) {
  // если таймер сработал, отправляем в топик alive "offline"
   if(isAlive == 1) {
     isAlive = 0;
     mqttClient.publish(mqttAliveTopic, "offline");
   }
  }  
}

// Debug.handle();
//}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 06 MQTT App</title></head><body>MQTT App demo";
  s += "<ul>";
  s += "<li>MQTT server: ";
  s += mqttServerValue;
  s += "</li>";
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void wifiConnected()
{
  needMqttConnect = true;
}

void configSaved()
{
  //Serial.println("Configuration was updated.");
  needReset = true;
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{
  //Serial.println("Validating form.");
  bool valid = true;

  int l = webRequestWrapper->arg(mqttServerParam.getId()).length();
  if (l < 3)
  {
    mqttServerParam.errorMessage = "Please provide at least 3 characters!";
    valid = false;
  }

  return valid;
}

bool connectMqtt() {
  unsigned long now = millis();
  if (1000 > now - lastMqttConnectionAttempt)
  {
    // Do not repeat within 1 sec.
    return false;
  }
  //Serial.println("Connecting to MQTT server...");

  if (!connectMqttOptions()) {
    lastMqttConnectionAttempt = now;
    return false;
  }
  //Serial.println("Connected!");
  mqttClient.publish(mqttAliveTopic, "online");  
  isAlive = 1;
  tmr2.start();
  tmr.start();

  //mqttClient.subscribe("test/action");
  mqttClient.subscribe(mqttActionTopic);
  //Serial.println(mqttActionTopic);  
  return true;
}

bool connectMqttOptions()
{
  bool result;
  if (mqttUserPasswordValue[0] != '\0')
  {
    result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue, mqttUserPasswordValue);
  }
  else if (mqttUserNameValue[0] != '\0')
  {
    result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue);
  }
  else
  {
    result = mqttClient.connect(iotWebConf.getThingName());
  }
  return result;
}

void mqttMessageReceived(String &topic, String &payload)
{
  int is_command_valid = 0;
  int str_len = payload.length()+1;
  char char_array[str_len];
  payload.toCharArray(char_array, str_len);

  GParser data(char_array, ':');
  int am = data.split();
  //Serial.println("received topic:");
  // Serial.println(data[0]);
  // Serial.println(data[1]);

  //команды вкл/выкл
  if (data.equals(0, "power")) {
    send_packet_prepare ();
    switch (data.getInt(1)) {
    case 0:
      //Serial.println("Turn Off");
      transmitt_byte[18] = 0x04; //0100
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;      
    break;
    case 1:
      //Serial.println("Turn On");
      transmitt_byte[18] = 0x0C; //1100
      transmitt_byte[23] = 0x04;
      is_command_valid = 1; 
    break;
    }
  }

  //команды вертикальный swing
  if (data.equals(0, "swingv")) {
    send_packet_prepare ();
    switch (data.getInt(1)) {
    case 0:
      //Serial.println("swingv Off");
      transmitt_byte[32] = 0x40; //0100 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;      
    break;
    case 1:
      //Serial.println("swingv On");
      transmitt_byte[32] = 0xc0; //1100 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1; 
    break;
    }
  }

  //команды горизонтальный swing
  if (data.equals(0, "swingh")) {
    send_packet_prepare ();
    switch (data.getInt(1)) {
    case 0:
      //Serial.println("swingh Off");
      transmitt_byte[32] = 0x10; //0001 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;      
    break;
    case 1:
      //Serial.println("swingh On");
      transmitt_byte[32] = 0x30; //0011 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1; 
    break;
    }
  }

  //команды wind (сила вентилятора): {"auto": "00000001" (1), "lowest": "00001011"(2), "low": "00001101"(3), "middle": "00001111"(4), "high": "00010001"(5), "highest": "00010011"(6)}
   if (data.equals(0, "wind")) {
    send_packet_prepare ();
    switch (data.getInt(1)) {
    case 1:
      //Serial.println("wind auto");      
      transmitt_byte[16] = 0x01; //0000 0001  0x01
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    case 2:
      //Serial.println("wind lowest");
      transmitt_byte[16] = 0x0b; //0000 1011  0x0b
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    case 3:
      //Serial.println("wind low");
      transmitt_byte[16] = 0x0d; //0000 1101  0x0d
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    case 4:
      //Serial.println("wind middle");
      transmitt_byte[16] = 0x0f; //0000 1111  0x0f
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;    
    case 5:
      //Serial.println("wind high");
      transmitt_byte[16] = 0x11; //0001 0001  0x11
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    case 6:
      //Serial.println("wind highest");
      transmitt_byte[16] = 0x13; //0001 0011  0x13
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    }
  } 

  //команды mode: {"fan": "0001" (0), "heat": "0011" (1), "cold": "0101" (2),  "dry": "0111" (3), "auto": "1001" (4)}
  if (data.equals(0, "mode")) {
    send_packet_prepare ();
    switch (data.getInt(1)) {
    case 0:
      //Serial.println("mode fan");      
      transmitt_byte[18] = 0x10; //0001 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    case 1:
      //Serial.println("mode heat");
      transmitt_byte[18] = 0x30; //0011 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    case 2:
      //Serial.println("mode cold");
      transmitt_byte[18] = 0x50; //0101 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    case 3:
      //Serial.println("mode dry");
      transmitt_byte[18] = 0x70; //0111 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;    
    case 4:
      //Serial.println("mode auto");
      transmitt_byte[18] = 0x90; //1001 0000
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    break;
    }
  }
    //команды установки температуры
  if (data.equals(0, "temp")) {
    //Serial.println(data.[1]));
    //Serial.println(data.getInt(1));
    send_packet_prepare ();
    if ((data.getInt(1)>15) && (data.getInt(1)<33)) {
      //Serial.print("Temp set: ");
      //Serial.println(data.getInt(1));
      transmitt_byte[19] = (data.getInt(1)*2)+1;
      transmitt_byte[23] = 0x04;
      is_command_valid = 1;
    }    
  }

  //Если команда была норм, то продолжаем форм ировать и отправлять пакет-команду
  if (is_command_valid == 1) {
    tmr.start();  //перезапускаем таймер отправки пакета-запроса состояния
    hash_adder(); //формируем контрольную сумму
    Serial.write(transmitt_byte, 50); //выводим в Serial сформированный пакет
    
    //Serial.println("Result packet");
    // for (int i=0; i<50; i++) {
    //   p(transmitt_byte[i]);
    //   ////Serial.print(transmitt_byte[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println(" ");

  }
}

  //для дебага печаталка HEX
  // void p(char X) {

  //  if (X < 16) {Serial.print("0");}

  //  Serial.print(X, HEX);

  // }
