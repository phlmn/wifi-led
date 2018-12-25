#pragma once

#include <cstring>

#include <Arduino.h>
#include <ESP8266WiFi.h>

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

class Wifi {
public:
    String ssid;
    String password;

    String (*handler)(String);

    Wifi(String ssid, String password):
        ssid(ssid),
        password(password),
        handler(nullptr) {
    }

    void setup() {
        // Connect to WiFi network
        Serial.println();
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);

        WiFi.begin(this->ssid.c_str(), this->password.c_str());

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }

        Serial.println("");
        Serial.println("WiFi connected");

        // Start the server
        server.begin();
        Serial.println("Server started");

        // Print the IP address
        Serial.println(WiFi.localIP());
    }

    void loop() {
        // ===== WIFI =====

        // Check if a client has connected
        WiFiClient client = server.available();
        if (!client) {
            return;
        }

        // Wait until the client sends some data
        Serial.println("new client");
        unsigned long timeout = millis() + 3000;
        while (!client.available() && millis() < timeout) {
            delay(1);
        }
        if (millis() > timeout) {
            Serial.println("timeout");
            client.flush();
            client.stop();
            return;
        }

        // Read the first line of the request
        String req = client.readStringUntil('\r');
        Serial.println(req);
        client.flush();

        if (this->handler != nullptr) {
            client.print(this->handler(req));
        }

        client.flush();

        delay(1);
        Serial.println("Client disconnected");

        // The client will actually be disconnected
        // when the function returns and 'client' object is detroyed
    }
};
