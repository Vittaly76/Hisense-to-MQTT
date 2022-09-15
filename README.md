# Hisense-to-MQTT
MQTT adapter for Hisense AC

Адаптер для упарвления кондиционерами Hisense Smart DC Inverter.

Код написан в среде Arduino с использованием ESP8266. Используется чип ESP-12F. Предполагается, что IDE уже настроена для работы с ESP8266. Для компиляции прошивки необходимо через менеджер библиотек установить следующие библиотеки:
- [arduinojson](https://arduinojson.org/)
- [arduino-mqtt](https://github.com/256dpi/arduino-mqtt)
- [IotWebConf](https://github.com/prampec/IotWebConf)
- [GParser](https://github.com/GyverLibs/GParser)
- [TimerMs](https://github.com/GyverLibs/TimerMs)
