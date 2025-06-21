# 🌦️ ESP8266 Weather IoT Station

ESP8266 Weather Monitoring System dengan dual-mode WiFi (STA + AP), koneksi MQTT TLS, sensor suhu, kelembaban, tekanan udara, pendeteksi hujan, serta error handler dan web konfigurasi SSID.

---

## 📦 Fitur Utama

- 🌐 Dual-mode WiFi: Station + Access Point
- 🔐 MQTT TLS via HiveMQ Cloud (CertStore + LittleFS)
- 🌡️ Sensor: DHT22, BMP280, Rain Sensor
- 🔔 Buzzer notifikasi hujan & tekanan drop
- 📤 MQTT Topic:
  - `weather/rain` → Publish data sensor
  - `device/error` → Kirim laporan error
  - `device/status` → Heartbeat & recovery
- 🌍 Web UI konfigurasi SSID (tanpa endpoint tambahan)
- 🔄 EEPROM penyimpanan SSID/PASS
- ♻️ Tombol Reset jaringan jika sudah terkoneksi

---

## 📁 Struktur Modular

```
├── src/
│   ├── main.cpp                # Entry point
│   ├── wifi_module.cpp         # Setup WiFi AP + STA
│   ├── mqtt_module.cpp         # MQTT TLS + cert
│   ├── sensor_module.cpp       # Baca sensor + publish
│   ├── error_module.cpp        # Error handling
│   └── utils.h                 # Konstanta & helper
├── data/
│   ├── certs.idx               # Index CA cert
│   └── certs.ar                # Sertifikat CA
├── platformio.ini              # Konfigurasi project
```

---

## ⚙️ Dependencies

```ini
board = nodemcuv2
framework = arduino
platform = espressif8266
lib_deps =
  adafruit/DHT sensor library
  adafruit/Adafruit BMP280 Library
  bblanchon/ArduinoJson
  knolleary/PubSubClient
  esp8266/Arduino
```

Tambahkan filesystem upload tools (optional):
```bash
pio run --target uploadfs
```

---

## 🧪 MQTT Topic Struktur

### `weather/rain`
```json
{
  "temperature": 29.4,
  "humidity": 70.2,
  "pressure": 1005.5,
  "altitude": 61.5,
  "rain_detected": true,
  "rain_level": 512,
  "local_conditions": {
    "conditions": "drizzle",
    "sudden_pressure_drop": false
  }
}
```

### `device/error`
```json
{
  "type": "error",
  "source": "sensor_module",
  "message": "Failed to read DHT!"
}
```

### `device/status`
```json
{
  "status": "ok"
}
```

---

## 🧠 Cara Kerja Umum

1. Boot → masuk mode **AP + STA**
2. Jika SSID valid, connect otomatis dan tampil sebagai *Connected*
3. UI HTML tampilkan semua SSID terdekat + form input password
4. Sensor membaca data dan kirim secara rutin ke MQTT
5. Error otomatis dikirim ke `device/error`, dan akan kirim recovery jika normal
6. Tombol *Lupakan Jaringan* tersedia jika device sudah terkoneksi

---

## 🌎 Mode Ap (acces point)
1. Koneksikan dengan wifi Esp ```ESP8266-Rain-Detector```
2. Jika sudah tersambung masuk ke ip ```192.168.4.1```
3. Koneksi kan wifi yang akan di sambungkan

---

## 📌 Catatan Debugging

- Pastikan waktu real-time disinkronisasi (`setSyncTime()`), jika tidak TLS gagal
- Cek cert store berhasil di-load (`certs > 0`)
- MQTT `rc = -2` artinya TLS gagal connect (biasanya karena cert atau waktu)
- Gunakan `serializeJsonPretty()` saat debugging via Serial

---

## 🧼 Contoh WebUI

- Tampilkan SSID aktif
- SSID lain bisa diklik untuk login
- Jika sudah konek, tampilkan tombol *Lupakan Jaringan* yang POST ke `/forget`

---

## ✨ Rencana Pengembangan

- Auto reconnect jika disconnect MQTT/WiFi
- OTA update via WiFi
- Dashboard Web untuk log historis via Firebase atau lainnya

---

## 👥 My Tim
- [Siraj Nurul Bilhaq](https://github.com/Qw3rty101) - [Email](siraj.nurul@gmail.com)
- [Fahmi Idris Anjounghan](https://github.com/Qw3rty101) - [Email](idrisanjounghan313@gmail.com)