#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Password.h>
#include <Key.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include "RTClib.h"
SoftwareSerial SIM900A(9,10);
int pirPin = 42;
int redLedPin = 46;
int greenLedPin=47;
int ldr=A0;
int ldrValue;
int sensorValue=500;
//Real Time Clock
RTC_DS1307 RTC;
Password password = Password("1234");
#define I2C_ADDR 0x3F // Define I2C Address for controller
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
#define BACKLIGHT 3
LiquidCrystal_I2C
lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);
const byte rows=4;
const byte cols=4;
32char keys[rows][cols]={
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'},
};
byte rowPins[rows]={30,31,32,33};
byte colPins[cols]={34,35,36,37};
Keypad keypad=Keypad(makeKeymap(keys),rowPins,colPins,rows,cols);
int passwd_pos = 8;
int alarmStatus=0;
int alarmActive = 0;
void setup(){
SIM900A.begin(9600);
Serial.begin(9600);
//Adding time
Wire.begin();
RTC.begin();
RTC.adjust(DateTime(__DATE__, __TIME__));
pinMode(pirPin, INPUT);
pinMode(ldr,INPUT);
pinMode(redLedPin, OUTPUT);
pinMode(greenLedPin, OUTPUT);
lcd.begin(16, 2);
lcd.setBacklightPin(BACKLIGHT,POSITIVE);
lcd.setBacklight(HIGH);
lcd.clear();
delay(1000);
lcd.home();
displayCodeEntryScreen();
keypad.addEventListener(keypadEvent);
}
33/////////////////////////////////////////////////// functions//////////////////////////////////////////////////
void displayCodeEntryScreen()
// Dispalying start screen for users to enter PIN
{
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Enter PIN:");
}
//////////////////////////////////////////Deactive the security
system/////////////////////////////////////////////
void deactivate()
{
//digitalWrite(camera, LOW);
alarmStatus = 0;
lcd.clear();
lcd.setCursor(0,0);
lcd.print("DEACTIVATED");
delay(10000);
Serial.println("Security System Deactived");
alarmActive = 0;
password.reset();
delay(5000);
displayCodeEntryScreen();
digitalWrite(redLedPin, LOW);
}
//////////////////////////////////Message send function////////////////////////////////////////////
void SendMessage()
{
Serial.println ("SIM900A Sending SMS");
SIM900A.println("AT+CMGF=1"); //Sets the GSM Module in Text
Mode
delay(1000); // Delay of 1000 milli seconds or 1 second
Serial.println ("Setting SMS Number");
SIM900A.println("AT+CMGS=\"+8801704894257\"\r"); // Replace with
your mobile number
34delay(1000);
Serial.println ("Setting SMS Content");
SIM900A.println("Alart!!!Someone Enter your room...");// The SMS text
you want to send
//delay(10000);
Serial.println ("Finish");
SIM900A.println((char)26);// ASCII code of CTRL+Z
delay(1000);
Serial.println (" -> Next Execution..");
}
///////////////////////////////////Keypad Event function////////////////////////////////////////
void keypadEvent(KeypadEvent eKey){
switch (keypad.getState())
{
case PRESSED:
if (passwd_pos - 8 >= 5) {
return ;
}
//displayCodeEntryScreen();
lcd.setCursor((passwd_pos++),1);
switch (eKey){
case '#':
//# is to validate password
passwd_pos = 8;
checkPassword();
break;
case '*':
//* is to reset password attempt
password.reset();
passwd_pos = 8;
break;
default:
password.append(eKey);
lcd.print("*");
}
}
}
35//////////////////////// To check if PIN is corrected, if not, retry!///////////////////////////////
void checkPassword()
{
if (password.evaluate())
{
if(alarmActive == 0 && alarmStatus == 0)
{
activate();
}
else if( alarmActive == 1 || alarmStatus == 1) {
deactivate();
}
}
else {
invalidCode();
}
}
///////////////////////////////display meaasge when a invalid is
entered////////////////////////////////////////
void invalidCode()
{
password.reset();
lcd.clear();
lcd.setCursor(0,0);
lcd.print("INVALID CODE");
lcd.setCursor(0,1);
lcd.print("TRY AGAIN!");
Serial.println("INVALID CODE, TRY AGAIN!");
delay(2000);
displayCodeEntryScreen();
}
////////////////Activate the system if correct PIN entered and display message on the
screen//////////////////////
void activate()
{
36lcd.clear();
lcd.setCursor(4,0);
lcd.print("Security");
lcd.setCursor(0,1);
lcd.print("System Active");
Serial.println("Security System Active");
alarmActive = 1;
password.reset();
delay(5000);
displayCodeEntryScreen();
digitalWrite(greenLedPin, LOW);
}
////////////////////////////////////Alarm triggered when Motion
Detected/////////////////////////////
void alarmTriggered()
{
alarmStatus = 1;
lcd.clear();
lcd.setCursor(0,0);
lcd.print(" SYSTEM TRIGGERED ");
Serial.println(" SYSTEM TRIGGERED ");
delay(1000);
displayCodeEntryScreen();
SendMessage();
}
void loop()
{
//displayCodeEntryScreen();
keypad.getKey();
if (alarmActive == 1){
if (digitalRead(pirPin) == HIGH)
{
Serial.println(" Motion Detected.... ");
37digitalWrite(redLedPin, HIGH);
digitalWrite(greenLedPin, LOW);
alarmTriggered();
}
else{
Serial.println(" Motion is not Detected.... ");
digitalWrite(redLedPin, LOW);
digitalWrite(greenLedPin, LOW);
}
}
if(alarmActive == 0){
Serial.println("Home Automation Active..");
ldrValue=analogRead(ldr);
Serial.println(ldrValue);
if ((digitalRead(pirPin) == HIGH)&&(ldrValue<sensorValue)){
Serial.println(" Motion Detected.... ");
Serial.print("Light Intensity Value is:");
Serial.println(ldrValue);
Serial.println("Light On..");
digitalWrite(redLedPin, LOW);
digitalWrite(greenLedPin, HIGH);
delay(10000);
}
else{
Serial.print("Light Intensity Value is:");
Serial.println(ldrValue);
Serial.println("Light Off..");
digitalWrite(redLedPin, LOW);
digitalWrite(greenLedPin, LOW);
}
}
