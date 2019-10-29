
#include <Adafruit_NeoPixel.h>
#include <SPI.h>

char buf [100];
volatile byte pos;
volatile boolean process_it;
bool ledWait;
bool ShotLedsBlink;
int semicolumnIndex;
String command;
String commandValue;
String Stringbuffer;
int RainbowPos;
int BlinkCounter;


#define LedDataPin        8
#define NumPixels         112
#define SSPin             4
// 0-107 for the flashlight (6x 18 leds), 108-112 for shothok

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NumPixels, LedDataPin, NEO_GRB + NEO_KHZ800);

void setup() {
  // init leds
  pixels.begin();         // This initializes the NeoPixel library.
  AllLedsOff();           // Ensure all leds are off
  pixels.show();
  RainbowPos = 0;
  BlinkCounter = 0;
  ShotLedsBlink = false;
  ledWait = false;        // waitstate for leds when machine is on, but no picture is taken
  
  // init serial interface for debugging
  Serial.begin (9600);    // debugging

  // SPI initialize
  pinMode(MISO, OUTPUT);
  pinMode(MOSI, INPUT);
  pinMode(SSPin, INPUT);
  SPCR |= _BV(SPE);       // SPI in slave mode
  pos = 0;                // buffer empty
  process_it = false;     // reset indicator
  SPI.attachInterrupt();  // now turn on interrupts
  SPI.setClockDivider(SPI_CLOCK_DIV4); // set max clockspeed for SPI

  // Wrap up setup
  Serial.println ("Setup done, starting program");
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

// Flashlight on
void FlashlightOn(){  
  for (int i = 0; i < 108; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
}

// Flashlight off
void FlashlightOff(){  
  for (int i = 0; i < 108; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
}

// All leds completely off
void AllLedsOff(){  
  for (int i = 0; i < 113; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
}

// Shotleds off
void ShotLedsOff(){  
  for (int i = 108; i < 113; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
}

// Shotleds Red
void ShotLedsRed(){  
  for (int i = 108; i < 113; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
  }
}

// Shotleds Green
void ShotLedsGreen(){  
  for (int i = 108; i < 113; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


// Main loop
void loop() {
  // Check Slave Select pin and if high, watch SPI interface
  bool val = digitalRead(SSPin);
  while (val == true){
    val = digitalRead(SSPin);
    delay(1);
  }

  // If message recieved, process it
  if (process_it){
    Stringbuffer = String(buf);
    Serial.print("recieved string: ");
    Serial.println (Stringbuffer);
    semicolumnIndex = Stringbuffer.indexOf(';');
    command = Stringbuffer.substring(0, semicolumnIndex);
    commandValue = Stringbuffer.substring(semicolumnIndex+1);
    if (command == "shot"){
      if (commandValue == "0"){
        ShotLedsOff();
        ShotLedsBlink = false;
        Serial.println ("Shot leds off");
      }
      else if (commandValue == "1"){
        ShotLedsRed();
        ShotLedsBlink = false;
        Serial.println ("Shot leds red");
      }
      else if (commandValue == "2"){
        ShotLedsGreen();
        ShotLedsBlink = false;
        Serial.println ("Shot leds green");
      }
      else if (commandValue == "3"){
        ShotLedsBlink = true;
        BlinkCounter = 0;
        Serial.println ("Shot leds blinking red");
      }
    }
    if (command == "state"){
      if (commandValue == "0"){
        ledWait = false;
        FlashlightOff();
        Serial.println ("Flashlight off");
      }
      else if (commandValue == "1"){
        ledWait = true;
        Serial.println ("Flashlight waitstate");
      }
      else if (commandValue == "2"){
        ledWait = false;
        FlashlightOn();
        Serial.println ("Flashlight on");
      }
    }
    for( int i = 0; i < sizeof(buf);  ++i )
      buf[i] = (char)0;
    pos = 0; 
    process_it = false;
  }

  // If flashlight in waitstate, create rainbow colors
  if (ledWait == true){
    RainbowPos++;
    for (int i = 0; i < 18; i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / 18*100) + RainbowPos) & 255));
      pixels.setPixelColor(35-i, Wheel(((i * 256 / 18*100) + RainbowPos) & 255));
      pixels.setPixelColor(i+36, Wheel(((i * 256 / 18*100) + RainbowPos) & 255));
      pixels.setPixelColor(i+54, Wheel(((i * 256 / 18*100) + RainbowPos) & 255));
      pixels.setPixelColor(89-i, Wheel(((i * 256 / 18*100) + RainbowPos) & 255));
      pixels.setPixelColor(i+90, Wheel(((i * 256 / 18*100) + RainbowPos) & 255));
    }
  }

  if (ShotLedsBlink == true){
    BlinkCounter = BlinkCounter % 42;
    //Serial.println (BlinkCounter);
    if (BlinkCounter == 0){
      ShotLedsRed();
      Serial.println ("Shot leds Blink On red");
    }
    else if (BlinkCounter == 21){
      ShotLedsOff();
      Serial.println ("Shot leds Blink Off");
    }
    BlinkCounter++;
  }

  // delay for reducing loop speed
  delay(20);        
  // write data to leds, this is a time-critical process so temporary disable interupts
  noInterrupts();
  pixels.show();
  interrupts();
  
}
