#include <OctoWS2811.h>
#include <FastLED.h>
// #include <algorithm>

const uint8_t height = 27;
const uint8_t width = 32;

#define NUM_LEDS (height * width)

DMAMEM int displayMemory[NUM_LEDS * 6];
int drawingMemory[NUM_LEDS * 6];

const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(NUM_LEDS, displayMemory, drawingMemory, config);

void flash_all(byte r, byte g, byte b, uint16_t ms);
void test_lights();

int serialData();
void handleCommand();
void printSettings();

void defaultPattern();

void handleFrame();
unsigned int Color(byte r, byte g, byte b);

void golPattern();
void randomFillWorld();
int neighbours(int x, int y);
void displayWorld();
void updateWorld();

byte r = 50, g = 100, b = 200;
uint8_t brightness = 100;
uint8_t saturation = 255;
uint8_t speed = 230;
uint8_t trail_length = 250;
uint8_t hue_speed = 1;
uint8_t hue = 0;
uint8_t density = 5;
unsigned long pixel = 0;
bool starting = 1;
int command;
bool mode = 0;

class Cell {
public:
  bool alive = 0;
  bool prev = 0;
  uint8_t hue = 0;
  uint8_t saturation = 255;
  uint8_t brightness = 0;
};

Cell world[width][height];
// Cell old_world[width][height];

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(230400);
    Serial.printf(
        "\nThis is LEDwall. engaging %d X %d matrix! %d LEDs\n",
        width, height, NUM_LEDS);

    leds.begin();
    test_lights();
}

void loop() {
    if (mode == 1) {
        command = serialData();
        while (command != 1 && command != 2) {
            command = serialData();
        }
        if (command == 2) mode = 0;
    }
    else {
        if (Serial.available()) handleCommand();
    }

    if (mode == 1) {
        handleFrame();
    }
    else {
        golPattern();
        // defaultPattern();
    }
}

void handleCommand() {
    command = serialData();

    // possible commands
    //  1: start of remote frame header, read frame and display
    //  2: switch back to autonomous mode
    //  3: set brightness to following byte
    //  4: set speed to following byte
    //  5: set trail off length with following byte
    //  6: set hue speed with following byte
    //  7: boost density with following byte briefly
    //  8: restart pattern
    //  9: print info
    switch (command) {
        case 1:
            mode = 1;
            Serial.println("REMOTE FRAME MODE");
            break;
        case 3:
            brightness = serialData();
            Serial.printf("brightness: %d\n", brightness);
            break;
        case 4:
            speed = serialData();
            Serial.printf("speed: %d\n", speed);
            break;
        case 5:
            trail_length = serialData();
            Serial.printf("trail_length: %d\n", trail_length);
            break;
        case 6:
            hue = serialData();
            Serial.printf("hue: %d\n", hue);
            break;
        case 7:
            hue_speed = serialData();
            Serial.printf("hue_speed: %d\n", hue_speed);
            break;
        case 8:
            density = serialData();
            Serial.printf("density: %d\n", density);
            break;
        case 9:
            starting = 1;
            Serial.println("restarting pattern");
            break;
        case 10:
            printSettings();
            break;
    }
}

void printSettings() {
    Serial.printf("brightness: %d\n", brightness);
    Serial.printf("speed: %d\n", speed);
    Serial.printf("trail_length: %d\n", trail_length);
    Serial.printf("hue: %d\n", hue);
    Serial.printf("hue_speed: %d\n", hue_speed);
    Serial.printf("density: %d\n", density);
    Serial.println("-------------------");
    Serial.printf("uptime: %dms\n", millis());
}

void defaultPattern() {
    unsigned long last = pixel - 1;
    if (pixel == NUM_LEDS) {
        pixel = 0;
        last = NUM_LEDS - 1;
    }
    leds.setPixel(pixel, Color(r, g, b));
    leds.setPixel(last, Color(0,0,0));
    leds.show();

    pixel++;
    r++;
    g -= 2;
    b += 3;
}

void handleFrame() {
    for (unsigned long i = 0; i < NUM_LEDS; i++) {
        b = serialData();
        r = serialData();
        g = serialData();

        leds.setPixel(i, Color(r,g,b));
   }
   leds.show();
}

void golPattern() {
    if (starting) {
        randomFillWorld();
        starting = 0;
    }

    displayWorld();
    updateWorld();
    delay(255 - speed);
}

void displayWorld() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            CRGB rgb;
            CHSV hsv(world[x][y].hue, world[x][y].saturation, world[x][y].brightness);
            hsv2rgb_rainbow(hsv, rgb);
            if (x % 2) {
                leds.setPixel((x * height + y), rgb.r, rgb.g, rgb.b);
            }
            else {
                leds.setPixel(((x + 1) * height - y - 1), rgb.r, rgb.g, rgb.b);
            }
        }
    }
    leds.show();
}

void updateWorld() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {

            // trail off brightness of dead cells
            if (world[x][y].brightness > 0 && world[x][y].prev == 0) {
                if (world[x][y].brightness - (255 - trail_length) < 0) {
                    world[x][y].brightness = 0;
                }
                else {
                    world[x][y].brightness -= (255 - trail_length);
                }
                // world[x][y].hue = hue; // add trigger?
            }

            int count = neighbours(x, y);

            // A new cell is born
            if (count == 3 && world[x][y].prev == 0) {
                world[x][y].alive = 1;
                world[x][y].hue = hue;
                world[x][y].brightness = brightness;
            }
            // Cell dies
            else if ((count < 2 || count > 3) && world[x][y].prev == 1) {
                world[x][y].alive = 0;
            }
            // just update hue for alive cells
            else if (world[x][y].alive) {
                world[x][y].hue = hue;
            }

            // very low random chance for each cell to be born parentless
            if (random(10000) < (unsigned long)density) {
                world[x][y].alive = 1;
                world[x][y].brightness = brightness;
                world[x][y].hue = hue;
            }
        }
    }
    hue += hue_speed;

    // std::swap(old_world, world);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            world[x][y].prev = world[x][y].alive;
        }
    }
}

void randomFillWorld() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (random(255) < (unsigned long)density) {
                world[x][y].alive = 1;
                world[x][y].brightness = brightness;
            }
            else {
                world[x][y].alive = 0;
                world[x][y].brightness = 0;
            }
            world[x][y].prev = world[x][y].alive;
            world[x][y].hue = 0;
        }
    }
}

int neighbours(int x, int y) {
  return
    (world[(x + 1) % width][y].prev) +
    (world[x][(y + 1) % height].prev) +
    (world[(x + width - 1) % width][y].prev) +
    (world[x][(y + height - 1) % height].prev) +
    (world[(x + 1) % width][(y + 1) % height].prev) +
    (world[(x + width - 1) % width][(y + 1) % height].prev) +
    (world[(x + width - 1) % width][(y + height - 1) % height].prev) +
    (world[(x + 1) % width][(y + height - 1) % height].prev);
}

void test_lights() {
    flash_all(255,0,0,1000);
    flash_all(0,255,0,1000);
    flash_all(0,0,255,1000);
}

void flash_all(byte r, byte g, byte b, uint16_t ms) {
    for (unsigned long i = 0; i < NUM_LEDS; i++) {
        leds.setPixel(i, Color(r,g,b));
    }
    leds.show();
    delay(ms);
    for (unsigned long i = 0; i < NUM_LEDS; i++) {
        leds.setPixel(i, Color(0,0,0));
   }
   leds.show();
}

int serialData() {
    while (!Serial.available()) {}
    return Serial.read();
}

// Create a 24 bit color value from R,G,B
unsigned int Color(byte r, byte g, byte b) {
    //Take the lowest 8 bits of each value and append them end to end
    return (
        (((unsigned int)b & 0xFF )<<16) |
        (((unsigned int)r & 0xFF)<<8) |
        ((unsigned int)g & 0xFF)
    );
}
