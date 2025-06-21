// sensor_module.h
#pragma once
#include <PubSubClient.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic pop

void init_sensors();
void publish_data(PubSubClient* client, float& lastPressure);
