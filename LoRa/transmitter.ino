 #include <SPI.h>
#include <LoRa.h>

// LoRa Pins (match your wiring)
#define SS      5    // GPIO5 (NSS)
#define RST     14   // GPIO14 (RST)
#define DIO0    2    // GPIO2 (DIO0, unused in TX but required by library)
#define LED_PIN 4   // GPIO34 (LED for TX indication - WARNING: INPUT ONLY!)

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);  // Wait for Serial Monitor (optional)

  Serial.println("LoRa Transmitter");

  // Initialize LED pin (NOTE: GPIO34 is input-only, replace if needed!)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {  // Use 868E6/915E6 for other regions
    Serial.println("LoRa init failed. Check wiring!");
    while (1);
  }
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // Blink LED (if using a different pin)
  digitalWrite(LED_PIN, HIGH);

  // Send LoRa packet
  LoRa.beginPacket();
  LoRa.print("Hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  digitalWrite(LED_PIN, LOW);
  counter++;

  delay(5000);  // Send every 5 seconds
}