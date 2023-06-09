#include <Adafruit_NeoPixel.h>

#define PIN 10

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(7, PIN, NEO_RGBW);
int indexByte = 0;
int redByte = 0;
int greenByte = 0;
int blueByte = 0;
int whiteByte = 0;
int timebyte = 0;

boolean isSerialMonitor = false;

class OffEvent {
  public:
    uint32_t atTime;
    int index;
    OffEvent() {}
};
OffEvent offEvents[16];
int offEventLength = 0;


void setup() {
  SerialUSB.begin(9600);
  pixels.begin();
  while(!SerialUSB);
}

void loop() {
  //Turn LEDs off after specified time
  if (offEventLength > 0 && offEvents[offEventLength-1].atTime < millis()) {
    setRGB(offEvents[offEventLength-1].index, 0, 0, 0, 0);
    offEventLength--;
  }

  //Read new data
  if (Serial.available() >= 5) {
    if (!readValues()) {
      return;
    }
    execute();
  }
  delay(50);
}

boolean readValues() {
    indexByte = monitorAdjust(Serial.read());
    redByte = monitorAdjust(Serial.read());
    greenByte = monitorAdjust(Serial.read());
    blueByte = monitorAdjust(Serial.read());
    whiteByte = monitorAdjust(Serial.read());
    timebyte = monitorAdjust(Serial.read());
    return indexByte != -1 && redByte != -1 && greenByte != -1 
      && blueByte != -1 && whiteByte != -1 && timebyte != -1;
}

int monitorAdjust(int val) {
  if (!isSerialMonitor) {
    return val;
  }
  if (val >= 48 && val <= 57) { //ascii 0-9 -> int 0-9
    return val - 48;
  }
  return val * 2; //cause ascii 0-127 and i want full byte range
}


void execute() {
  setRGB(indexByte, redByte, greenByte, blueByte, whiteByte);
  if (timebyte > 0) {
    addOffEvent(millis() + (timebyte * 100), indexByte);
  }
}

void setRGB(int index, int red, int green, int blue, int white) {
  unsigned int mask;
  int maskPos=6;
  for (mask = 0b01000000; mask != 0; mask >>= 1) {
    if (index & mask) {
      pixels.setPixelColor(maskPos, pixels.Color(green, red, blue, white));
    }
    maskPos--;
  }
  pixels.show();
}

void addOffEvent(uint32_t atTime, int index) {
  if(offEventLength == 16) {
    return;
  }
  int currentIndex = offEventLength;
  while(currentIndex > 0 && offEvents[currentIndex-1].atTime < atTime) {
    offEvents[currentIndex].atTime = offEvents[currentIndex-1].atTime;
    offEvents[currentIndex].index = ~index & offEvents[currentIndex-1].index; //remove leds that got overridden by a later event
    currentIndex--;
  }
  offEventLength++;
  offEvents[currentIndex].atTime = atTime;
  offEvents[currentIndex].index = index;
  
}
