/*  CODING IOT TRACKER SHUTTLE ITB SI BIRU 2025 
    Developed by : Raegrand Archamadeus, Zaki Azzamy, Kenny Pramanik,...
    Last Update  : 24/06/2025 
*/

#include <WiFi.h>
#include <Wire.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


/******** DEFINISI ********/
// VARIABEL
String databasePath;
String mhsPath;
String latPath;
String lgtPath;
String terPath;

//WIFI Definition
const char *ssid = "ITB IoT";
const char *password = "";

//Firebase Definition
#define API_KEY "AIzaSyBFXji0tc6nLJfv-NFcuKod-IHU-5jTZX8"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "itbshuttletracker@gmail.com"
#define USER_PASSWORD "itbshuttletracker1920"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://esp32-firebase-demo-f2551-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseData auth;
FirebaseConfig config;

String uid;
String databasePath;
String latPath, lgtPath, timePath; 

//GPS
Hardwareserial gpsSerial(1);
const int RXPin = 16;
const int TXPin = 17;

struct GPSRawData {
    double latitude;
    double longitude;
    string timestamp;
    int day, month, year;
    int hours, minutes, seconds;
    String timestamp;
};

struct GPSData {
    double latitude;
    double longitude;
    double timestamp;
};

GPSRawData latestGPSRawData;
GPSData latest GPSData;
bool gpsDataValid = false;
String gpsBuffer = "";

//FUNGSI
void initWiFi();
void sendString(String path, String value);
void sendFloat(String path, float value);
void reconnectWiFi();

/******** PROGRAM ********/
void setup() {
  Serial.begin(115200);
  //Setup GPS
  gpsSerial.begin(9600, SERIAL_8N1, RXPin, TXPin);
    
  //Setup Wifi
  initWiFi();

  config.api_key = API_KEY;
  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;
  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);
    // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/ShuttleData/" + uid + "/Rute1/Bus1";
  // Update database path for sensor readings
  mhsPath = databasePath + "/countMhs";
  latPath = databasePath + "/latitude";
  lgtPath = databasePath + "/longitude";
  terPath = databasePath + "/Terminal";
  timePath = databasePath + "/timestamp";
}

void loop() {
  //if WiFi disconnected
  if (WiFi.status() != WL_CONNECTED){
    void reconnectWiFi();
  }
    while(gpsSerial.available()) {
        char c = gpsSerial.read();
        gpsBuffer += c;
        if (c == '\n') {
            processGPSData(gpsData);
            gpsData = "";
        }
    }
    static unsigned long lastSent = 0;
    if(millis() - lastSent > 10000 && gpsDataValid){
        sendFloat(latPath, latestGPSData.latitude);
        sendFloat(lgtPath, latestGPSData.longitude);
        sendString(timePath, latestGPSData.timestamp);
        lastSent = millis();
    }
}

/**** Fungsi WiFI *****/
// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}
// Reconnect WiFi
void reconnectWiFi(){
  Serial.println("WiFi not connected! Reconnecting...");
  WiFi.disconnect(true, true);           // drop any old session
  WiFi.setSleep(false);                  // keep radio awake
  WiFi.begin(ssid, password); 
  delay(2000);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi reconnect failed");
    return;
  }
  Serial.println("WiFi reconnected");
}

/**** Fungsi Firebase *****/
// Mengirim string ke database
void sendString(String path, String value){
  String temp=" ";
  value = value + temp;
  if (Firebase.RTDB.setString(&fbdo, path.c_str(), value)){
  Serial.print("Writing value: ");
  Serial.print (value);
  Serial.print(" on the following path: ");
  Serial.println(path);
  Serial.println("PASSED");
  Serial.println("PATH: " + fbdo.dataPath());
  Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}
// Mengirim float ke database
void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

//Parsing GPS
void processGPSData(String raw){
    if(raw.startswith("$GPGGA")){
        parseGPGGA(raw);
        convertToLocalTime();
    } else if (raw.startsWith("$GPRMC")){
        parseGPRMC(raw);
    }
}

void parseGPGGA(String gpgga) {
  String tokens[15];
  int idx = 0, lastIdx = 0;
  for (int i = 0; i < gpgga.length(); i++) {
    if (gpgga[i] == ',' || gpgga[i] == '*') {
      tokens[idx++] = gpgga.substring(lastIdx, i);
      lastIdx = i + 1;
    }
  }

  if (idx > 6) {
    String utcTime = tokens[1];
    latestGPSRawData.hours = utcTime.substring(0, 2).toInt();
    latestGPSRawData.minutes = utcTime.substring(2, 4).toInt();
    latestGPSRawData.seconds = utcTime.substring(4, 6).toInt();

    latestGPSRawData.latitude = nmeaToDecimal(tokens[2]);
    latestGPSRawData.longitude = nmeaToDecimal(tokens[4]);

    latestGPSData.latitude = latestGPSRawData.latitude;
    latestGPSData.longitude = latestGPSRawData.longitude;

    gpsDataValid = (latestGPSData.latitude != 0 || latestGPSData.longitude != 0);
  }
}

void parseGPRMC(String gprmc) {
  String tokens[15];
  int idx = 0, lastIdx = 0;
  for (int i = 0; i < gprmc.length(); i++) {
    if (gprmc[i] == ',' || gprmc[i] == '*') {
      tokens[idx++] = gprmc.substring(lastIdx, i);
      lastIdx = i + 1;
    }
  }

  if (idx > 9) {
    String utcDate = tokens[9];
    latestGPSRawData.day = utcDate.substring(0, 2).toInt();
    latestGPSRawData.month = utcDate.substring(2, 4).toInt();
    latestGPSRawData.year = 2000 + utcDate.substring(4, 6).toInt();
  }
}

void convertToLocalTime() {
  int offsetHours = 7; // Waktu Indonesia Barat
  latestGPSRawData.hours += offsetHours;

  if (latestGPSRawData.hours >= 24) {
    latestGPSRawData.hours -= 24;
    latestGPSRawData.day++;
  }

  char timeBuf[25];
  snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d %02d:%02d:%02d",
           latestGPSRawData.year, latestGPSRawData.month, latestGPSRawData.day,
           latestGPSRawData.hours, latestGPSRawData.minutes, latestGPSRawData.seconds);
  latestGPSRawData.timestamp = String(timeBuf);
  latestGPSData.timestamp = latestGPSRawData.timestamp;

  Serial.println("Timestamp: " + latestGPSData.timestamp);
}

double nmeaToDecimal(String coord) {
  if (coord == "") return 0.0;
  double raw = coord.toDouble();
  int deg = int(raw / 100);
  double min = raw - (deg * 100);
  return deg + (min / 60.0);
}
