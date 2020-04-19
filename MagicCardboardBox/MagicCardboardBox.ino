/*
    Name:       MagicCardboardBox.ino
    Created:	4/14/2020 4:59:53 PM
    Author:     DESKTOP-U04GU6B\Carson
*/

#include <AS726X.h>
#include <LiquidCrystal.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <wire.h>


/*General Environment Variables*/
#define DEBUG 1 //true if debugging
#define SERIAL_BAUD 9600

/*LCD Display & Keyboard*/

//board pins
constexpr unsigned int pin_anal = 0;
constexpr unsigned int pin_RS = 8;
constexpr unsigned int pin_EN = 9;
constexpr unsigned int pin_d4 = 4;
constexpr unsigned int pin_d5 = 5;
constexpr unsigned int pin_d6 = 6;
constexpr unsigned int pin_d7 = 7;
constexpr unsigned int pin_BL = 10;

//analog read values
constexpr unsigned int right_key = 0; //0 on 2.56
constexpr unsigned int left_key= 408; //775 on 2.56
constexpr unsigned int up_key = 98; //188 on 2.56v
constexpr unsigned int down_key = 256; //484 on 2.56
constexpr unsigned int select_key = 639; //1023 on 2.56

//lcd properties
constexpr unsigned int ROW_WIDTH = 17;
constexpr unsigned int COL_HEIGH = 2;
constexpr char clear_buff[17] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

LiquidCrystal lcd(pin_RS, pin_EN, pin_d4, pin_d5, pin_d6, pin_d7);


/*MPU6050 Accceleerometer and Gyrometer*/
Adafruit_MPU6050 mpu;

//AS7263 Spectrometer
AS726X ams;

void setup()
{
	if (DEBUG) {
		Serial.begin(SERIAL_BAUD);
		while (!Serial)
			delay(10);
		Serial.println("Magic Cardboard Box: Debugging is On");
	}

	//initalize LCD
	lcd.begin(17, 2);
	lcd.setCursor(0, 0);
	lcd.print("Cardboard Box");
	digitalWrite(pin_BL, HIGH);///ensure lcd backlight is on

	//initalize MPU6050
	if (!mpu.begin()) {
		if (DEBUG)
			Serial.println("MPU6050 not Detected");
		while (1);
	}

	//setup MPU components
	mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
	mpu.setGyroRange(MPU6050_RANGE_500_DEG);
	mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

	//setup Spectrometer
	if (!ams.begin()) {
		if (DEBUG)
			Serial.println("AS7263 Not Detected");
		while (1);
	}

	if (DEBUG)
		Serial.println("Setup Complete");
}

/*to-do
implement menu options
Serialize all data somehow


*/

//allows for easy reordering of menu, initalized at compile time
constexpr unsigned short int max_menu_x = 5, max_menu_y = 3;
constexpr char menu[max_menu_y][max_menu_x] = { 
						{'A', 'R', 'D', ' ', ' '},
						{'0', 'T', 'C', '3', '6'}, //Spectrometer options
						{'0', 'A', 'G', ' T'} //MPU options
}; 


void loop()
{
	auto menu_x = 2, menu_y = 1;

	/*Menu Key
		0: display all of the sensors data on screen
		MPU -
			A: Accelerometer Data
			G: Gyroscope Data
			T: Temperature Data
		Spectrometer -
			T: Temperature Data
			C: Color Data
			3: RGB Data
			6: Violet, Yellow, Orange Data
		Settings - 
			A: Display all sensor information on scroll
			R: Reset 
			D: Display Device Information
	*/
	int last_key = -1;
	bool update_top_row = false, update_bot_row = false;
	while (1) {
		menu_x %= max_menu_x;
		menu_y %= max_menu_y;
		int key_press = -1;
		key_press = analogRead(pin_anal);
		
		if (DEBUG)
			Serial.println("Menu:" + String(menu_x) + String(menu_y) + " Key Press:" + String(key_press));

		//take user input
		if (key_press <= right_key) {
			menu_x += 1;
		}
		else if (key_press <= up_key) {
			menu_y += 1;
		}
		else if (key_press <= down_key) {
			menu_y -= 1;
		}
		else if (key_press <= left_key) {
			menu_x -= 1;
		}
		else {
			//do nothing on select or no press
		}
		delay(200);

		//apply user input to lcd display
		//two buffers for lcd display
		String top_buff = "";
		String bot_buff = "";

		switch (menu_y) {
			case 2: //MPU6050 Sensor
				sensors_event_t accel, gyro, temp;
				mpu.getEvent(&accel, &gyro, &temp);
				switch (menu[menu_y][menu_x]) {
					case '0':
						break;
					case 'A':
						top_buff = "Accelerometer:";
						bot_buff = "X:" + String(accel.acceleration.x, DEC) +
									" Y:" + String(accel.acceleration.y,DEC) + 
									" Z:" + String(accel.acceleration.y, DEC);
						break;
					case 'G':
						top_buff = "Gyroscope:";
						bot_buff = "X:" + String(gyro.gyro.x, DEC) +
							" Y:" + String(gyro.gyro.y, DEC) +
							" Z:" + String(gyro.gyro.y, DEC);
						break;
					default: //Temperature
						top_buff = "Temperature:";
						bot_buff = String(temp.temperature, DEC);
						break;
				}
				break;
			case 1: //AS7263 Sensor
				ams.takeMeasurements();
				char headers[7] = {'R', 'G', 'B', 'V', 'Y', 'O', 'T'};
				int head_vals[7] = {ams.getRed(), ams.getBlue(), ams.getGreen(), ams.getViolet(), ams.getYellow(),
					ams.getOrange(), (int) ams.getTemperatureF()};
				switch (menu[menu_y][menu_x]) {
					case '0':
						break;
					case 'C':
						for (auto i = 0; i < 3; i++) {
							top_buff += headers[i] + String(head_vals[i], DEC) + ' ';
							bot_buff += headers[i + 3] + String(head_vals[i + 3], DEC) + ' ';
						}
						break;
					case '3':
						top_buff = "RGB Values";
						for (auto i = 0; i < 3; i++) {
							bot_buff += headers[i] + String(head_vals[i], DEC) + ' ';
						}
						break;
					case '6':
						bot_buff = "RGB Values";
						for (auto i = 3; i < 6; i++) {
							bot_buff += headers[i] + String(head_vals[i], DEC) + ' ';
						}
						break;
					default: //temperature
						top_buff = "Temperature:";
						bot_buff = "fahrenheit:" + String(head_vals[6], DEC);
						break;
				}
				
				break;
			default: //settings menu
				break;
		}
		update_top_row = true;
		update_bot_row = true;
		//update lcd diplay
		if (update_top_row) {
			if (DEBUG)
				Serial.println("Top:" + top_buff);

			lcd.setCursor(0, 0);
			lcd.println(clear_buff);
			lcd.home();
			for (auto i = top_buff.length(); i < ROW_WIDTH; i++)
				top_buff += ' ';
			lcd.println(top_buff);
		}
		if (update_bot_row) {
			if (DEBUG)
				Serial.println("Bottom:" + bot_buff);
			lcd.setCursor(0, 1);
			lcd.println(clear_buff);
			for (auto i = bot_buff.length(); i < ROW_WIDTH; i++)
				bot_buff += ' ';
			lcd.setCursor(0, 1);
			lcd.println(bot_buff);
		}

		last_key = key_press;
		int wait = analogRead(0);
		while (wait >= 1000 || wait < 600) {
			wait = analogRead(0);
			delay(10);
		}
		delay(100);
	}
}
