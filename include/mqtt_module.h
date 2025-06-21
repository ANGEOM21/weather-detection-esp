#pragma once

#include <PubSubClient.h>
#include <CertStoreBearSSL.h>
#include <WiFiClientSecureBearSSL.h>

void init_mqtt(PubSubClient *&client,
							 const char *server,
							 BearSSL::WiFiClientSecure *&secureClient,
							 const char *root);
void ensure_mqtt_connected(PubSubClient *client, const char *user, const char *pass);
