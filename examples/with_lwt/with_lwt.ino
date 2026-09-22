/*
 * with_lwt.ino
 *
 * Demonstrates Last Will and Testament (LWT) support.
 *
 * Subscribe to `devices/+/status` from any MQTT client.
 * When a device disconnects unexpectedly, the broker publishes
 * its Will message ("offline") automatically.
 */

#include <WiFi.h>
#include <sMQTTBroker.h>

const char* WIFI_SSID     = "your-ssid";
const char* WIFI_PASSWORD = "your-password";

class MyBroker : public sMQTTBroker {
public:
    bool onEvent(sMQTTEvent* event) override {
        if (!event) return true;

        if (event->Type() == NewClient_sMQTTEventType) {
            auto* e = (sMQTTNewClientEvent*)event;
            if (e->Client()) {
                Serial.printf("CONNECT:    %s\n",
                              e->Client()->getClientId().c_str());
            }
            return true;
        }

        if (event->Type() == RemoveClient_sMQTTEventType) {
            auto* e = (sMQTTRemoveClientEvent*)event;
            sMQTTClient* c = e->Client();
            if (c) {
                Serial.printf("DISCONNECT: %s\n",
                              c->getClientId().c_str());

                // Was this an unexpected disconnect? If so, the broker
                // has already published the Will before reaching here.
                if (c->hasWillMessage() && !c->wasGracefulDisconnect()) {
                    Serial.printf("LWT:        %s = %s\n",
                                  c->getWillTopic().c_str(),
                                  c->getWillMessage().c_str());
                } else if (c->hasWillMessage()) {
                    Serial.println("LWT:        skipped (graceful DISCONNECT)");
                }
            }
            return true;
        }

        if (event->Type() == Public_sMQTTEventType) {
            auto* e = (sMQTTPublicClientEvent*)event;
            const char* src = (e->Client() == nullptr) ? "INTERNAL" : "CLIENT";
            Serial.printf("PUBLISH:    [%s] %s = %s\n",
                          src,
                          e->Topic().c_str(),
                          e->Payload().c_str());
            return true;
        }

        return true;
    }
};

MyBroker broker;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("=== sMQTTBroker LWT example ===");
    Serial.println("Subscribe to `devices/+/status` from an MQTT client");

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

    broker.init(1883, true);
    Serial.println("Broker started. Waiting for clients...");
}

void loop() {
    broker.update();
    delay(1);
}
