/*  CODING IOT TRACKER SHUTTLE ITB SI BIRU 2025 
    Developed by : Raegrand Archamadeus, Zaki Azzamy, Kenny Pramanik,...
    Last Update  : 24/06/2025 
*/

#include <WiFi.h>
#include <Wire.h>
#include <HardwareSerial.h>
/******** DEFINISI ********/
//WIFI
const char *ssid = "ITB IoT";
const char *password = "";

//MQTT
const char* mqtt_server = "192.168.17.222";
const int   mqtt_port   = 1883;
const char* gps_topic = "IoTSiBiru/GPS";


void reconnectMQTT();

/******** PROGRAM ********/

/**** Fungsi MQTT *****/
void reconnectMQTT() {
  static int attempt = 0;
  static unsigned long lastAttempt = 0;
  
  // Only try every 5 seconds
  if (millis() - lastAttempt < 5000 && lastAttempt != 0) {
    return;
  }
  lastAttempt = millis();
  attempt++;
  
  Serial.printf("\nMQTT Attempt #%d\n", attempt);
  Serial.printf("Broker: %s:%d\n", mqtt_server, mqtt_port);
  
  // 1. Check WiFi first
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected! Reconnecting...");
    WiFi.disconnect(true, true);           // drop any old session
    WiFi.mode(WIFI_STA);                   // station mode
    WiFi.setSleep(false);                  // keep radio awake
    WiFi.begin(ssid, password); 
    delay(2000);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi reconnect failed");
      return;
    }
    Serial.println("WiFi reconnected");
  }
  
  // 2. Resolve hostname to IP
  IPAddress brokerIP;
  if (!WiFi.hostByName(mqtt_server, brokerIP)) {
    Serial.println("DNS lookup failed");
    return;
  }
  Serial.printf("Resolved IP: %s\n", brokerIP.toString().c_str());
  
  // 3. Test raw TCP connection
  WiFiClient testClient;
  Serial.print("Testing TCP connection...");
  if (testClient.connect(brokerIP, mqtt_port)) {
    Serial.println(" SUCCESS");
    testClient.stop();
  } else {
    Serial.println(" FAILED");
    Serial.println("Check broker is running and firewall allows port 1883");
    return;
  }
  
  // 4. Attempt MQTT connection
  Serial.print("MQTT connecting...");
  if (client.connect("ESP32Cam")) {
    Serial.println(" SUCCESS");
    client.subscribe(result_topic);
    attempt = 0;  // Reset attempt counter on success
  } else {
    Serial.print(" FAILED, rc=");
    Serial.print(client.state());
    
    // Detailed error mapping
    switch (client.state()) {
      case -4: Serial.println(" (Connection timeout)"); break;
      case -3: Serial.println(" (Connection lost)"); break;
      case -2: Serial.println(" (Connect failed)"); break;
      case -1: Serial.println(" (Disconnected)"); break;
      case 1: Serial.println(" (Bad protocol)"); break;
      case 2: Serial.println(" (Bad client ID)"); break;
      case 3: Serial.println(" (Server unavailable)"); break;
      case 4: Serial.println(" (Bad credentials)"); break;
      case 5: Serial.println(" (Unauthorized)"); break;
      default: Serial.println(" (Unknown error)"); break;
    }
    
    // Additional diagnostics
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
  }
}

void setup() {

  //Setup Wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  reconnectMQTT();
}

void loop() {
  //if MQTT disconected
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();


  client.publish(gps_topic, gps, gps_length)//Publish data ke MQTT
}
