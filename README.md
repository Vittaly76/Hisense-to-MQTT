# Hisense-to-MQTT
MQTT adapter for Hisense AC

Адаптер для упарвления кондиционерами Hisense Smart DC Inverter.

Код написан в среде Arduino с использованием ESP8266. Используется чип ESP-12F. Предполагается, что IDE уже настроена для работы с ESP8266. Для компиляции прошивки необходимо через менеджер библиотек установить следующие библиотеки:
- [arduinojson](https://arduinojson.org/)
- [arduino-mqtt](https://github.com/256dpi/arduino-mqtt)
- [IotWebConf](https://github.com/prampec/IotWebConf)
- [GParser](https://github.com/GyverLibs/GParser)
- [TimerMs](https://github.com/GyverLibs/TimerMs)

**Важно:**
Библиотека IotWebConf по умолчанию находится в Debag-mode, и поэтому сыпет логами в Serial, что неприемлемо, так как Serial используется для взаимодействия с кондиционером. Возможность отключения Debug предусмотрена только при работе с VSCode. Если же используется IDE Arduino, то Debag можно отключить, добавив в файл Ваше_расположение\Arduino\libraries\IotWebConf\src\IotWebConfSettings.h в самом начале строчку:

`#define IOTWEBCONF_DEBUG_DISABLED`

**Использование**
После загрузки в чип и запуска инициируется точка доступа "HisenseTest". Подключившись к ней, нужно зайти браузером по адресу 192.168.4.1. Логин/пароль по умолчанию Admin/Admin. В открывшейся странице задаются:
- Имя модуля
- Пароль администратора для будущего входа на данную страницу
- SSID и пароль домашней сети
- Параметры MQTT-брокера (адрес, логин, пароль)

После ввода и сохранения всех параметров произойдёт перезагрузка, и введённые параметры вступят в силу.
На плате предусмотрен светодиод, который индицирует режим работы (есть/нет подключение к серверу) и в штатном режиме он изредка мигает.

При подключении к MQTT-брокеру модуль публикует 2 топика:
1. devices/{Имя_модуля}/status. Здесь раз в 5 сек. в формате JSON выводятся данные о состоянии кондиционера:
   `{
    "wind_status": 0,
    "sleep_status": 0,
    "mode_status": 2,
    "run_status": 0,
    "direction_status": 0,
    "indoor_temperature_setting": 24,
    "indoor_temperature_status": 23,
    "up_down": 0,
    "left_right": 0,
    "outdoor_temperature": 13
    }`
    
    - wind_status: скорость вентилятора. 1 - auto, 10 - lowest, 12 - low, 14 - middle, 16 - high, 18 - highest.
    - sleep_status: статус таймера. 0 - выкл, 1 - вкл
    - mode_status: режим работы. 0 - fan, 1 - heat, 2 - cool, 3 - dry, 4 - auto
    - run_status: вкл/выкл. 0 - выкл, 2 - вкл
    - direction_status: ?
    - indoor_temperature_setting: выставленная (желаемая) температура в помещении
    - indoor_temperature_status: текущая ткмпература в помещении
    - up_down: качание шторки вверх/вниз. 0 - выкл, 1 - вкл.
    - left_right: качание шторки влево/вправо. 0 - выкл, 1 - вкл.
    - outdoor_temperature: температура на улице
    
2. devices/{Имя_модуля}/alive. Возможные состояния offline/online. Можно использовать для определения доступности. Топик использует режим LastWill (при отвале модуля от сервера по любой причине брокер опубликует в топике offline)

3. Для управления кондиционером модуль слушает команды в топике devices/{Имя_модуля}/action. Возможные команды:
    - `mode:0`. Возможные значения:
        -  0: fan
        -  1: heat
        -  2: cool
        -  3: dry
        -  4: auto
    -  `wind:1`. Возможные значения:
        - 1: auto
        - 2: lowest
        - 3: low
        - 4: middle
        - 5: high
        - 6: highest
    - `power:0`. Возможные значения:
        - 0: выкл
        - 1: вкл
    - `swingv:0`. Вертикальное покачивание. Возможные значения:
        - 0: выкл
        - 1: вкл
    - `swingh:0`. Горизонтальное покачивание. Возможные значения:
        - 0: выкл
        - 1: вкл


