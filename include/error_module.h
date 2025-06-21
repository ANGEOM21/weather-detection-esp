#pragma once

#include <Arduino.h>
#include <PubSubClient.h>

void init_error_module(PubSubClient *mqttClient);
void report_error(const String &source, const String &message);
void report_recovery(const String &source);
void heartbeat_status();
