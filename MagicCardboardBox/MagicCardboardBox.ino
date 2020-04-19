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
constexpr unsigned int right_key = 0;
constexpr unsigned int left_key= 408;
constexpr unsigned int up_key = 98;
constexpr unsigned int down_key = 256;
constexpr unsigned int select_key = 639;

//lcd properties
constexpr unsigned int ROW_WIDTH = 17;
constexpr unsigned int COL_HEIGH = 2;

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
	auto menu_x = 0, menu_y = 0;

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
	while (1) {
		menu_x %= max_menu_x;
		menu_y %= max_menu_y;
		int key_press = -1;
		key_press = analogRead(pin_anal);
		if (DEBUG)
			Serial.println("Menu:" + String(menu_x) + String(menu_y) + " Key Press:" + String(key_press));
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
		String buffer = "";
		switch (menu_y) {
			case 2: //MPU6050 Sensor
				menu_x %= 4;
				lcd.home();
				lcd.println("MPU6050 Sensor: ");
				delay(500);
				lcd.setCursor(0, 1);
				
				//mpu sensor events
				sensors_event_t accel, gyro, temp;
				mpu.getEvent(&accel, &gyro, &temp);
				if (DEBUG) {
					Serial.println(menu[menu_y][menu_x]);
				}
				else if (menu[menu_y][menu_x] == '0') {
					lcd.println(
						"A:" + String(accel.acceleration.x) + ',' + String(accel.acceleration.y) + "   "
					);
				}
				else if (menu[menu_y][menu_x] == 'A') {
					lcd.clear();
					lcd.println(
						"A:[" + String(accel.acceleration.x) + ',' + String(accel.acceleration.y) + ','+ String(accel.acceleration.z)+']'
					);
				}
				else if (menu[menu_y][menu_x] == 'G') {
					lcd.println(
						"G:" + String(gyro.gyro.x) + ',' + String(gyro.gyro.y) + ',' + String(gyro.gyro.z)
					);
				}
				else { //temperature
					lcd.println(
						"T:" + String(temp.temperature)
					);
				}
				delay(500);
				break;
			
			case 1: //AS7263 Sensor
				lcd.home();
				lcd.println("AS7263 Sensor:  ");
				lcd.setCursor(0, 1);

				if (menu[menu_y][menu_x] == 'T') {
					buffer = "T(F):";
					float temper = ams.getTemperatureF();
					buffer += String(temper, 1);
				}
				else if (menu[menu_y][menu_x] == 'C') {
					lcd.home();
					lcd.println("                ");
					lcd.setCursor(0,0);
					buffer += "V:" + String(ams.getViolet(), DEC);
					buffer += " B:" + String(ams.getBlue(), DEC);
					buffer += " G:" + String(ams.getGreen(), DEC);
					buffer += " Y:" + String(ams.getYellow(), DEC);
					lcd.println(buffer);
					lcd.setCursor(0, 1);
					buffer = "";
					buffer += "O:" + String(ams.getOrange(), DEC);
					buffer += " R:" + String(ams.getRed(), DEC);
					lcd.println(buffer);
					delay(2000);
				}
				else if (menu[menu_y][menu_x] == '3') {
					lcd.clear();
					lcd.println("Red, Green, Blue");
					lcd.setCursor(0, 1);
					buffer = String(ams.getRed(), DEC) + ' ' +  String(ams.getGreen(), DEC) + ' ' + String(ams.getBlue(), DEC);
					lcd.println(buffer);
				}
				else {//Violet, yellow, orange
					lcd.clear();
					lcd.println("VT, YELO, ORG   ");
					lcd.setCursor(0, 1);
					buffer = String(ams.getRed(), DEC) + ' ' + String(ams.getGreen(), DEC) + ' ' +  String(ams.getBlue(), DEC);
					lcd.println(buffer);
				}
				if (DEBUG)
					Serial.println("Menu: " + menu[menu_y][menu_x] + ' ' + buffer);
				/*for (auto i = buffer.length(); i < row_width; i++)
					buffer += ' ';
				lcd.println(buffer);*/
				delay(500);
				break;

			default: //settings menu
				lcd.home();   
				lcd.println("Settings Menu   ");
				lcd.setCursor(0, 1);
				if (menu[menu_y][menu_x] == 'A') {

				}
				else if (menu[menu_y][menu_x] == 'R') {

				}
				else { //device info

				}
				break;
		}
		last_key = key_press;
		delay(50);
	}
}
