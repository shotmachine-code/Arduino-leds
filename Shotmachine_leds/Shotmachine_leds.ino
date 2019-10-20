// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <SPI.h>

byte buf;
volatile byte pos;
volatile boolean process_it;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            8

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      112

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.

 for (int i = 108; i < 113; i++) {

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    //pixels.setPixelColor(i, pixels.Color(255, 255, 255)); // Moderately bright green color.
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Moderately bright green color.


  }
   pixels.show();
   Serial.begin (9600);   // debugging

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  pinMode(MOSI, INPUT);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  
  // get ready for an interrupt 
  pos = 0;   // buffer empty
  process_it = false;

  // now turn on interrupts
  SPI.attachInterrupt();
  
}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
byte c = SPDR;  // grab byte from SPI Data Register
  
  // add to buffer if room
  if (pos < sizeof buf)
    {
    buf = c;
    //Serial.println(c);
    
    // example: newline means time to process buffer
    if (c == 72 || c == 73){
      process_it = true;  
    }  // end of room available
    }
}  // end of interrupt routine SPI_STC_vect

void ledson(){
  for (int i = 0; i < 108; i++) {

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(255, 255, 255)); // Moderately bright green color.

   
    //delay(delayval); // Delay for a period of time (in milliseconds).

  }
   pixels.show(); // This sends the updated pixel color to the hardware.
}

void ledsoff(){
  for (int i = 0; i < 108; i++) {

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Moderately bright green color.

   
    //delay(delayval); // Delay for a period of time (in milliseconds).

  }
   pixels.show(); // This sends the updated pixel color to the hardware.
}

void loop() {
  //ledson();
  //delay(10000);
  //ledsoff();
  //delay(10000);
  
  if (process_it){
    Serial.print("recieved string: ");
    Serial.println (buf);
    if (buf == 72){
        //ledson();
        ledsoff();
    }
    if (buf == 73){
        ledsoff();
    }
    process_it = false;
    }  // end of flag set
}
