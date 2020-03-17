/*
	Name:       Norman.ino
	Created:	3/12/2020 5:50:28 PM
	Author:     DESKTOP-U04GU6B\Carson
*/

#include <ArduinoBlue.h>

/*
Motor 1 - left wheel
Motor 2 - right wheel

*/


//bluetooth 
constexpr unsigned int BLE_BAUD = 9600;
int prevThrottle = 49;
int prevSteering = 49;
int throttle, steering, button;


//motor pins
constexpr unsigned int MOTOR1 = 2, MOTOR2 = 3, REVERSE1 = 24, REVERSE2 = 25;
bool m1_fwd_flag = true, m2_fwd_flag = true, reverse = false;

//motor constants
constexpr unsigned int MSTART_VAL = 100, MTR_INCREMENT = 10, HALF_POWER = 150;
unsigned int m1_val = MSTART_VAL, m2_val = MSTART_VAL;
bool hard_turning = false;
constexpr unsigned int THROTTLE_ZERO_THRESHOLD = 5;

ArduinoBlue phone(Serial2);

void setup()
{ 
	//setup reverse
	pinMode(REVERSE1, OUTPUT);
	pinMode(REVERSE2, OUTPUT);
	digitalWrite(REVERSE1, HIGH);
	digitalWrite(REVERSE2, HIGH);

	//setup bluetooth serial
	Serial.begin(9600);
	Serial2.begin(BLE_BAUD);
	if (m1_fwd_flag != m2_fwd_flag)
		m1_fwd_flag = m2_fwd_flag;
}

void reverse_motor(bool left_mtr = false) {
	unsigned int m_val = 0;

	if (left_mtr) {
		//stop motors
		analogWrite(MOTOR1, m_val);
		delay(50);
		digitalWrite(REVERSE1, LOW);
		delay(500);
		analogWrite(MOTOR1, HALF_POWER);
		delay(20);
		analogWrite(MOTOR1, 0);
		delay(500);
		digitalWrite(REVERSE1, HIGH);
		m1_fwd_flag = !m1_fwd_flag;
	}
	else {
		analogWrite(MOTOR2, m_val);
		delay(50);
		digitalWrite(REVERSE2, LOW);
		delay(500);
		analogWrite(MOTOR2, HALF_POWER);
		delay(20);
		analogWrite(MOTOR2, 0);
		delay(500);
		digitalWrite(REVERSE2, HIGH);
		m2_fwd_flag = !m2_fwd_flag;
	}
}

void toggle_motor_reverse() {
	reverse = !reverse;
	if(m1_fwd_flag != reverse) //check left wheel
		reverse_motor(true);
	if (m2_fwd_flag != reverse) //check right wheel
		reverse_motor(false);
}

void sec_motors_forward(unsigned int duration_ms = 1000, unsigned int power_level = HALF_POWER) {
	analogWrite(MOTOR1, power_level);
	analogWrite(MOTOR2, power_level);
	delay(duration_ms);
	analogWrite(MOTOR1, 0);
	analogWrite(MOTOR2, 0);
}

bool run_bttn() {
	if (button == 0) {
		(reverse) ? phone.sendMessage("Reverse"): phone.sendMessage("Forward");
		toggle_motor_reverse();
	}
	else if (button == 1) {
		hard_turning = !hard_turning;
		(hard_turning) ? phone.sendMessage("Hard Turning is On"): phone.sendMessage("Hard Turning is Off");
		if (!hard_turning) {
			if (m1_fwd_flag != reverse)
				reverse_motor(true);
			if (m2_fwd_flag != reverse)
				reverse_motor();
		}
	}
	else if (button == 2) {
		m1_val = 0;
		m2_val = 0;
		analogWrite(MOTOR1, m1_val);
		analogWrite(MOTOR2, m2_val);
	}
	else if (button == 3) {
		if (m1_fwd_flag != reverse) {
			reverse_motor(false); //get both wheels into same 
			m1_fwd_flag = !m1_fwd_flag;
		}
		else {
			reverse_motor(true);
			m2_fwd_flag = !m2_fwd_flag;
		}
		Serial.println("Fixed");
	}
	else {
		return true; //err occurred
	}
	return false;
}

bool drive() {
	throttle -= 49;
	steering -= 49;
	// If the throttle is close enough to zero, the robot stays still.
	if (abs(throttle) < THROTTLE_ZERO_THRESHOLD) {
		m1_val = 0;
		m2_val = 0;
	}
	// If the throttle is greater than zero, then the robot goes forward
	else {
		if (steering < -20) { //turn left
			if (hard_turning) {
				if (m1_fwd_flag != false)
					reverse_motor(true);
				if (!m2_fwd_flag)
					reverse_motor();
				m1_val = 120;
				m2_val = 120;
				
			}
			else {
				m1_val = 0;
				m2_val = HALF_POWER;
			}
			Serial.println("Turn Left" + String(m1_fwd_flag) + "," + String(m2_fwd_flag));
		}else if(steering > 20) { //turn right
			
			if (hard_turning) {
				//Serial.print("Hard ");
				if(m2_fwd_flag != false)
					reverse_motor();
				if (!m1_fwd_flag)
					reverse_motor(true);
				m2_val = 120;
				m1_val = 120;

			}
			else {				
				m2_val = 0;
				m1_val = HALF_POWER;
			}
			Serial.println("Turn Right" + String(m1_fwd_flag) + "," + String(m2_fwd_flag));
		}
		else { //forward
			(reverse) ? Serial.println("Backward") : Serial.println("Forward");
			if (m1_fwd_flag != m2_fwd_flag) {
				if (m1_fwd_flag == reverse) {
					reverse_motor(true);
				}
				else {
					reverse_motor(false);
				}
			}
			m1_val = HALF_POWER;
			m2_val = m1_val;
		}
	}
	analogWrite(MOTOR1, m1_val);
	analogWrite(MOTOR2, m2_val);
	
	
	return false;
}

void loop()
{
	button = phone.getButton();

	String str = phone.getText();

	// Throttle and steering values go from 0 to 99.
	// When throttle and steering values are at 99/2 = 49, the joystick is at center.
	throttle = phone.getThrottle();
	steering = phone.getSteering();
	
	// Display button data whenever its pressed.
	if (button != -1) {
		if (run_bttn())
			Serial.println("Error running button with value: " + button);
	}

	// Display throttle and steering data if steering or throttle value is changed
	if (prevThrottle != throttle || prevSteering != steering) {
		
		//Serial.print("Throttle: "); 
		//Serial.print(throttle);
		//Serial.print("\tSteering: "); 
		//Serial.println(steering);
		prevThrottle = throttle;
		prevSteering = steering;
		
	}
	else {
		if (drive())
			Serial.println("Error driving with values: " + String(throttle) + "," + String(steering));
	}
	/*
	Serial.println("Current State: m1val= " + String(m1_val) +
		" M1 Forward Flag= " + String(m1_fwd_flag) + " m2val= " + String(m2_val) +
		" M2 forward Flag= " + String(m2_fwd_flag) + " Reverse: " + String(reverse) +
		" hard turning= " + String(hard_turning));*/

}

