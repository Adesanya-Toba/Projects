/*
OBSTACLE-AVOIDING ROVER USING THE INVENT ONE(R)
Required components;
1. Invent One Prototyping Board
2. HCSR04 Ultrasonic Range Finder
3. Servo Motor
4. L298N DC Motor Driver
5. Rover chasis and 4 DC motors

.........................................................
Written by Adesanya Toba, with help from the InvenTech team
Code adapted from http://www.educ8s.tv 

*/

//Include the Servo library
#include <Servo.h>

//Assign GPIO 12 and 13 to the TRIG and ECHO pins of the HCSR04 
//Ultrasonic rangefinder,respectively
#define trigPin 12
#define echoPin 13

//Create an instance of the Servo library
Servo myservo;

//Assign pins for the L298N motor driver
//IN1 = 4, IN2 =5, IN3 = 2(SDA), IN4 = 14(SCK)

//Motor A
#define motorPin1 4
#define motorPin2 5
//Motor B
#define motorPin3 2
#define motorPin4 14

int distance = 0;

void setup()
{
	//setup the motordriver pins as OUTPUT
	pinMode(motorPin1, OUTPUT);
	pinMode(motorPin2, OUTPUT);
	pinMode(motorPin3, OUTPUT);
	pinMode(motorPin4, OUTPUT);

	Serial.begin(9600);
	//setup the pinmode for the ultrasonic sensor
	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);
	//assign the servo pin to 16
	myservo.attach(16); 
	delay(1000);
	//allign the servo to its centre, mine was 80
	//this value may vary, depending on your calculation
	myservo.write(80);

  pinMode(15, OUTPUT);
	//read the distance a couple of times to get a steady value
	distance = readDistance();
	delay(100);
	distance = readDistance();
	delay(100);
	distance = readDistance();
	delay(100);
	//printing to the serial monitor (just for debugging)
	//can be commented out
	Serial.print("First distance: ");
	Serial.println(distance);
	//moveForward();
}

void loop()
{	
	//initiate straight, left and right distance variables to 0
	int distanceS = 0;
	int distanceR = 0;
	int distanceL = 0;

	//this will loop every 40 milliseconds
	delay(10);
	//moveForward();
	//measure the distance, in cm, from any obstacle
	distance = readDistance();

	if (distance < 30) 
	{
    digitalWrite(15, HIGH);
		//if the distance is less than 30cm, STOP
		moveStop();
		delay(100);
		//read the distance again(just for debugging)
		distance = readDistance();
		Serial.println(distance);
		//move back for 200ms
		moveBackward();
		delay(200);
		//STOP after 200ms
		moveStop();
		//look straight
		delay(200);
		distanceS = readDistance();
		delay(10);
		//look LEFT and measure distance
		distanceL = lookLeft();
		delay(10);
		//look RIGHT and measure distance
		distanceR = lookRight();
		delay(10);

		//compare and decide where to go
		if (distanceS < 20 || distanceL < 20 || distanceR < 20) 
		{
			moveStop();
			delay(10);
			moveBackward();
			delay(200);
			move360();
			delay(750);
			moveStop();
		}

		else if (distanceL > distanceR && distanceL > distanceS)
		{
			turnLeft();
      		digitalWrite(15, LOW);
			moveStop();
		}
		else if(distanceR > distanceL && distanceR > distanceS)
		{
			turnRight();
      		digitalWrite(15, LOW);
			moveStop();
		}
		else if (distanceS > distanceL && distanceS > distanceR)
		{
			moveForward();
		}
		else 
		{
			moveStop();
			moveBackward();
			delay(200);
			move360();
			delay(750);
      		digitalWrite(15, LOW);
			moveStop();
		}
	}
	else {
		//if distance is greater than 30cm, 
		//keep moving forward
		moveForward();
	}
	distance = readDistance();
}

// Necessary Functions
int readDistance()
{
	int duration, distanceCM;
	//digitalWrite(trigPin, LOW);
	delayMicroseconds(100);
	digitalWrite(trigPin, HIGH);
	delayMicroseconds(100);
	digitalWrite(trigPin, LOW);

	duration = pulseIn(echoPin, HIGH);

	distanceCM = (duration / 2) / 29;

	//Serial.print(distanceCM);
	//Serial.println("cm");
	//delay(250);
	return distanceCM;
}

int lookRight()
{
	myservo.write(180);
	delay(250);
	int distance = readDistance();
	delay(500);
	myservo.write(80);
	return distance;
}

int lookLeft()
{
	myservo.write(0);
	delay(250);
	int distance = readDistance();
	delay(500);
	myservo.write(80);
	return distance;
}

void turnLeft() {
	analogWrite(motorPin1, 0);
	analogWrite(motorPin2, 600);
	analogWrite(motorPin3, 0);
	analogWrite(motorPin4, 0);
	delay(400);
	//moveForward();
}

void turnRight() {
	analogWrite(motorPin1, 0);
	analogWrite(motorPin2, 0);
	analogWrite(motorPin3, 600);
	analogWrite(motorPin4, 0);
	delay(400);
	//moveForward();
}

void moveForward()
{
	analogWrite(motorPin1, 0);
	analogWrite(motorPin2, 600);
	analogWrite(motorPin3, 600);
	analogWrite(motorPin4, 0);
}

void moveBackward() {

	analogWrite(motorPin1, 600);
	analogWrite(motorPin2, 0);
	analogWrite(motorPin3, 0);
	analogWrite(motorPin4, 600);

}

void move360()
{
	analogWrite(motorPin1, 0);
	analogWrite(motorPin2, 600);
	analogWrite(motorPin3, 0);
	analogWrite(motorPin4, 600);
}

void moveStop() {
	analogWrite(motorPin1, 0);
	analogWrite(motorPin2, 0);
	analogWrite(motorPin3, 0);
	analogWrite(motorPin4, 0);
}

/*WAKANDA FOREVER*/
