/* 
 * Project Mid Term Room Controller
 * Author: Dillon Davis
 * Date: 3/5/25
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
//#include "Sunrise.h"
#include "IoTClassroom_CNM.h"
#include "wemo.h"
#include "Adafruit_BME280.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "RTClibrary.h"
#include "Keypad_Particle.h"

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int BMEADDRESS = 0x76;
const int BUTTONPIN = D2;
const int LIGHT = 3;
const int HEATER = 2;
const int FAN = 1;
unsigned int currentTime;
unsigned int lastTime = 0;
float tempC, pressPA, humidRH, tempF, pressInHg, lastSecond, lastMinute, lastDaySecond, lastNightSecond; 
float lastLightOnSecond, lastLightOffSecond;
const int OLED_RESET = -1;
byte status;
const char DEG = 248;
const char PER = 37;
const byte ROWS = 4;
const byte COLS = 4;
bool manual = false;
bool buttonClick;
char customKey;
int count;
int count2;
int count3;
int count4;
int count5;
int count6;
int startHour = 7, endHour = 18, startMin = 0, endMin = 0;
int brightness;
bool lightPower;
 

char hexaKeys [ ROWS ][ COLS ] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
  };
 
//  char key[4] = {'0', '1', '0', '6'};
//  char enteredKey[4];
 
 byte rowPins [ ROWS ] = {D8 ,D9 , D16 , D15 }; // 1st to 4th Keypad pins ( starting on left )
 byte colPins [ COLS ] = { D17 , D18 , D19 , D14 }; // 5th to 8th Keypad pins 
 
 Keypad customKeypad = Keypad ( makeKeymap ( hexaKeys ) , rowPins , colPins , ROWS , COLS );
 Adafruit_SSD1306 display(OLED_RESET);
 Button button(BUTTONPIN);
 Adafruit_BME280 bme;
 RTC_DS3231 rtc;
 DateTime now;
 IoTTimer brightTime;
 
 void goManual();
 void goAuto();
 void lightOn();
 void lightOff();
 void showInfo();
 void dayMode();
 void nightMode();
// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(MANUAL);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// setup() runs once, when the device is first turned on
void setup() {
  // Put initialization like pinMode and begin functions here
  count = 0;
  count2 = 0;
  count3 = 0;
  count4 = 0;
  Serial.begin(9600);
  waitFor(Serial.isConnected, 15000);

  WiFi.on();
  WiFi.clearCredentials();
  WiFi.setCredentials("IoTNetwork");

  WiFi.connect();
  while(WiFi.connecting()){
    Serial.printf(".");
  }
  Serial.printf("\n\n");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  display.clearDisplay();
  display.display();
  
  status = bme.begin(BMEADDRESS);
  if ( status == false ) {
    Serial.printf("BME280 at address 0x%02X failed to start \n", BMEADDRESS);
  }

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  buttonClick = false;
  wemoWrite(FAN, LOW);
  wemoWrite(HEATER, LOW);
}


// loop() runs over and over again, as quickly as it can execute.
void loop() {
  

  tempC = bme.readTemperature (); // deg C
  pressPA = bme.readPressure (); // pascals
  humidRH = bme.readHumidity (); //%RH
  
  tempF = tempC*(9.0/5)+32.0; 
  pressInHg = pressPA / 3386.39;
  
  now = rtc.now();
  
  if(button.isClicked()){
    buttonClick = !buttonClick;
    Serial.printf("%i\n",buttonClick);
  }

  if(!buttonClick){
    startHour = 14;
    startMin = 00;
    endHour = 15;
    endMin = 00; 
    manual = false;
    if(now.minute() >= startMin){
      if((now.hour() >= startHour  && now.hour() <= endHour)){
        dayMode();
    }
    if((now.hour() < startHour) || (now.hour() > endHour)){
      nightMode();
    }
  }
}
  if(buttonClick){
    manual = true;
    //count2 = 0;
    //currentTime = millis();
    while(count2 < 1){
      Serial.printf("in the while loop");
      goManual();
      count2++;
    }
    if(now.minute() >= startMin){
      if((now.hour() >= startHour  && now.hour() <= endHour)){
        dayMode();
    }
    if((now.hour() < startHour) || (now.hour() > endHour)){
      nightMode();
    }
    
  }
  
  
  //Serial.printf("Start hour: %i\n", startHour);
  
}
}

void goManual(){
    int keyNum[4];
    int twice = 0;
    while(twice < 2){
      count = 0;
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(2);
      display.printf("  MANUAL");
      display.setTextSize(1);
      if(twice == 0){
        display.printf("\n\nEnter the ON time!\nON: ");
      }
      if(twice == 1){
        display.printf("\n\nEnter the OFF time!\nOFF: ");
      }
      display.display();
    
      while(count < 4){
        customKey = customKeypad.getKey();
        if(customKey){
          switch (customKey)
          {
          case '0':
            keyNum[count] = 0;
            break;
          case '1':
            keyNum[count] = 1;
            break;
          case '2':
            keyNum[count] = 2;
            break;
          case '3':
            keyNum[count] = 3;
            break;
          case '4':
            keyNum[count] = 4;
            break;
          case '5':
            keyNum[count] = 5;
            break;
          case '6':
            keyNum[count] = 6;
            break;
          case '7':
            keyNum[count] = 7;
            break;
          case '8':
            keyNum[count] = 8;
            break;
          case '9':
            keyNum[count] = 9;
            break;  
          }
          Serial.printf("%c     %i\n", customKey, keyNum[count]);
          display.printf("%i", keyNum[count]);
          display.display();
          count++;
        }
      }
      if(twice == 0){
        String s1, s2, s3;
        s1 = String(keyNum[0]);
        s2 = String(keyNum[1]);
        s3 = s1 + s2;
        startHour = atoi(s3.c_str());
        s1 = String(keyNum[2]);
        s2 = String(keyNum[3]);
        s3 = s1 + s2;
        startMin = atoi(s3.c_str());
      }
      if(twice == 1){
        String s1, s2, s3;
        s1 = String(keyNum[0]);
        s2 = String(keyNum[1]);
        s3 = s1 + s2;
        endHour = atoi(s3.c_str());
        s1 = String(keyNum[2]);
        s2 = String(keyNum[3]);
        s3 = s1 + s2;
        endMin = atoi(s3.c_str());
      }
      twice++;
    }
    
    
    
    display.printf("%i:%i",startHour, startMin);
    display.display();  
   
}

void goAuto(){
  
}

void lightOn(){
  if(!lightPower){
    Serial.printf("Turning Light ON");
    //currentTime = millis();
    if((millis() - lastLightOnSecond)> 1000)
    { 
      lastLightOnSecond = millis();
      Serial.printf("Light ON\n");
      setHue(LIGHT, true, HueOrange, brightness, 255);
      brightness++;
      if(brightness >= 256){
        brightness = 0;
        lightPower = true;
      }
    }
  }
}

void lightOff(){
  brightness = 256;
  if(brightness > 0){
    //currentTime = millis();
    if((millis() - lastLightOffSecond)> 1000)
    { 
      lastLightOffSecond = millis();
      setHue(LIGHT, true, HueOrange, brightness, 255);
      brightness--;
    }
  }
}


void showInfo(){
    //lastSecond = millis();
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.printf("  ");
    display.print(daysOfTheWeek[now.dayOfTheWeek()]);
    display.printf("\n %i:%i:%i", now.hour(), now.minute(), now.second());
    Serial.printf("\n %i:%i:%i", now.hour(), now.minute(), now.second());
    display.printf("\n%.0f%c || %.0f%c", tempF, DEG, humidRH, PER);
    if(!manual){
      display.setTextSize(1);
      display.printf("\n AUTO");
    }
    else{
      display.setTextSize(1);
      display.printf("\n MANUAL");
    }
    if(tempF < 70 ){
      display.setTextSize(1);
      display.printf("      HEAT ON");
    }
    if(tempF > 70){
      display.setTextSize(1);
      display.printf("        FAN ON");
    }
    display.display();
    
}

void dayMode(){
  //currentTime = millis();
  if (( millis() - lastDaySecond ) > 1000) {
    lastDaySecond = millis();
    Serial.printf("DAY MODE\n");
    showInfo();
  }
  if(count3 < 1){
    lightPower = false;
    if(tempF < 70){
        wemoWrite(HEATER, HIGH);
        wemoWrite(FAN, LOW);
      }
      else{
        wemoWrite(HEATER, LOW);
        wemoWrite(FAN, HIGH);
      }
      Serial.printf("Heater ON\nFan OFF\n");
      wemoWrite(FAN, LOW);
      brightTime.startTimer(1000);
      count3++;
      lightOn();
      }
  

 }
  


void nightMode(){
  //currentTime = millis();
    if (( millis() - lastNightSecond ) > 1000) {
      lastNightSecond = millis();
      Serial.printf("NIGHT MODE\n");
      showInfo();
    }
  if(count4 < 1){
    wemoWrite(HEATER, LOW);
    if(tempF > 65){
      wemoWrite(FAN, HIGH);
    }
    else{
      wemoWrite(FAN, LOW);
    }
    Serial.printf("Heater OFF\nFan ON\n");
    lightOff();
    count4++;
  }
  
  
 }

