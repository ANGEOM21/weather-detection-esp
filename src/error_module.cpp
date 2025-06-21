#include "error_module.h"

static PubSubClient *client = nullptr;
static bool hasError = false;

void init_error_module(PubSubClient *mqttClient) {
  client = mqttClient;
}

void report_error(const String &source, const String &message) {
  hasError = true;

  String payload = "{\"type\":\"error\",\"source\":\"" + source + "\",\"message\":\"" + message + "\"}";
  String topic = "device/errors/" + source;

  Serial.println("🚨 " + payload);

  if (client && client->connected()) {
    client->publish(topic.c_str(), payload.c_str());
  }
}

void report_recovery(const String &source) {
  if (!hasError) return;

  hasError = false;

  String payload = "{\"type\":\"recovery\",\"source\":\"" + source + "\"}";
  String topic = "device/recovery/" + source;

  Serial.println("✅ " + payload);

  if (client && client->connected()) {
    client->publish(topic.c_str(), payload.c_str());
  }
}

void heartbeat_status() {
  if (client && client->connected()) {
    client->publish("device/status", "{\"status\":\"ok\"}");
  }
}
