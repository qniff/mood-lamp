#include <RTClib.h>

/*
   Mood light using Arduino
   Interactive Bluetooth Programming using Terminal
   Alarm function for wake up and sleep.
   Program by: B.Aswinth Raj https://circuitdigest.com/users/baswinth-raj
   For: Circuitdigest.com
   Dated: 24-9-2018
*/

/*PIN CONNECTIONS
   Bluetooth (HC-05)
   #Tx -> 11
   Rx ->10
   #DS3231 (RTC)
   SDA ->A4
   SCL ->A5
   #Neo Pixel
   Data -> pin 6
*/

#include <SoftwareSerial.h> //Bluetooth module works with software serial 
#include <Adafruit_DotStar.h> //Library for Neo Pixel (Download from Link in article)
//#include <DS3231.h> //Library for RTC module (Download from Link in article)
#include <SPI.h> //Library for SPI communication (Pre-Loaded into Arduino)
#include <EEPROM.h> //Library for EEPROM (Pre-Loaded into Arduino)

//Define the value of colours
#define Red 1
#define Orange 20
#define Yellow 30
#define Green 70
#define Cyan 100
#define Blue 130
#define Purple 200
#define Pink 240

#define NUMPIXELS 30

#define DATAPIN 6
#define CLOCKPIN 11

//Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG); //5 LEDs on PIN-6
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

//DS3231 rtc(SDA, SCL); //object for RTC module
RTC_DS3231 rtc;
DateTime t; //element t

SoftwareSerial Bluetooth(11, 10); // TX, RX


//GLOBAL variables
char incoming; //to store value form Bluetooth
int lightvalue = 0; //LDR output vale
int colour_count = 0; //to increment array

//variable to store hour and minute values
uint8_t wake_alarm_hour;
uint8_t sleep_alarm_hour;
uint8_t wake_alarm_minute;
uint8_t sleep_alarm_minute;
uint8_t current_time_hour;
uint8_t current_time_minute;

//flag variables
boolean settings = false;
boolean sleeping = false;


int      head  = 0, tail = -10; // Index of first 'on' and 'off' pixels
uint32_t color = 0xFF0000;      // 'On' color (starts red)

//array declarations
char current_rainbow[8] = {}; //the main array
char morning_rainbow[4] = {Red, Orange, Green, Pink}; //colours to show during day time
char evening_rainbow[4] = {Yellow, Cyan, Blue, Purple}; //colours to show during night time
char all_rainbow[8] = {Red, Orange, Yellow, Green, Cyan, Blue, Purple, Pink}; //colours that can be controlled thorugh bluetooth



void wait_for_reply() //Function to wait for user to enter value for BT
{
  Bluetooth.flush(); while (!Bluetooth.available());
}

void Initialize_RTC()
{
  rtc.begin();   // Initialize the rtc object

  //#### The following lines can be uncommented to set the date and time for the first time###
  //rtc.setDOW(TUESDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(13, 01, 00);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(9, 25, 2018);   // Set the date to January 1st, 2014
}

int get_hour() //get the hour value for setting alarm
{
  char UD; char LD; //upper digit and lower digit
  Bluetooth.println("Enter hours");
  wait_for_reply(); //wait for user to enter something
  UD = Bluetooth.read(); delay (100); //Read the first digit
  wait_for_reply(); //wait for user to enter something
  LD = Bluetooth.read(); //Read the lower digit

  UD = int(UD) - 48; LD = int(LD) - 48; //convert the char to int by subtracting 48 from it

  return (UD * 10) + LD; // Comine the uper digit and lowe digit to form the number which is hours
}

int get_minute() //get the minute value for setting alarm
{
  char UD; char LD; //upper digit and lower digit
  Bluetooth.println("Enter minutes");
  wait_for_reply();//wait for user to enter something
  UD = Bluetooth.read(); delay (100); //Read the first digit
  wait_for_reply();//wait for user to enter something
  LD = Bluetooth.read(); //Read the first digit

  UD = int(UD) - 48; LD = int(LD) - 48; //convert the char to int by subtracting 48 from it

  return (UD * 10) + LD; // Comine the uper digit and lowe digit to form the number which is hours
}



void setup() { //Execute once during the launch of program

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
    clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
  #endif


  
  Initialize_RTC();
  Bluetooth.begin(9600);
  Serial.begin (9600); //for debugging

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Bluetooth.println("Mood Lamp Alarm -CircuitDigest");
  Serial.println("Mood Lamp Alarm -CircuitDigest");

  //If anything was stored previously in EEPROM copy it to the alarm variables
  sleep_alarm_hour = EEPROM.read(0);
  sleep_alarm_minute = EEPROM.read(1);
  wake_alarm_hour =  EEPROM.read(2);
  wake_alarm_minute = EEPROM.read(3);

}

void loop() {

  lightvalue = analogRead(A0); //Read the value form LDR

  while (lightvalue > 800) //IF the room is very brigt (you can increase this values to make it glow even duringf day)
  {
    for (int i = 0; i <= 5; i++) //turn of all LED
    {
      strip.setBrightness(0); //by setting brightness to zero
      strip.show();
    }
    lightvalue = analogRead(A0); //kepp checking if the room is getting darker
    Serial.print("Too bright to glow: ");
    Serial.println(lightvalue); //for debugging
    Interactive_BT();  //Also check if the user is trying to access through bluetooth
    delay(100);
  }


  settings = true; //if setting is true it means we have are ready to get into bluetooth control
  //  Interactive_BT(); //Also check if the user is trying to access through bluetooth


  t = rtc.now(); //get the current time
  current_time_hour = t.hour(); //get hour value
  current_time_minute = t.minute(); //get minute valued

  if (t.second() % 5 == 0) //For every 5 seconds
  {

    colour_count++; //change the colour
    if (colour_count >= 4) //if we exceed array count
      colour_count = 0; //initialise the count

    while (t.second() % 5 == 0) //wait till the 5th secound is over
      t = rtc.now(); //update t.sec

    //For Debugging
    Serial.print ("Glowing clour: "); Serial.println(colour_count);
    Serial.print("At time: "); Serial.print(t.hour()); Serial.print (" : "); Serial.println (t.minute());
    Serial.print("Enviroment Brightness: "); Serial.println(lightvalue);
    Serial.print("Wake up at :"); Serial.print(wake_alarm_hour); Serial.print (" : "); Serial.println (wake_alarm_minute);
    Serial.print("Sleep at :"); Serial.print(sleep_alarm_hour); Serial.print (" : "); Serial.println (sleep_alarm_minute);
    Serial.print ("Is Lamp sleeping? : "); Serial.println (sleeping);
    Serial.println("                   ****                ");
    //End of debugging lines

    if (sleeping == false) //If we are not sleeping
      glow_rainbow(colour_count); //dsplay the colours
    if (sleeping == true) //if we are sleeping
      night_lamp(); //display the night lamp effect

    if (t.hour() >= 17) //During evening time
    {
      for (int i = 0; i <= 3; i++)
      {
        current_rainbow[i] = evening_rainbow[i];  //copy evening raninbow into current_rainbow
        delay(100);
      }
    }
    else //During Morning
    {
      for (int i = 0; i <= 3; i++)
      {
        current_rainbow[i] = morning_rainbow[i];  //copy  mornign rainboe into current rainbow
        delay(100);
      }
    }

  }



  if (t.hour() == sleep_alarm_hour && t.minute() == sleep_alarm_minute) //If  the sleep time is meat
  {
    sleeping = true;  //get into sleeping mode
    Serial.println("Lamp getting into Sleep Mode");
  }

  if (t.hour() == wake_alarm_hour && t.minute() == wake_alarm_minute) // If wake up time is meat
  {
    sleeping = false;  // get out of sleeping mode.
    Serial.println("Lamp is up and ready");
  }



}



void Interactive_BT() //using this funciton the user cna control LED colour and set alarm time.
{

  if (Bluetooth.available() > 0 && settings == true) { //if the user has sent something
    Serial.println("here");
    incoming = Bluetooth.read(); //read and clear the stack
    Serial.println(incoming);
    Bluetooth.println("0-> Set Alarm "); Bluetooth.println("1 -> Control Lamp"); Bluetooth.println("x -> Exit Anytime"); //Display the options
    wait_for_reply();
    incoming = Bluetooth.read(); //read what the user has sent
    //Based on user request
    if (incoming == '0') //if user sent 0
    {
      Bluetooth.println("Setting Alarm"); Bluetooth.println("0-> Sleep alarm"); Bluetooth.println("1 -> Wake up Alarm"); //give alarm options
      wait_for_reply();
      incoming = Bluetooth.read();

      if (incoming == '0')
      {
        Bluetooth.println("Go to sleep at:"); sleep_alarm_hour = get_hour(); sleep_alarm_minute = get_minute();
        Bluetooth.print("Sleep alarm set at: "); Bluetooth.print(sleep_alarm_hour); Bluetooth.print(" : "); Bluetooth.println(sleep_alarm_minute);
        EEPROM.write(0, sleep_alarm_hour);  EEPROM.write(1, sleep_alarm_minute);
      }

      if (incoming == '1')
      {
        Bluetooth.println("Wake me at:"); wake_alarm_hour = get_hour(); wake_alarm_minute = get_minute();
        Bluetooth.print("Wake up alarm set at: "); Bluetooth.print(wake_alarm_hour); Bluetooth.print(" : "); Bluetooth.println(wake_alarm_minute);
        EEPROM.write(2, wake_alarm_hour);  EEPROM.write(3, wake_alarm_minute);
      }
      incoming = 'x';
    }

    if (incoming == '1')
    {
      Bluetooth.println("Select the colour you like");
      Bluetooth.println("0-> Red"); Bluetooth.println("1-> Orange"); Bluetooth.println("2-> Yellow"); Bluetooth.println("3-> Green"); Bluetooth.println("4-> Cyan");
      Bluetooth.println("5-> Blue"); Bluetooth.println("6-> Purple"); Bluetooth.println("7-> Pink");

      do {
        wait_for_reply();
        incoming = Bluetooth.read();

        memcpy(current_rainbow, all_rainbow, 8);

        glow_rainbow(incoming - 48);
        Serial.println(incoming - 48);
      } while (incoming != 'x');

      Bluetooth.println("Exiting control mode");
    }

    if (incoming == 'x') //exit from Bluetooth mode
    {
      Bluetooth.flush();
      incoming = Bluetooth.read();
      incoming = 0;
      settings = false;
      Bluetooth.println("Back to main");
    }

  } else {

    Serial.println("user is not sending");
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color( (255 - WheelPos * 3), 0, (WheelPos * 3) );
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, (WheelPos * 3) , (255 - WheelPos * 3) );
  }
  WheelPos -= 170;
  return strip.Color((WheelPos * 3), (255 - WheelPos) * 3, 0);
}

void glow_rainbow(int count)
{
  for (int j = 150; j >= 10; j--) //decrease the brightness to create dim effect
  {
    for (int i = 0; i <= 5; i++) //do it for all 5 leds
    {
      strip.setBrightness(j);
      strip.show();
    }
    delay(2);
  }


  for (int j = 0; j <= 255; j++) //increase the brightness
  {
    for (int i = 0; i <= 5; i++) //do it for all 5 leds
    {
      strip.setPixelColor(i, Wheel(current_rainbow[count])); //select the colour based on count value
      strip.setBrightness(j);
      strip.show();
    }
    delay(10);
  }
}

void night_lamp()
{
  for (int j = 240; j <= 254; j++) //decrease the brightness to create dim effect
  {
    for (int i = 0; i <= 5; i++) //do it for all 5 leds
    {
      strip.setPixelColor(i, 255 - j, 0, 255 - j);
      strip.show();
    }
    delay(300);
  }
  for (int j = 254; j >= 240; j--) //decrease the brightness to create dim effect
  {
    {
      for (int i = 0; i <= 5; i++) //do it for all 5 leds
      {
        strip.setPixelColor(i, 255 - j, 0, 255 - j);
        strip.show();
      }
      delay(300);
    }
  }
}

void test_lamp()
{
  strip.setPixelColor(head, color); // 'On' pixel at head
  strip.setPixelColor(tail, 0);     // 'Off' pixel at tail
  strip.show();                     // Refresh strip
  delay(20);                        // Pause 20 milliseconds (~50 FPS)

  if(++head >= NUMPIXELS) {         // Increment head index.  Off end of strip?
    head = 0;                       //  Yes, reset head index to start
    if((color >>= 8) == 0)          //  Next color (R->G->B) ... past blue now?
      color = 0xFF0000;             //   Yes, reset to red
  }
  if(++tail >= NUMPIXELS) tail = 0; // Increment, reset tail index
}
