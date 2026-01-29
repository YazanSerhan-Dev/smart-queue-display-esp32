# Smart Queue Display System (ESP32 + MQTT)

A lightweight IoT system that displays **real-time queue and reservation statistics** from the Bistro backend on an external display using an ESP32.

The system uses **MQTT (publish/subscribe)** to push updates instantly, avoiding inefficient polling and making the architecture closer to real-world IoT systems.

---

## System Overview

**Bistro Server (Java)**  
→ publishes JSON messages  
→ **MQTT Broker (Mosquitto)**  
→ pushes updates  
→ **ESP32 Subscriber**  

---

## Data Format

**MQTT Topic**
bistro/reservations/today

**JSON Payload**
```json
{
  "today": 12,
  "served": 7
}

## Quick Setup

1. Configure Wi-Fi + MQTT broker IP in the code.
2. Upload to ESP32 using PlatformIO.
3. Make sure Mosquitto is running (or any MQTT broker).
4. Publish a test message:

```bash
mosquitto_pub -h <BROKER_IP> -t bistro/reservations/today -m '{"today":5,"served":2}'


### 3) Where to change config (super important)
Add:

```md
## Configuration

Update these values in the ESP32 code:
- `WIFI_SSID`, `WIFI_PASS`
- `MQTT_BROKER_IP`
- `MQTT_TOPIC`
