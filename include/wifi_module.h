// wifi_module.h
#pragma once

// Inisialisasi waktu
void sync_time();

// Inisialisasi WiFi (AP + STA), mulai web server
void setup_wifi();

// Jalankan loop untuk DNS dan web server
void handle_ap_mode();
