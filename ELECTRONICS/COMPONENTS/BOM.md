# BILL OF MATERIALS
Arduino nRF Bluetooth Boards:
	
	Adafruit Feather nRF52 Bluefruit LE - nRF52832
		https://www.adafruit.com/product/3406

	Adafruit Feather nRF52840 Express
		https://www.adafruit.com/product/4062

Arduino ESP Wi-Fi Boards:
	Adafruit Feather HUZZAH with ESP8266
		https://www.adafruit.com/product/2821

	Adafruit HUZZAH32 – ESP32 Feather Board
		https://www.adafruit.com/product/3405



Components:
	Connector #1 -- Wheel trigger input -- {name_AND_link_for_product}

	Button #1 -- Manual Shutter trigger and Wheel input disabler -- larger 5mm button
	Button #2 -- Manual Bluetooth Reset -- smaller button

	LED #1 -- Visual feedback -- 5mm basic LED
	Resistor #1 -- Goes with LED -- 210 ohm

	Connector #2 -- 1x3 r/a pin headers for slide switch
	Switch #1 -- On/Off switch -- 3 pin slide switch

	Headers #1 -- select num wheel rotations for trigger -- 2x4 straight headers
	Jumper #1 -- 

	OPTIONAL: 
		Headers #2 -- select board type for thermistor input voltage divider -- 1x4 straight headers
		Jumper #2 -- 
		Resistor #2 -- comparison resistor (sized to thermistor) -- 100k

		Resistor #3 & #4 -- voltage divider resistors to drop voltage for ESP8266 (because analog reading has to be 0 to 1v for this board) -- 270 ohm & 120 ohm



REFERENCE INFORMATION

	All Adafruit Bluetooth Feather Boards: https://learn.adafruit.com/adafruit-feather/bluetooth-feathers