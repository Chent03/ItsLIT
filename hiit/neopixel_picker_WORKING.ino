#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>

#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#include <Adafruit_NeoPixel.h>

#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define FACTORYRESET_ENABLE     1
#define BLUEFRUIT_HWSERIAL_NAME Serial1
#define PIN                     6
#define NUMPIXELS               8


Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUMPIXELS, PIN);
Adafruit_LSM303 lsm;
Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

int DELTA_X = 3000;
int DELTA_Y = 3000;

int INIT_X_VALUE = 0;
int INIT_Y_VALUE = 0;
int PREV_X_VALUE = 0;
int PREV_Y_VALUE = 0;
int CURRENT_NUMBER_OF_STEPS = 0;
int STEPS_GOAL = 5; // 1 mile = 2000 steps
bool FIRST_ENTRY = true;
bool SHOW_FIRST_COLOR = true;


// A small helper
void error(const __FlashStringHelper*err) {
//  Serial.println(err);
  while (1);
}

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];



void setup(void)
{
//  while (!Serial);  // required for Flora & Micro
  delay(500);

  // turn off neopixel
  pixel.begin(); // This initializes the NeoPixel library.
  for(uint8_t i=0; i<NUMPIXELS; i++) {
    pixel.setPixelColor(i, pixel.Color(0,0,0)); // off
  }
  pixel.show();

  Serial.begin(115200);
//  Serial.println(F("Adafruit Bluefruit Neopixel Color Picker Example"));
//  Serial.println(F("------------------------------------------------"));

  /* Initialise the module */
//  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
//    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
//  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
//    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
//      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

//  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

//  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
//  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
//  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

//  Serial.println(F("***********************"));

  // Set Bluefruit to DATA mode
//  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

//  Serial.println(F("***********************"));

  if (!lsm.begin())
  {
//    Serial.println("Oops ... unable to initialize the LSM303. Check your wiring!");
    while (1);
  }

}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  lsm.read();
  Serial.print("Accel X: "); Serial.print((int)lsm.accelData.x); Serial.print(" ");
  Serial.print("Y: "); Serial.print((int)lsm.accelData.y);       Serial.print(" ");
  Serial.print("Z: "); Serial.println((int)lsm.accelData.z);     Serial.print(" ");

  // X - back-forward, Y - up-down, Z - ignore haha

  int X = (int)lsm.accelData.x;
  int Y = (int)lsm.accelData.y;

  if (FIRST_ENTRY) {
    
    FIRST_ENTRY = false;
    
    INIT_X_VALUE = X;
    INIT_Y_VALUE = Y;
    PREV_X_VALUE = X;
    PREV_Y_VALUE = Y;
        
  } else {

    int delta_X = X - PREV_X_VALUE;
    int delta_Y = Y - PREV_Y_VALUE;
    if (delta_X < 0) delta_X *= -1;
    if (delta_Y < 0) delta_Y *= -1;

    // STEP ++
    if (DELTA_X < delta_X && DELTA_Y < delta_Y) {
      CURRENT_NUMBER_OF_STEPS += 1;
      ble.print("Current number of STEPS = "); ble.println(CURRENT_NUMBER_OF_STEPS);
//      Serial.print("STEP++ :"); Serial.println(CURRENT_NUMBER_OF_STEPS);
    }

    if (CURRENT_NUMBER_OF_STEPS >= STEPS_GOAL) {
      CURRENT_NUMBER_OF_STEPS = 0;
      ble.println("1 mile completed.");
      pixel.begin(); // This initializes the NeoPixel library.
      for(uint8_t i=0; i<NUMPIXELS; i++) {
        if (SHOW_FIRST_COLOR) {
          pixel.setPixelColor(i, pixel.Color(0,0,0x10)); // blue
        } else {
          pixel.setPixelColor(i, pixel.Color(0,0x10,0)); // green
        }
      }
      pixel.show();
      SHOW_FIRST_COLOR = !SHOW_FIRST_COLOR;
    }
    
  }
  
  
  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;

  /* Got a packet! */
  // printHex(packetbuffer, len);

  // Color
  if (packetbuffer[1] == 'C') {
    uint8_t red = packetbuffer[2];
    uint8_t green = packetbuffer[3];
    uint8_t blue = packetbuffer[4];
//    Serial.print ("RGB #");
    if (red < 0x10) Serial.print("0");
//    Serial.print(red, HEX);
    if (green < 0x10) Serial.print("0");
//    Serial.print(green, HEX);
    if (blue < 0x10) Serial.print("0");
//    Serial.println(blue, HEX);

    for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixel.setPixelColor(i, pixel.Color(red,green,blue));
    }
    pixel.show(); // This sends the updated pixel color to the hardware.
  }

  delay(500);

}
