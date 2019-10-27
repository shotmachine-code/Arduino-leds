
#include <Adafruit_NeoPixel.h>
#include <SPI.h>

char buf [100];
volatile byte pos;
volatile boolean process_it;
bool ledWait;
int semicolumnIndex;
String command;
String commandValue;
String Stringbuffer;

#define LedDataPin        8
#define NumPixels         112
// 108 for the flashlight, 109 till 113 for shothok

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NumPixels, LedDataPin, NEO_GRB + NEO_KHZ800);
int delayval = 500;

void setup() {

  pixels.begin();         // This initializes the NeoPixel library.
  ledsoff();              // Ensure all leds are off

  Serial.begin (9600);    // debugging

  // SPI initialize
  pinMode(MISO, OUTPUT);
  pinMode(MOSI, INPUT);
  SPCR |= _BV(SPE);       // SPI in slave mode
  pos = 0;                // buffer empty
  process_it = false;     // reset indicator
  ledWait = false;        // waitstate for leds when machine is on, but no picture is taken 
  SPI.attachInterrupt();  // now turn on interrupts
  SPI.setClockDivider(SPI_CLOCK_DIV4); // set max clockspeed for SPI
  
}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
byte c = SPDR;  // grab byte from SPI Data Register
  if (pos < (sizeof (buf) -1))
    {
      if (c == '\n'){
        process_it = true;  
      }
      else{
        buf [pos++] = c;
      }
    }
}

void ledson(){  // Flashlight on
  for (int i = 0; i < 108; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
   pixels.show();
}

void ledsoff(){  // All leds completely off
  for (int i = 0; i < 113; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
}

void loop() {
  if (process_it){
    Serial.print("recieved string: ");
    Serial.println (buf);
    Stringbuffer = String(buf);
    semicolumnIndex = Stringbuffer.indexOf(';');
    command = Stringbuffer.substring(0, semicolumnIndex);
    commandValue = Stringbuffer.substring(semicolumnIndex+1);
    if (command == "state"){
      if (commandValue == "0"){
        ledWait = false;
        ledsoff();
        Serial.println ("leds complete off");
      }
      else if (commandValue == "1"){
        ledWait = true;
        Serial.println ("leds waitstate");
      }
      else if (commandValue == "2"){
        ledWait = false;
        ledson();
        Serial.println ("leds complete on");
      }
    }
    buf[0] = (char)0;
    pos = 0; 
    process_it = false;
  }
  
    
}
