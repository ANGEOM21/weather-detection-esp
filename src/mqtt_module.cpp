// mqtt_module.cpp
#include "mqtt_module.h"
#include <LittleFS.h>
#include <ESP8266WiFi.h>

// callback MQTT
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  // callback
}

// init MQTT
void init_mqtt(
    PubSubClient *&client,
    const char *server,
    BearSSL::WiFiClientSecure *&secureClient,
    const char *root_ca
  )
{

  secureClient = new BearSSL::WiFiClientSecure();
  secureClient->setBufferSizes(512, 512);
  static BearSSL::X509List cert(root_ca);
  secureClient->setTrustAnchors(&cert);
  // secureClient->setInsecure();

  client = new PubSubClient(*secureClient); 
  client->setServer(server, 8883);
  client->setCallback(mqtt_callback);
}

// connect MQTT
void ensure_mqtt_connected(PubSubClient *client, const char *user, const char *pass)
{
  while (!client->connected())
  {
    Serial.print("Connecting to MQTT... ");
    String clientId = "ESP8266Client-" + String(ESP.getChipId());

    if (client->connect(clientId.c_str(), user, pass))
    {
      Serial.println("Connected");
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(client->state());

      switch (client->state())
      {
      case MQTT_CONNECTION_TIMEOUT:
        Serial.println("⏱️ Connection timeout");
        break;
      case MQTT_CONNECTION_LOST:
        Serial.println("❌ Connection lost");
        break;
      case MQTT_CONNECT_FAILED:
        Serial.println("❌ Connection failed");
        break;
      case MQTT_DISCONNECTED:
        Serial.println("🔌 Disconnected");
        break;
      case MQTT_CONNECTED:
        Serial.println("✅ Connected");
        break;
      case 1: // 0x01
        Serial.println("📛 Refused: Unacceptable protocol version");
        break;
      case 2: // 0x02
        Serial.println("📛 Refused: Identifier rejected");
        break;
      case 3: // 0x03
        Serial.println("📛 Refused: Server unavailable");
        break;
      case 4: // 0x04
        Serial.println("📛 Refused: Bad username or password");
        break;
      case 5: // 0x05
        Serial.println("📛 Refused: Not authorized");
        break;
      default:
        Serial.print("❓ Unknown error: ");
        Serial.println(client->state());
        break;
      }
      delay(5000);
    }
  }
}
