#include <SPI.h>
#include <LoRa.h>

// LoRa Pins (match your wiring)
#define SS      5    // GPIO5 (NSS)
#define RST     14   // GPIO14 (RST)
#define DIO0    2    // GPIO2 (DIO0 for interrupts)
#define LED_PIN 6    // GPIO6 (LED for RX indication)

String receivedMessage;
int rssi;
float snr;

void setup() {
  Serial.begin(115200);
  while (!Serial);   // Wait for Serial Monitor

  Serial.println("LoRa Receiver");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {  // Match TX frequency!
    Serial.println("LoRa init failed. Check wiring!");
    while (1);
  }

  // Enable receive mode + interrupt
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("Waiting for messages...");
}

void loop() {
  // Empty (interrupt-driven)
}

// Interrupt handler
void onReceive(int packetSize) {
  if (packetSize == 0) return;

  // Read packet
  receivedMessage = "";
  while (LoRa.available()) {
    receivedMessage += (char)LoRa.read();
  }

  // Get signal stats
  rssi = LoRa .packetRssi();
  snr = LoRa.packetSnr();

  // Print to Serial
  Serial.print("Received: ");
  Serial.println(receivedMessage);
  Serial.print("RSSI: ");
  Serial.println(rssi);
  Serial.print("SNR: ");
  Serial.println(snr);

  // Blink LED
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
}