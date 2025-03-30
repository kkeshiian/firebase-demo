#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Include Firebase helper
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi credentials
#define WIFI_SSID "HUAWEI"
#define WIFI_PASSWORD "k2sdm3TS"

// Firebase credentials
#define API_KEY "AIzaSyDASQlF6LjCGM8JkVEfU6f26BvvV5XLaAY"
#define DATABASE_URL "https://pabrik-demo-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println("\nConnected with IP: " + WiFi.localIP().toString());

    // Set API Key & Database URL
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    // Assign token callback
    config.token_status_callback = tokenStatusCallback;

    // **Anonymous Authentication**
    Serial.println("🔄 Signing in anonymously...");
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("✅ Anonymous sign-up berhasil!");
        signupOK = true;
    } else {
        Serial.printf("❌ Sign-up gagal: %s\n", config.signer.signupError.message.c_str());
    }

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop() {
    // Cek apakah koneksi WiFi masih ada
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi disconnected! Reconnecting...");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        delay(5000);
        return;
    }

    // Cek apakah token sudah siap sebelum mengirim data
    if (!Firebase.ready() || !signupOK) {
        Serial.println("⏳ Firebase belum siap, menunggu...");
        delay(2000);
        return;
    }

    if (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0) {
        sendDataPrevMillis = millis();

        // Simulasi data suhu dan kelembaban
        float suhu = random(200, 350) / 10.0;
        float kelembaban = random(400, 800) / 10.0;

        // Buat JSON untuk dikirim ke Firebase
        FirebaseJson json;
        json.set("suhu", suhu);
        json.set("kelembaban", kelembaban);

        Serial.println("🔄 Mengirim data JSON ke Firebase:");
        Serial.print("\t\"suhu\": ");
        Serial.println(suhu);
        Serial.print("\t\"kelembaban\": ");
        Serial.println(kelembaban);

        // Kirim JSON ke Firebase
        if (Firebase.RTDB.setJSON(&fbdo, "/sensor", &json)) {
            Serial.println("✅ Data berhasil dikirim ke Firebase!");
        } else {
            Serial.println("❌ Gagal mengirim data!");
            Serial.println(fbdo.errorReason());
        }
    }
}
