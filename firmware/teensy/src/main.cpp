#include <OctoWS2811.h>

const int ledsPerStrip = 800;
const int NUM_LEDS = 800;

DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];

const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

unsigned int Color(byte r, byte g, byte b);

void setup() {
    Serial.begin(230400);
    Serial.println("This is LEDwall. engaging matrix!");

    leds.begin();
    leds.show();
}

int serialGlediator() {
    while (!Serial.available()) {}
    return Serial.read();
}

byte r,g,b;
int i;

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
