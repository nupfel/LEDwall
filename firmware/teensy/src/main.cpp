#include <OctoWS2811.h>

const int ledsPerStrip = 864;
const int NUM_LEDS = 864;

DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];

const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

unsigned int Color(byte r, byte g, byte b);
byte r,g,b;
int i;

void flash_all(byte r, byte g, byte b, long ms) {
    for (i=0; i < NUM_LEDS; i++) {
        leds.setPixel(i, Color(r,g,b));
    }
    leds.show();
    delay(ms);
    for (i=0; i < NUM_LEDS; i++) {
        leds.setPixel(i, Color(0,0,0));
   }
   leds.show();
}

void test_lights() {
    flash_all(255,0,0,1000);
    flash_all(0,255,0,1000);
    flash_all(0,0,255,1000);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(230400);
    Serial.println("This is LEDwall. engaging matrix!");

    leds.begin();
    test_lights();
}

int serialGlediator() {
    while (!Serial.available()) {}
    return Serial.read();
}

void loop() {
    while (serialGlediator() != 1) {}

    for (i=0; i < NUM_LEDS; i++) {
        b = serialGlediator();
        r = serialGlediator();
        g = serialGlediator();

        leds.setPixel(i, Color(r,g,b));
   }
   leds.show();
}

/* Helper functions */
// Create a 24 bit color value from R,G,B
unsigned int Color(byte r, byte g, byte b) {
    //Take the lowest 8 bits of each value and append them end to end
    return( ((unsigned int)b & 0xFF )<<16 | ((unsigned int)r & 0xFF)<<8 | (unsigned int)g & 0xFF);
}
