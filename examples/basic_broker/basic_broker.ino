/*
 * basic_broker.ino
 *
 * Minimal example — starts an MQTT broker on port 1883.
 * Prints every incoming publish to Serial.
 */

#include <WiFi.h>
#include <sMQTTBroker.h>

const char* WIFI_SSID     = "your-ssid";
const char* WIFI_PASSWORD = "your-password";

class MyBroker : public sMQTTBroker {
public:
    bool onEvent(sMQTTEvent* event) override {
        if (!event) return true;

        switch (event->Type()) {
            case NewClient_sMQTTEventType: {
                auto* e = (sMQTTNewClientEvent*)event;
                if (e->Client()) {
                    Serial.printf("CONNECT:    %s\n",
                                  e->Client()->getClientId().c_str());
                }
                break;
            }

            case RemoveClient_sMQTTEventType: {
                auto* e = (sMQTTRemoveClientEvent*)event;
                if (e->Client()) {
                    Serial.printf("DISCONNECT: %s\n",
                                  e->Client()->getClientId().c_str());
                }
                break;
            }

            case Public_sMQTTEventType: {
                auto* e = (sMQTTPublicClientEvent*)event;
                if (e->Client() == nullptr) {
                    Serial.printf("INTERNAL:   %s = %s\n",
                                  e->Topic().c_str(),
                                  e->Payload().c_str());
                } else {
                    Serial.printf("PUBLISH:    [%s] %s = %s\n",
                                  e->Client()->getClientId().c_str(),
                                  e->Topic().c_str(),
                                  e->Payload().c_str());
                }
                break;
            }

            case Subscribe_sMQTTEventType:
            case UnSubscribe_sMQTTEventType: {
                auto* e = (sMQTTSubUnSubClientEvent*)event;
                Serial.printf("SUB:        %s -> %s\n",
                              e->Client() ? e->Client()->getClientId().c_str() : "?",
                              e->Topic().c_str());
                break;
            }

            case LostConnect_sMQTTEventType:
                Serial.println("Wi-Fi lost");
                break;
        }

        return true;
    }
};

MyBroker broker;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=== sMQTTBroker basic example ===");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Starting MQTT broker on port 1883...");
    broker.init(1883, true);
    Serial.println("Broker started");
}

void loop() {
    broker.update();
    delay(1);
}
