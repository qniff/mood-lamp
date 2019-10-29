#include <RTClib.h>
#include <SoftwareSerial.h> //Bluetooth module works with software serial 
#include <Adafruit_DotStar.h> //Library for Neo Pixel (Download from Link in article)
#include <SPI.h> //Library for SPI communication (Pre-Loaded into Arduino)
#include <EEPROM.h> //Library for EEPROM (Pre-Loaded into Arduino)

#define NUMPIXELS 30

#define DATAPIN 6
#define CLOCKPIN 11
#define MAX_INPUT 50

Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

RTC_DS3231 rtc;
DateTime t;

SoftwareSerial Bluetooth(4, 2); // RX, TX

//GLOBAL variables
char incoming; //store value form Bluetooth
int lightvalue = 0; //LDR output vale
int colour_count = 0; //to increment array

//main color of strip
int r = 0;
int g = 0;
int b = 0;


void setup() { //Execute once during the launch of program

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
    clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
  #endif
  
  rtc.begin();

  Bluetooth.begin(9600);
  Serial.begin(9600);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  changeColor(0, 0, 0);
  printInfo();
}

void loop() {
  handleBluetooth();
}


void handleBluetooth(){
    static char input_line [MAX_INPUT];
    static unsigned int input_pos = 0;
    

    if (Serial.available () > 0)
    {
      char inByte = Serial.read ();
      switch (inByte)
      {
        case '\n':   // end of text
          input_line [input_pos] = 0;  // terminating null byte
          
          // terminator reached! process input_line here ...
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
  if(text == "auto"){
    Serial.println("Some RTC stuff will happen in here");
  }
  
  if(text == "alarm"){
    Serial.println("Some alarm stuff might happen in here");
    handleAlarm();
  }
  
  if(text == "red"){
    changeColor(255, 0, 0);
    Serial.println("Changing to red..");
  }
  
  if(text == "green"){
    changeColor(0, 255, 0);
    Serial.println("Changing to green..");
  }
    
  if(text == "blue"){
    changeColor(0, 0, 255);
    Serial.println("Changing to blue..");
  }  
  
  if(text == "purple"){
    changeColor(128, 0, 128);
    Serial.println("Changing to purple..");
  }
  
  if(text == "pink"){
    changeColor(255, 192, 203);
    Serial.println("Changing to pink..");
  }
  
  if(text == "purple"){
    changeColor(128, 0, 128);
    Serial.println("Changing to purple..");
  }   
  
  if(text == "yellow"){
    changeColor(255, 255, 0);
    Serial.println("Changing to yellow..");
  }

  if(text == "cyan"){
    changeColor(0, 255, 255);
    Serial.println("Changing to cyan..");
  }
  
   if(text == "orange"){
    changeColor(255, 165, 0);
    Serial.println("Changing to cyan..");
  }
  
  if(text == "help"){
    printInfo();
  }

   if(text == "off"){
    changeColor(0, 0, 0);
    Serial.println("Turning off..");
  }
}

void handleAlarm(){
    Serial.println("Alarm menu:");
    Serial.println("Alarm menu:");
}

void changeColor(int green, int red, int blue)
{
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

void printInfo()
{
    Serial.println("Available commands: ");
    Serial.println("Colors: auto, red, green, blue, purple, yellow, cyan, orange, off");
    Serial.println("Other: alarm, help");
}

 
