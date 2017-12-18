#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <ESP8266WiFi.h>

int poti = A0;
int button = D6;

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(1);
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(2);

const char* ssid     = "LED3";
const char* password = "BardiaLED3";
IPAddress host(192,168,100,1);
uint8_t reconnectCount = 0;
WiFiClient client;

// void connectToServer();
void displaySensorDetails(void);

void setup() {
        // Serial interface
        Serial.begin(230400);
        delay(1000);

        // declare push button
        pinMode(button, INPUT_PULLUP);

        // Enable auto-gain
        mag.enableAutoRange(true);

        // Initialise the magnetometer sensor
        if(!mag.begin()) {
                Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
                while(1) ;
        }

        /* Initialise the accelerometer sensor */
        if(!accel.begin()) {
                /* There was a problem detecting the ADXL345 ... check your connections */
                Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
                while(1) ;
        }

        // Display some basic information on this sensor
        displaySensorDetails();

        // connect to a WiFi network
        // Serial.println();
        // Serial.print("Connecting to ");
        // Serial.println(ssid);
        //
        // WiFi.begin(ssid, password);
        //
        // while (WiFi.status() != WL_CONNECTED) {
        //         delay(500);
        //         if (reconnectCount >= 60) {
        //                 ESP.restart();
        //         }
        //         Serial.print(".");
        //         reconnectCount++;
        // }
        // reconnectCount = 0;
        //
        // Serial.println("");
        // Serial.println("WiFi connected");
        // Serial.print("IP address: ");
        // Serial.println(WiFi.localIP());

        // connectToServer();
}

void displaySensorDetails(void) {
        sensor_t sensor;
        mag.getSensor(&sensor);
        Serial.println("------------------------------------");
        Serial.print  ("Sensor:       "); Serial.println(sensor.name);
        Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
        Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
        Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
        Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
        Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");
        Serial.println("------------------------------------");
        Serial.println("");
        delay(500);

        accel.getSensor(&sensor);
        Serial.println("------------------------------------");
        Serial.print  ("Sensor:       "); Serial.println(sensor.name);
        Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
        Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
        Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
        Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
        Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
        Serial.println("------------------------------------");
        Serial.println("");
        delay(500);
}

void loop() {
        /* Get a new sensor event */
        sensors_event_t accel_event;
        sensors_event_t mag_event;
        mag.getEvent(&mag_event);
        accel.getEvent(&accel_event);

        /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
        Serial.print("X: "); Serial.print(mag_event.magnetic.x); Serial.print("  ");
        Serial.print("Y: "); Serial.print(mag_event.magnetic.y); Serial.print("  ");
        Serial.print("Z: "); Serial.print(mag_event.magnetic.z); Serial.print("  ");
        Serial.print("uT  ");

        /* Note: You can also get the raw (non unified values) for */
        /* the last data sample as follows. The .getEvent call populates */
        /* the raw values used below. */
        Serial.print("X Raw: "); Serial.print(mag.raw.x); Serial.print("  ");
        Serial.print("Y Raw: "); Serial.print(mag.raw.y); Serial.print("  ");
        Serial.print("Z Raw: "); Serial.print(mag.raw.z); Serial.println("");

        /* Display the results (acceleration is measured in m/s^2) */
        Serial.print("X: "); Serial.print(accel_event.acceleration.x); Serial.print("  ");
        Serial.print("Y: "); Serial.print(accel_event.acceleration.y); Serial.print("  ");
        Serial.print("Z: "); Serial.print(accel_event.acceleration.z); Serial.print("  ");
        Serial.print("m/s^2 ");

        /* Note: You can also get the raw (non unified values) for */
        /* the last data sample as follows. The .getEvent call populates */
        /* the raw values used below. */
        Serial.print("X Raw: "); Serial.print(accel.raw.x); Serial.print("  ");
        Serial.print("Y Raw: "); Serial.print(accel.raw.y); Serial.print("  ");
        Serial.print("Z Raw: "); Serial.print(accel.raw.z); Serial.println("");

        Serial.println(analogRead(poti));

        if (digitalRead(button) == LOW) Serial.println("button pressed");

        delay(100);
}

// void connectToServer() {
//     Serial.println("connecting to " + host.toString() + ":" + String(port));
//
//     if (!client.connect(host, port)) {
//         Serial.println("connection failed");
//         return;
//     }
//     else {
//         Serial.println("connected");
//     }
// }
