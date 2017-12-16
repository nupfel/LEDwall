#include <ESP8266WiFi.h>
ADC_MODE(ADC_VCC);

const char* ssid     = "LEDwall";
const char* password = "pewpewpew";

IPAddress host(192,168,4,1);
const int port = 23;
unsigned char reconnectCount = 0;

WiFiClient client;

void connectToServer();

void setup() {
    Serial.begin(115200);
    delay(2000);
    float batVolt=ESP.getVcc() / 1000.0;
    Serial.println();
    Serial.println();
    Serial.print("Battery: ");
    Serial.print(batVolt);
    Serial.println("V");

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (reconnectCount >= 60) {
            ESP.restart();
        }
        Serial.print(".");
        reconnectCount++;
    }
    reconnectCount = 0;

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    connectToServer();
}
byte b;
void loop() {
    if (Serial.available()) {
        // Serial.println("1");
        // size_t len = Serial.available();
        // Serial.println("2");
        // uint8_t * sbuf = (uint8_t *)malloc(len);
        // Serial.println("3");
        b = Serial.read();
        // Serial.println("4");
        // Serial.write(sbuf, len);
        // Serial.println("5");
        if (client.connected()) {
            Serial.println("6");
            // client.write((uint8_t *)sbuf, len);
            client.write(b);
            Serial.println("7");
            // yield();
        }
        else {
            delay(500);
            if (reconnectCount >= 60) {
                ESP.restart();
            }
            Serial.println("Connection lost!");
            Serial.println("Try to reconnect...");
            connectToServer();
            reconnectCount++;
        }
        Serial.println("8");
        // free(sbuf);
        // Serial.println("9");
    }

    // if (client.available()) {
    //     Serial.println("10");
    //     char c = client.read();
    //     Serial.println("11");
    //     Serial.print(c);
    //     Serial.println("12");
    // }
}

void connectToServer() {
    Serial.println("connecting to " + host.toString() + ":" + String(port));

    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }
    else {
        Serial.println("connected");
    }
}
