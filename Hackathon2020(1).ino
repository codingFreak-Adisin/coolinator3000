#include <DHT.h>
#include <DHT_U.h>

#include <Servo.h>
#include <LiquidCrystal.h>
#include <IRremote.h>


//*** Hardware config. ***
//digital 9 is the servo control output
//digital 6 is the IR reciever input
//pins 2, 3, 4 are LED output pins
//pin 8 is the temperature input pin
//pin 5 is the PWM motor control pin
//analog pin 3 is for the microhpone input
//pins for LCD display...

//int DHT sensor on pin 8
#define DHTPIN 8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int RECV_PIN = 6; //Make sure pin 6 is the IR reciever input pin
IRrecv irrecv(RECV_PIN);
decode_results results;

Servo myservo; //create servo object

//LED control variables
int audioVal = 0; //the audio input from mic
int ledOut = 2; //start LED out on pin 2 (can be digital out pins 2, 3, 4)
int ledMode = 1; //(1=sound resp., 2=circle, 3=off)

//fan control variables
int fanSpeed = 100; //(can be 0-255 analog output to motor)
bool manualOn = 0; //(1=fan manually set on, 0=temp. controlled)
int tempSense = 0; //sensor data from temp. sensor
int tempMax = 75; //degrees set for maximum temp. b/f fan turns on
bool turnOn = 0; //(1=fan turns from side to side automatically)
int turnDirection = 1; //variable to determine turn direction of fan

 //Servo control variables
 int servoAngle = 0; //angle set for servo position

//timing variables
unsigned long lastTime = 0;
unsigned long diffTime = 0;
unsigned long lastTimeServo = 0;
unsigned long diffTimeServo = 0;

//temperature variables
float f = 0;


void setup() {
  // put your setup code here, to run once:

//int LED pins
pinMode(2, OUTPUT);
pinMode(3, OUTPUT);
pinMode(4, OUTPUT);

//int PWM motor control pin
pinMode(5, OUTPUT);

//int servo control pin
myservo.attach(9);
myservo.write(45);

Serial.begin(9600);
Serial.println("Starting up!");
irrecv.enableIRIn();
irrecv.blink13(true);

dht.begin(); //int DHT sensor

}

void loop() {
  // put your main code here, to run repeatedly:


//** Start of LED control code **
if (ledMode == 2){
  diffTime = millis() - lastTime; //calculate the time difference between last LED switch
  //Serial.println(diffTime);
}
if (ledMode == 1) { //ledMode set for sound resp.
  audioVal = analogRead(3);
  if (audioVal < 565) {
    Serial.print("Got value for audio: ");
    Serial.println(audioVal);
    ledOut += 1;
    delay(10);
  } //end of if got audio signal
} else if ((ledMode == 2)&&(diffTime > 250)) { //if LED mode is set to circle
  ledOut += 1;
  lastTime = millis(); //update lastTime
}
if (ledOut == 5) {
  ledOut = 2; //reset LED output
}
if (ledMode != 3) {
  //turn on/off the LEDs based on ledOut config.
  if (ledOut == 2) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }
  if (ledOut == 3) {
    digitalWrite(3, HIGH);
  } else {
    digitalWrite(3, LOW);
  }
  if (ledOut == 4) {
    digitalWrite(4, HIGH);
  } else {
    digitalWrite(4, LOW);
  }
} else {//i.e. if LED mode is set to off
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
}//end of LED on/off


//** Start of IR remote control code **
if (irrecv.decode(&results)) {
  Serial.print("Got IR code: ");
  Serial.println(results.value, HEX);

  switch(results.value){
    case 0xFFA25D: //case power button pressed
    Serial.println("toggling manual fan control");
    if (manualOn == 1) { //toggle manualOn
      manualOn = 0;
    } else {
      manualOn = 1;
    }
    break;
    case 0xFF629D: //case Vol+
    if (fanSpeed < 229) {
      fanSpeed += 25;
      Serial.println("changing fan speed");
    }
    break;
    case 0xFFA857: //case Vol-
    if (fanSpeed > 24){
      fanSpeed -= 25;
      Serial.println("changing fan speed");
    }
    break;
    case 0xFF22DD: //case |<<
    turnOn = 0; //disable auto-turning if needed
    if (servoAngle > 9) {
      servoAngle -= 10;
      Serial.print("changing servo angle to:");
      Serial.println(servoAngle);
    }
    break;
    case 0xFFC23D: //case >>|
    turnOn = 0; //disable auto-turning if needed
    if (servoAngle < 91) {
      servoAngle += 10;
      Serial.print("changing servo angle to: ");
      Serial.println(servoAngle);
    }
    break;
    case 0xFF30CF: //case 1
    ledMode = 1;
    break;
    case 0xFF18E7: //case 2
    ledMode = 2;
    break;
    case 0xFF7A85: //case 3
    ledMode = 3;
    break;
    case 0xFFE01F: //case v
    tempMax -= 2;
    Serial.print("tempMax set to ");
    Serial.println(tempMax);
    break;
    case 0xFF906F: //case ^
    tempMax += 2;
    Serial.print("tempMax set to ");
    Serial.println(tempMax);
    break;
    case 0xFF02FD: //case >|| (toggle on/off fan turning left-right)
    if (turnOn == 1) {
      turnOn = 0;
    } else {
      turnOn = 1;
    }
  } //end of switch
  
  irrecv.resume();
}//end of IR remote getting data

//**Motor control code **
if (manualOn == 1) {
  analogWrite(5, fanSpeed);
  //Serial.print("manual fan on. Fan Speed: ");
  //Serial.println(fanSpeed);
} else { //i.e. if manual is off

//get sensor data from temp sensor...
  f = dht.readTemperature(true);
  tempSense = int(f);
  //Serial.print("Got temperature reading as: ");
  //Serial.println(tempSense);
  if (tempSense > tempMax) {
    analogWrite(5, fanSpeed);
    //Serial.println("automatic cooling initiated!");
  } else {
    analogWrite(5, 0); //turn off fan if tempSense is too low
  }
}//end of motor control code

//**LCD Code**


//**Servo Control Code**
if (turnOn == 0) {//if the fan is not turning from left-right
myservo.write(servoAngle);
delay(15);
} else {//if auto-turning of fan is enabled
 diffTimeServo = millis() - lastTimeServo; //update diffTimeServo, then test to see if delay has occured
 if (diffTimeServo > 50) {
  if ((turnDirection == 1)&&(servoAngle < 90)) { //if direction is positive
    servoAngle += 2; //increment servoAngle by 2 degrees
  } else if ((turnDirection == -1)&&(servoAngle > 1)){//if direction is negative
    servoAngle -= 2; //increment servoAngle by -2 degrees
  } else { //this condition will be true only if the servoAngle is outside of operating range
    turnDirection = turnDirection * -1; //invert turn direction
  }
  myservo.write(servoAngle); //finally, update position on servo
  lastTimeServo = millis(); //update lastTimeServo
 }
}


}
