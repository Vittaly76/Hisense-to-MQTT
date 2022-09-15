typedef struct _Device_Status{
	uint8_t wind_status;//风量
	uint8_t sleep_status;//睡眠
	
	//uint8_t mode_status:4;//模式  
	// uint8_t run_status:2;//运行
	// uint8_t direction_status:2;//风向
  
  uint8_t mode_run_direction_status;  //mode_status:4; run_status:2; direction_status:2;
	// 4
	uint8_t indoor_temperature_setting;//室内温度
	uint8_t indoor_temperature_status;//室内温度
	uint8_t indoor_pipe_temperature;//室内管温值
	// 7
	uint8_t indoor_humidity_setting;//室内湿度
	uint8_t indoor_humidity_status;//室内湿度
	
	uint8_t somatosensory_temperature;//体感温度
	// 10
	uint8_t somatosensory_compensation:5;//体感补偿
	uint8_t somatosensory_compensation_ctrl:3;//体感补偿控制
    // 11
	uint8_t temperature_compensation:5;//温度补偿
	uint8_t temperature_Fahrenheit:3;//华氏显示
	
	// 12
	uint8_t timer;//定时

	// 13
	uint8_t hour;
	// 14
	uint8_t minute;
	// 15
	uint8_t poweron_hour;
	// 16
	uint8_t poweron_minute;
	// 17
	uint8_t poweroff_hour;
	// 18
	uint8_t poweroff_minute;
	// 19
	uint8_t drying:4;
	uint8_t wind_door:4;
	// 20
	// uint8_t up_down:1;//上下风
	// uint8_t left_right:1;//左右风
	// uint8_t nature:1;//自然风
	// uint8_t heat:1;//加热风
	// uint8_t low_power:1;//节能
	// uint8_t low_electricity:1;//节电
	// uint8_t efficient:1;//高效
	// uint8_t dual_frequency:1;//双频
  uint8_t binary_byte_20;  //Описание бит выше

	// 21
	uint8_t dew:1;//清新
	uint8_t swap:1;//换风
	uint8_t indoor_clear:1;//室内清洁
	uint8_t outdoor_clear:1;//室外清洁
	uint8_t smart_eye:1;//智慧眼
	uint8_t mute:1;//静音
	uint8_t voice:1;//语音
	uint8_t smoke:1;//除烟
	// 22
	uint8_t back_led:1;//背景灯
	uint8_t display_led:1;//显示屏
	uint8_t indicate_led:1;//指示灯
	uint8_t indoor_led:1;//室内室外切换灯

	uint8_t filter_reset:1;//过滤网复位
	uint8_t left_wind:1;//左风
	uint8_t right_wind:1;//右风
	uint8_t indoor_electric:1;//室内电量板
	//23
	uint8_t auto_check:1;//自检
	uint8_t time_lapse:1;//缩时
	uint8_t rev23:4;
	uint8_t sample:1;//样品
	uint8_t indoor_eeprom:1;//eeprom
	//24
	uint8_t indoor_temperature_sensor:1;
	uint8_t indoor_temperature_pipe_sensor:1;
	uint8_t indoor_humidity_sensor:1;
	uint8_t indoor_water_pump:1;
	uint8_t indoor_machine_run:1;
	uint8_t indoor_bars:1;
	uint8_t indoor_zero_voltage:1;
	uint8_t indoor_outdoor_communication:1;

	//25
	uint8_t display_communication:1;
	uint8_t keypad_communication:1;
	uint8_t wifi_communication:1;
	uint8_t electric_communication:1;
	uint8_t eeprom_communication:1;
	uint8_t rev25:3;
	//26
	uint8_t compressor_frequency;
	//27
	uint8_t compressor_frequency_setting;
	//28
	uint8_t compressor_frequency_send;
	//29
	uint8_t outdoor_temperature;
	//30
	uint8_t outdoor_condenser_temperature;
	//31
	uint8_t compressor_exhaust_temperature;
	//32
	uint8_t target_exhaust_temperature;
	//33
	uint8_t expand_threshold;
	//34
	uint8_t UAB_HIGH;
	//35
	uint8_t UAB_LOW;
	//36
	uint8_t UBC_HIGH;
	//37
	uint8_t UBC_LOW;
	//38
	uint8_t UCA_HIGH;
	//39
	uint8_t UCA_LOW;
	//40
	uint8_t IAB;
	//41
	uint8_t IBC;
	//42
	uint8_t ICA;
	//43
	uint8_t generatrix_voltage_high;
	//44
	uint8_t generatrix_voltage_low;
	//45
	uint8_t IUV;
	//46
	uint8_t rev46:3;
	uint8_t four_way:1;
	uint8_t outdoor_machine:1;
	uint8_t wind_machine:3;
	// //47
	// uint8_t rev47;
	// //48
	// uint8_t rev48;
	// //49
	// uint8_t rev49;
	// //50
	// uint8_t rev50;
	// //51
	// uint8_t rev51;
	// //52
	// uint8_t rev52;
	// //53
	// uint8_t rev53;
	// //54
	// uint8_t rev54;
	// //55
	// uint8_t rev55;
	// //56
	// uint8_t rev56;
} * PDevice_Status, Device_Status;