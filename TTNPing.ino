/*  
 * ------------------------------------------------------------------------
 * "PH2LB LICENSE" (Revision 1) : (based on "THE BEER-WARE LICENSE" Rev 42) 
 * <lex@ph2lb.nl> wrote this file. As long as you retain this notice
 * you can do modify it as you please. It's Free for non commercial usage 
 * and education and if we meet some day, and you think this stuff is 
 * worth it, you can buy me a beer in return
 * Lex Bolkesteijn 
 * ------------------------------------------------------------------------ 
 * Filename : TTNPing.ino  
 * Version  : 1.0 (BETA)
 * ------------------------------------------------------------------------
 * Description : A low power ping for the ThingsNetwork.
 *  with deepsleep support and variable interval
 * ------------------------------------------------------------------------
 * Revision : 
 *  - 2016-nov-11 1.0 first "beta"
 * ------------------------------------------------------------------------
 * Hardware used : 
 *  - Arduino Nano
 *  - RN2483 
 * ------------------------------------------------------------------------
 * Software used : 
 *  - Modified TheThingsNetwork library (for deepsleep support) check
 *    my github 
 *  - LowPower library
 * ------------------------------------------------------------------------ 
 * TODO LIST : 
 *  - add more sourcode comment
 * ------------------------------------------------------------------------ 
 * TheThingsNetwork Payload functions : 
 * 
 * DECODER : 
 * 
 * function (bytes) {
 *  var batt = bytes[0] / 10.0;
 * 
 *  if (bytes.length >= 2)
 *  {
 *    var sw2 = (bytes[1] & 0x02) > 0;
 *    var sw3 = (bytes[1] & 0x04) > 0;
 *    var sw4 = (bytes[1] & 0x08) > 0;
 *    var sw = bytes[1];
 *  }  
 *   
 *  return {
 *    batt: batt, 
 *    sw2: sw2,
 *    sw3: sw3,
 *    sw4: sw4,
 *    sw: sw,
 *    bytes: bytes
 *  };
 * }
 *
 */  

#include <TheThingsNetwork.h>
#include <SoftwareSerial.h> 
#include <LowPower.h>

// Set your AppEUI and AppKey for OTAA
const byte appEui[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const byte appKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const byte devAddr[4] = {0x00, 0x00, 0x00, 0x00};
const byte appSKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const byte nwkSKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


SoftwareSerial loraSerial(10, 11); // RX, TX

#define debugSerial Serial

#define debugPrintLn(...) { if (debugSerial) debugSerial.println(__VA_ARGS__); }
#define debugPrint(...) { if (debugSerial) debugSerial.print(__VA_ARGS__); }

// define IO pins
#define LED         13  // D13
#define SW2         2   // D02
#define SW3         3   // D03
#define SW4         4   // D04
#define RN2483RESET 12  // D12
#define BATTADC     3   // A03  

#define BURSTINTERVAL   10    // 10 seconds
#define FASTINTERVAL    30    // 30 seconds
#define NORMALINTERVAL  60    // 60 seconds
#define SLOWINTERVAL    900   // 15 minutes

// global variables 
TheThingsNetwork ttn(loraSerial, debugSerial, TTN_FP_EU868 );  

bool useLowPower = false;

// general functions
void led_on()
{
  digitalWrite(LED, HIGH);
}

void led_off()
{
  digitalWrite(LED, LOW);
} 

// the arduino setup
void setup()
{ 
  analogReference(EXTERNAL);
  //setup outputs
  pinMode(LED, OUTPUT); 
  led_on();
  debugSerial.begin(115200);
  loraSerial.begin(57600); 
  debugPrintLn(F("Startup"));
  // allow the ttn client use autobaud on reset
  // ttn.baudRate= 9600;
  //reset rn2483
  pinMode(RN2483RESET, OUTPUT);
  digitalWrite(RN2483RESET, LOW);
  delay(500);
  digitalWrite(RN2483RESET, HIGH);
  delay(1000); 

 
  //the device will attempt a join every 5 second till the join is successfull
//  while (!ttn.join(appEui, appKey)) {
//    delay(5000);
//  }

  ttn.personalize(devAddr, nwkSKey, appSKey);

  led_off(); //turn on LED to confirm join
  // show status on debug.
  delay(5000);
  ttn.showStatus();
  debugPrintLn(F("Setup for The Things Network complete"));
  // give it a little time.
  delay(1000);
}

void SendPing()
{
  for (int i = 0; i < 4; i++)
  {
    // blinck 4 times to indicate that we will transmit.
    led_on();
    delay(250);
    led_off();
    delay(250);
  }
  // indicate that we are busy
  led_on(); 
  
  int batt = analogRead(BATTADC); // max 1023 = 6.6V/2 because ref = 3.3V resize to 0...66
  debugPrint(F("batt = "));
  debugPrintLn(batt);
  unsigned int batvaluetmp = batt * 66;
  batvaluetmp = batvaluetmp / 1023;
  byte batvalue = (byte)batvaluetmp; // no problem putting it into a int.
  debugPrint(F("batvalue = "));
  debugPrintLn(batvalue); 
  int sw2 = digitalRead(SW2);
  int sw3 = digitalRead(SW3);
  int sw4 = digitalRead(SW4);
  debugPrint(F("SW2 = "));
  debugPrintLn(sw2);
  debugPrint(F("SW3 = "));
  debugPrintLn(sw3);
  debugPrint(F("SW4 = "));
  debugPrintLn(sw4);

  byte data[2];
  data[0] = batvalue; 
  data[1] = sw2 << 1 | sw3 << 2 | sw4 << 3;
  ttn.sendBytes(data, sizeof(data));
 
  led_off();
}
  
// the loop routine runs over and over again forever:
void loop()
{ 
  // send ping (batt, temp, humd)
  SendPing(); 
 
  int sw2 = digitalRead(SW2);
  int sw3 = digitalRead(SW3);
  int sw4 = digitalRead(SW4);
  int interval = BURSTINTERVAL;   // 10 seconds
  useLowPower = (sw4 == LOW);   // default low power.
   
  if (sw2 == HIGH && sw3 == LOW)
  {
    interval = FASTINTERVAL;    // 30 seconds
  }
  else if (sw2 == LOW && sw3 == HIGH) 
  {
    interval = NORMALINTERVAL;  // 60 seconds 
  }
  else if (sw2 == HIGH && sw3 == HIGH)
  {
    interval = SLOWINTERVAL;    // 15 minutes 
  }
  
  if (useLowPower)
  { 
    ttn.deepSleep((interval + 10)*1000);
  }
  for (int i = 0; i < interval; i++)
  { 
    if (useLowPower)
    {  
      // Enter power down state for 8 s with ADC and BOD module disabled
      LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);   
    }
    else
    {
      led_on();
      delay(200);
      led_off();
      delay(800); 
    } 
  } 
  if (useLowPower)
  { 
    ttn.wakeUp();
  }
}

