// wifi_module.cpp
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <time.h>
#include <TZ.h>

#define EEPROM_SIZE 512
#define SSID_ADDRESS 0
#define PASS_ADDRESS 100

ESP8266WebServer server(80);
DNSServer dnsServer;

String ssid = "";
String password = "";
String name_wifi = "ESP8266-Rain-Detector";
bool apMode = true;

// --- Sync Time ---
void sync_time()
{
  // Sync time with NTP
  configTime(TZ_Europe_Berlin, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr); // Get current time

  // Wait for NTP sync
  while (now < 8 * 3600 * 2)
  {
    delay(100);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nNTP time synced");
}

// Clear kredensial
void clearEEPROM()
{
  Serial.println("🧹 Clearing EEPROM...");
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

// Handle halaman root
void handleRoot()
{
  if (server.method() == HTTP_POST)
  {
    String newSSID = server.arg("ssid");
    String newPass = server.arg("pass");

    Serial.println("User input: " + newSSID);
    WiFi.begin(newSSID.c_str(), newPass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
    {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConnected WiFi! Saving...");
      ssid = newSSID;
      password = newPass;

      char sBuffer[33] = {0};
      char pBuffer[65] = {0};

      ssid.toCharArray(sBuffer, sizeof(sBuffer));
      password.toCharArray(pBuffer, sizeof(pBuffer));

      EEPROM.put(SSID_ADDRESS, sBuffer);
      EEPROM.put(PASS_ADDRESS, pBuffer);
      EEPROM.commit();
    }
    else
    {
      Serial.println("\nFailed to connect WiFi. Stay in AP mode");
      clearEEPROM();
    }
  }

  int n = WiFi.scanNetworks();
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
      <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <style>
          body {
            font-family: sans-serif;
            padding: 20px;
            background: #f9f9f9;
            max-width: 480px;
            margin: 0 auto;
          }

          .wifi {
            margin: 10px 0;
            padding: 12px 16px;
            background: #ffffff;
            border: 1px solid #ccc;
            border-radius: 8px;
            cursor: pointer;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            display: flex;
            justify-content: space-between;
            align-items: center;
          }

          .wifi:hover {
            background: #f0f0f0;
          }

          .wifi span {
            flex: 1;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
          }

          .wifi .icon {
            margin-left: 10px;
            font-size: 20px;
            color: #007BFF;
          }

          #modal {
            display: none;
            position: fixed;
            top: 0; left: 0;
            width: 100%; height: 100%;
            background: rgba(0, 0, 0, 0.4);
          }

          #modalContent {
            background: white;
            padding: 20px;
            margin: 10% auto;
            width: 90%;
            max-width: 320px;
            border-radius: 8px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.3);
          }

          input[type=password], input[type=text] {
            width: 100%;
            padding: 10px;
            margin-top: 10px;
            border-radius: 6px;
            border: 1px solid #ccc;
            box-sizing: border-box;
          }

          .checkbox-container {
            display: flex;
            align-items: center;
            margin-top: 10px;
            font-size: 14px;
          }

          .checkbox-container input {
            margin-right: 8px;
          }

          button {
            padding: 10px 20px;
            margin-top: 15px;
            background: #007BFF;
            color: white;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            width: 100%;
          }

          button:hover {
            background: #0056b3;
          }
        </style>

      </head>
      <body>
        <h2>Pilih Jaringan WiFi</h2>
  )rawliteral";

  if (WiFi.status() == WL_CONNECTED)
  {
    html += R"rawliteral(
    <form method="POST" action="/forget">
      <button style="margin-top:30px;background:#dc3545">Lupakan Jaringan</button>
    </form>
  )rawliteral";
  }

  for (int i = 0; i < n; i++) {
    String s = WiFi.SSID(i);
    int enc = WiFi.encryptionType(i);
    bool isCurrent = (s == ssid && WiFi.status() == WL_CONNECTED);
    String extra = isCurrent ? "<br><small style='color:green'>Connected</small>" : "";

    html += "<div class='wifi' ";
    if (!isCurrent) {
      html += "onclick='selectWiFi(\"" + s + "\", " + String(enc) + ")'";
    } else {
      html += "style='opacity: 0.6; cursor: not-allowed;'";
    }
    html += ">";

    html += "<span>" + s + extra + "</span>";

    if (enc != AUTH_OPEN) {
      html += "<span class='icon'>🔒</span>";
    }
    html += "</div>";
  }

  html += R"rawliteral(
    <div id='modal'><div id='modalContent'>
      <h3 id='modalSSID'>SSID</h3>
      <form method='POST'>
        <input type='hidden' name='ssid' id='ssidInput'>
        <input type='password' name='pass' id='password' placeholder='Masukkan Password'>
        <div class='checkbox-container'><input type='checkbox' id='showPassword'><label for='showPassword'>Tampilkan Password</label></div>
        <button type='submit'>Sambungkan</button>
      </form></div></div>
      <script>
        function selectWiFi(ssid, enc) {
          if (enc === 0) document.forms[0].pass.value = "";
          document.getElementById('modalSSID').innerText = 'SSID: ' + ssid;
          document.getElementById('ssidInput').value = ssid;
          document.getElementById('modal').style.display = 'block';
        }
        window.onclick = (e) => { if (e.target.id == 'modal') document.getElementById('modal').style.display = 'none'; }
        document.getElementById('showPassword').addEventListener('change', function() {
          document.getElementById('password').type = this.checked ? 'text' : 'password';
        });
      </script>
      </body></html>)rawliteral";

  server.send(200, "text/html", html);
}

// Setup WiFi mode
void setup_wifi()
{
  EEPROM.begin(EEPROM_SIZE);
  delay(100);

  // Load saved credentials
  char s[33] = {0}; // +1 buat null-terminator
  char p[65] = {0}; // +1 buat null-terminator

  EEPROM.get(SSID_ADDRESS, s);
  EEPROM.get(PASS_ADDRESS, p);

  // Cek kalau datanya valid (print hex misal)
  Serial.print("Raw SSID HEX: ");
  for (int i = 0; i < 32; i++)
    Serial.print((uint8_t)s[i], HEX), Serial.print(" ");
  Serial.println();

  s[32] = '\0';
  p[64] = '\0';

  ssid = String(s);
  password = String(p);

  // Start dual mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(name_wifi);
  dnsServer.start(53, "*", WiFi.softAPIP());
  Serial.println("📡 SoftAP IP: " + WiFi.softAPIP().toString());

  Serial.println("EEPROM SSID: " + ssid);
  Serial.println("EEPROM PASS: " + password);

  if (ssid != "" && password != "")
  {
    delay(500);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("🔌 Trying saved WiFi...");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
    {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nConnected as STA");
      Serial.println("IP: " + WiFi.localIP().toString());
    }
    else
    {
      Serial.println("\n❌ Failed STA connection");
    }
  }
  else
  {
    Serial.println("⚠️ No valid credentials found.");
  }

  server.on("/", HTTP_ANY, handleRoot);
  server.on("/forget", HTTP_POST, []() {
    clearEEPROM();
    ESP.restart();
  });

  server.begin();
}

// Handle AP Mode Loop
void handle_ap_mode()
{
  dnsServer.processNextRequest();
  server.handleClient();

  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    if (input == "clear")
    {
      clearEEPROM();
      Serial.println("🗑️ Cleared creds. Restarting...");
      ESP.restart();
    }
  }
}
