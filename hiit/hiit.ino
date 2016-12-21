#include<SimbleeForMobile.h>


const uint32_t LEDPIN = 3;
#define    RELAY_PIN    3

int redVal;
int greenVal;
int blueVal;
int rounds;

int leds = 0;

uint8_t hiitID;
uint8_t partyID;


void setup() {
  // put your setup code here, to run once:
          pinMode(LEDPIN, OUTPUT);
          rounds = 0;
          RGB_Show(0x00,0x00,0x00, LEDPIN);

          Serial.begin(9600);
          
          SimbleeForMobile.advertisementData = "HIIT";
          SimbleeForMobile.deviceName = "WRL-13632";
          SimbleeForMobile.txPowerLevel = -4;
          SimbleeForMobile.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  SimbleeForMobile.process();
}

void ui(){
  color_t darkgray = rgb(85, 85, 85);

  uint16_t wid = SimbleeForMobile.screenWidth;
  uint16_t hgt = SimbleeForMobile.screenHeight;

  SimbleeForMobile.beginScreen(darkgray);

 
  hiitID = SimbleeForMobile.drawButton(
                              (wid/2) - 75,          // x location
                              (hgt/2) - 22,          // y location
                              150,                   // width of button
                              "HIIT",         // text shown on button
                              WHITE,                 // color of button
                              BOX_TYPE);             // type of button  
  partyID = SimbleeForMobile.drawButton(
                              (wid/2) - 75,          // x location
                              (hgt/2) - 66,          // y location
                              150,                   // width of button
                              "PARTY",         // text shown on button
                              WHITE,                 // color of button
                              BOX_TYPE);             // type of button                         
                                 
  SimbleeForMobile.endScreen();
}


void ui_event_helper(event_t &event) {
  if (event.id == hiitID) {

    if (event.value == 1) {

        RGB_Show(0x00,0xFF,0x00, LEDPIN);
        delay(1000);
      }else if(event.value == 0) {
        RGB_Show(0x00,0x00,0x00, LEDPIN);     
    }
    
  }
  
}

void ui_event(event_t &event) {
  if (event.id == hiitID) {
    hiitbeg();
  }
  if(event.id == partyID){
    partymode();
  }
}

void hiitbeg(){
  rounds = 0;
  //two rounds for demonstration purposes
    while(rounds < 2){
    RGB_Show(0x00,0xFF,0x00, LEDPIN);
    
    delay(30000);
    
    RGB_Show(0xFF,0xFF,0x00, LEDPIN);
    
    rounds = rounds + 1;
    delay(30000);
    }
    if(rounds == 2){
    RGB_Show(0xFF,0x00,0x00, LEDPIN);
    delay(30000);
    RGB_Show(0x00,0x00,0x00, LEDPIN);
  }
}
void partymode(){
  
    RGB_Show(0x00,0x00,0xFF, LEDPIN);
    delay(1000);
    RGB_Show(0xFF,0xFF,0x00, LEDPIN);
    delay(1000);
    RGB_Show(0x00,0xFF,0x00, LEDPIN);
    delay(1000);
    RGB_Show(0x00,0xFF,0xFF, LEDPIN);
    delay(1000);
    RGB_Show(0xC0,0x00,0xFF, LEDPIN);
    delay(1000);
    RGB_Show(0xFF,0xAF,0xAF, LEDPIN);
    delay(1000);
    RGB_Show(0x00,0x00,0x00, LEDPIN);
}
void RGB_Show(uint8_t r, uint8_t g, uint8_t b, uint32_t ledPin)
{
    uint8_t rgb[3] = {g, r, b};
    uint8_t *p = rgb;
    uint8_t *end = p + 3;

    while (p < end)
    {
        uint8_t pix = *p++;
        for (uint8_t mask = 0x80; mask; mask >>= 1)
        {
            if (pix & mask)
            {
                // T1H 760ns
                NRF_GPIO->OUTSET = (1UL << ledPin);
                NRF_GPIO->OUTSET = (1UL << ledPin);
                NRF_GPIO->OUTSET = (1UL << ledPin);

                // T1L 660ns
                NRF_GPIO->OUTCLR = (1UL << ledPin);
            }
            else
            { 
                // T0H 380ns
                NRF_GPIO->OUTSET = (1UL << ledPin);

                // T0L 840ns
                NRF_GPIO->OUTCLR = (1UL << ledPin);
                NRF_GPIO->OUTCLR = (1UL << ledPin);
                NRF_GPIO->OUTCLR = (1UL << ledPin);
                NRF_GPIO->OUTCLR = (1UL << ledPin);
            }
        }
    } 
    NRF_GPIO->OUTCLR = (1UL << ledPin);
}
