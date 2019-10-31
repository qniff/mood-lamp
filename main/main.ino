#include <RTClib.h>
#include <SoftwareSerial.h> //Bluetooth module works with software serial 
#include <Adafruit_DotStar.h> //Library for Neo Pixel (Download from Link in article)
#include <SPI.h> //Library for SPI communication (Pre-Loaded into Arduino)
#include <EEPROM.h> //Library for EEPROM (Pre-Loaded into Arduino)
#include <ctype.h>
#include "Time.h"

#define NUMPIXELS 30

#define BUZZERPIN 9
#define DATAPIN 6
#define CLOCKPIN 11
#define MAX_INPUT 50

Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

RTC_DS3231 rtc;
DateTime t;

SoftwareSerial Bluetooth(4, 2); // RX, TX

//GLOBAL variables
String alarmHour;
String alarmMinute;
bool alarmEnabled = false;
bool rainbowEnabled = false;
bool settingHour = false;
bool settingMinute = false;
bool buzzing = false;
int colour_count = 0;

//main color of strip
int r = 0;
int g = 0;
int b = 0;

//melody
int length = 15; // the number of notes
char notes[] = "ccggaagffeeddc "; // a space represents a rest
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;

int      head  = 0, tail = -10;
uint32_t color = 0xFF0000;      // 'On' color (starts red)



void setup() { //Execute once during the launch of program

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
    clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
  #endif

  pinMode(BUZZERPIN, OUTPUT);
  
  rtc.begin();
  
  Serial.begin(9600);
  delay(3000);
  
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  strip.begin();
  strip.show();
  changeColor(0, 0, 0);
  
  printInfo();
  

}

void loop() {

  handleBluetooth();

  if(rainbowEnabled){
    DateTime now = rtc.now();
    if(now.second()%2 == 0){
      rainbow();
    }
  }

  if(alarmEnabled){
    doAlarm();
  }
}


void handleBluetooth(){
    static char input_line [MAX_INPUT];
    static unsigned int input_pos = 0;
    

    if (Serial.available () > 0)
    {
      char inByte = Serial.read();
      switch (inByte)
      {
        case '\n':   // end of text
          input_line [input_pos] = 0;  // terminating null byte
          
          // terminator reached! process input_line here ...
          
          if(settingMinute){
            Serial.println("Setting minute");
            setMinute(input_line);
          }

          if(settingHour){
            Serial.println("Setting hour");
            setHour(input_line);
          }

          executeCommand(input_line);
          
          // reset buffer for next time
          input_pos = 0;  
          break;

        case '\r':   // discard carriage return
          break;

        default:
          // keep adding if not full ... allow for terminating null byte
          if (input_pos < (MAX_INPUT - 1))
              input_line [input_pos++] = inByte;
          break;
      }  // end of switch
    }
}


void executeCommand(String text){
  if(text == "alarm"){
    handleAlarm();
  }
  
  if(text == "red"){
    Serial.println("\nChanging to red..");
    changeColor(255, 0, 0);
  }
  
  if(text == "green"){
    Serial.println("\nChanging to green..");
    changeColor(0, 255, 0);
  }
    
  if(text == "blue"){
    Serial.println("\nChanging to blue..");
    changeColor(0, 0, 255);
  }  
  
  if(text == "purple"){
    Serial.println("\nChanging to purple..");
    changeColor(128, 0, 128);
  }
  
  if(text == "pink"){
    Serial.println("\nChanging to pink..");
    changeColor(255, 192, 203);
  }
  
  if(text == "purple"){
    Serial.println("\nChanging to purple..");
    changeColor(128, 0, 128);
  }   
  
  if(text == "yellow"){
    Serial.println("\nChanging to yellow..");
    changeColor(255, 255, 0);
  }

  if(text == "cyan"){
    Serial.println("\nChanging to cyan..");
    changeColor(0, 255, 255);
  }
  
   if(text == "orange"){
    Serial.println("\nChanging to orange..");
    changeColor(255, 165, 0);
  }  

   if(text == "rainbow"){
    Serial.println("\nChanging to rainbow..");
    rainbowEnabled = true;
  }
  
  if(text == "help"){
    printInfo();
  }

   if(text == "off"){
    changeColor(0, 0, 0);
    Serial.println("\nTurning off..");
  }

   if(text == "melody"){
    playMelody();
  }
}

void handleAlarm(){
    if(alarmEnabled){
      alarmEnabled = false;
      Serial.println("\nAlarm is unset");
      return;
    }
    Serial.println("\nAlarm menu:");
    Serial.println("What hour do you want to wake up? (24h):");
    settingHour = true;
    handleBluetooth();
}

void setHour(String text){
    alarmHour = text;
    Serial.println("\nWhat minute do you want to wake up?");
    settingHour = false;
    settingMinute = true;
    handleBluetooth();
}

void setMinute(String text){
    alarmMinute = text;
    settingMinute = false;
    alarmEnabled = true;
    Serial.println("\nAlarm is set to " + alarmHour + ":" + alarmMinute);
    Serial.println("Use 'alarm' command to turn it off (Helpful in the morning)");
}

void changeColor(int green, int red, int blue)
{
  rainbowEnabled = false;
  while ( r != red || g != green || b != blue ) {
    if ( r < red ) r += 1;
    if ( r > red ) r -= 1;

    if ( g < green ) g += 1;
    if ( g > green ) g -= 1;

    if ( b < blue ) b += 1;
    if ( b > blue ) b -= 1;

    for (int i = 0; i <= 30; i++) //do it for all leds
    {
      strip.setPixelColor(i, r, g, b);
      strip.show();
    }
  }
}

void doAlarm(){
  DateTime now = rtc.now();
  String hour = String(now.hour(), DEC);
  String minute = String(now.minute(), DEC);
  if((hour == alarmHour) && (minute == alarmMinute)){
    Serial.println("!!!ALARM!!!");
    executeCommand("red");
    rainbowEnabled = true;
    playMelody();
  }
}

void playMelody(){
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } else {
      playNote(notes[i], beats[i] * tempo);
    }

    // pause between notes
    delay(tempo / 2); 
  }
}

void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(BUZZERPIN, HIGH);
    delayMicroseconds(tone);
    digitalWrite(BUZZERPIN, LOW);
    delayMicroseconds(tone);
  }
}

void rainbow()
{
  for(int x = 0; x < NUMPIXELS * 3; x++){

    strip.setPixelColor(head, color); // 'On' pixel at head
    strip.setPixelColor(tail, 0);     // 'Off' pixel at tail
    strip.show();                     // Refresh strip
    delay(30);                        // Pause 20 milliseconds (~50 FPS)
  
    if(++head >= NUMPIXELS) {         // Increment head index.  Off end of strip?
      head = 0;                       //  Yes, reset head index to start
      if((color >>= 8) == 0)          //  Next color (R->G->B) ... past blue now?
        color = 0xFF0000;             //   Yes, reset to red
    }
    if(++tail >= NUMPIXELS) tail = 0; // Increment, reset tail index
  }
  
}

void printInfo()
{
    Serial.println("\nAvailable commands: ");
    Serial.println("Color: rainbow, red, green, blue, purple, yellow, cyan, orange, off");
    Serial.println("Other: alarm, melody, help");
}

 

 
