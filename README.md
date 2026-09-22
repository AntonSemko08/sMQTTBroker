# sMQTTBroker

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-supported-blue.svg)](https://platformio.org/)
[![Arduino](https://img.shields.io/badge/Arduino-supported-green.svg)](https://www.arduino.cc/)

An MQTT 3.1.1 broker library for **ESP32** and **ESP8266** with full **Last Will and Testament (LWT)** support.

This is a fork of the original [sMQTTBroker](https://github.com/) library. The main difference: **complete LWT implementation** and proper delivery of internal publish events to the application layer.

---

## Why this fork?

The original library parses the LWT fields from the CONNECT packet but never stores or uses them — so a client's Last Will is never published. This fork completes the implementation:

- ✅ Stores `willTopic`, `willMessage`, `willQos`, `willRetain` on CONNECT
- ✅ Publishes the Will when a client disconnects **unexpectedly**
- ✅ Does **not** publish the Will when the client sends a proper `DISCONNECT`
- ✅ Fires `Public_sMQTTEventType` for internal publishes (LWT, rules, timers), so applications can react to them
- ✅ Exposes public accessors on `sMQTTClient` for the Will fields

---

## Features

- MQTT 3.1.1 compatible
- QoS 0 and QoS 1
- Retained messages
- Subscriptions with `+` and `#` wildcards
- Optional username/password authentication (via `NewClient` event)
- **Full Last Will and Testament (LWT) support**
- Internal publish event for LWT / rules / timers
- Works on ESP32, ESP8266, and WIO Terminal

---

## Installation

### Arduino IDE

1. Download this repository as ZIP.
2. **Sketch → Include Library → Add .ZIP Library…**
3. Or copy the `src/` folder into `~/Documents/Arduino/libraries/sMQTTBroker/`.

### PlatformIO

Add to `platformio.ini`:

```ini
lib_deps =
    https://github.com/AntonSemko08/sMQTTBroker.git
```

---

## Quick start

```cpp
#include <WiFi.h>
#include <sMQTTBroker.h>

class MyBroker : public sMQTTBroker {
public:
    bool onEvent(sMQTTEvent* event) override {
        if (event->Type() == Public_sMQTTEventType) {
            auto* e = (sMQTTPublicClientEvent*)event;
            Serial.printf("%s = %s\n",
                          e->Topic().c_str(),
                          e->Payload().c_str());
        }
        return true;
    }
};

MyBroker broker;

void setup() {
    Serial.begin(115200);

    WiFi.begin("ssid", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
    }

    broker.init(1883, true);  // port, checkWiFi
}

void loop() {
    broker.update();
}
```

---

## LWT support

### How it works

1. Client sends `CONNECT` with the Will flag set, including Will topic, message, QoS and retain flag.
2. The broker stores these fields in `sMQTTClient`.
3. When the client disconnects **without** sending `DISCONNECT` (power loss, Wi-Fi drop, crash), the broker publishes the Will.
4. When the client sends a proper `DISCONNECT`, the Will is **not** published.

### Client-side example (ESPHome)

```yaml
mqtt:
  broker: 192.168.1.7
  port: 1883
  topic_prefix: my-device

  birth_message:
    topic: devices/my-device/status
    payload: online
    qos: 0
    retain: true

  will_message:
    topic: devices/my-device/status
    payload: offline
    qos: 0
    retain: true
```

Now any subscriber to `devices/+/status` will immediately see the current state of every device.

### Client-side example (PubSubClient)

```cpp
client.connect(
    "my-device",                    // client id
    nullptr, nullptr,               // user, password
    "devices/my-device/status",     // will topic
    0,                              // will qos
    true,                           // will retain
    "offline"                       // will message
);

// Announce that we're alive
client.publish("devices/my-device/status", "online", true);
```

---

## Public API additions

### `sMQTTClient`

```cpp
bool hasWillMessage() const;
bool wasGracefulDisconnect() const;
const std::string& getWillTopic() const;
const std::string& getWillMessage() const;
unsigned char getWillQos() const;
bool getWillRetain() const;
```

### `sMQTTBroker`

The public `publish(topic, payload, qos, retain)` now fires `Public_sMQTTEventType` with `client == nullptr`, so application code can distinguish internal publishes from client publishes:

```cpp
if (event->Type() == Public_sMQTTEventType) {
    auto* e = (sMQTTPublicClientEvent*)event;
    if (e->Client() == nullptr) {
        Serial.println("Internal publish (LWT, rule, or timer)");
    } else {
        Serial.printf("Client %s published\n",
                      e->Client()->getClientId().c_str());
    }
}
```

---

## Examples

See the [`examples/`](examples/) folder:

- [`basic_broker/`](examples/basic_broker) — minimal broker
- [`with_lwt/`](examples/with_lwt) — LWT + device tracking

---

## Compatibility

| Platform       | Status        |
|----------------|---------------|
| ESP32          | ✅ Tested     |
| ESP8266        | ✅ Tested     |
| WIO Terminal   | ⚠️ Untested   |

---

## Documentation

### API reference

**`sMQTTBroker`**

| Method | Description |
|---|---|
| `bool init(unsigned short port, bool checkWiFiConnection = false)` | Start the broker |
| `void update()` | Process incoming traffic — call in `loop()` |
| `void publish(const std::string& topic, const std::string& payload, unsigned char qos = 0, bool retain = false)` | Publish a message |
| `void restart()` | Restart the TCP server |
| `virtual bool onEvent(sMQTTEvent* event) = 0` | Event callback — implement in subclass |
| `unsigned long getRetainedTopicCount()` | Number of retained topics |
| `std::string getRetaiedTopicName(unsigned long index)` | Retained topic name by index |

**Events**

| Event | Description |
|---|---|
| `NewClient_sMQTTEventType` | Client connected — return `false` to reject (auth) |
| `RemoveClient_sMQTTEventType` | Client disconnected |
| `Public_sMQTTEventType` | Message published (client or internal) |
| `LostConnect_sMQTTEventType` | Wi-Fi lost |
| `Subscribe_sMQTTEventType` | Client subscribed |
| `UnSubscribe_sMQTTEventType` | Client unsubscribed |

---

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

---

## License

MIT — see [LICENSE](LICENSE).

---

## Credits

- Original `sMQTTBroker` library — the initial implementation.
- LWT support and internal-publish events — Anton Semko ([@AntonSemko08](https://github.com/AntonSemko08)).
