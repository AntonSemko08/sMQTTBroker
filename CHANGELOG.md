# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] — 2025-XX-XX

### Added

- Full MQTT Last Will and Testament (LWT) support
- `willTopic`, `willMessage`, `willQos`, `willRetain` fields on `sMQTTClient`
- `hasWill` and `disconnectedGracefully` state flags
- Public accessors on `sMQTTClient`:
  - `hasWillMessage()`
  - `wasGracefulDisconnect()`
  - `getWillTopic()`
  - `getWillMessage()`
  - `getWillQos()`
  - `getWillRetain()`
- Internal publishes (LWT, rules, timers) now fire `Public_sMQTTEventType` with `client == nullptr`
- `friend class sMQTTBroker` on `sMQTTClient` for clean access to Will fields

### Changed

- `publish(topic, payload, qos, retain)` now fires `onEvent` before broadcasting, so applications can observe internal publishes

### Fixed

- Will is no longer published on proper `DISCONNECT`
- Will is now actually published on unexpected disconnect (the original library parsed it but never used it)

## [1.0.0] — original

- Imported from upstream `sMQTTBroker`
- MQTT 3.1.1 broker with QoS 0/1, retained messages, subscriptions
