#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN  9
#define CSN_PIN 10

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
RF24 radio(CE_PIN, CSN_PIN); 

float data[2];

void setup() {
  Serial.begin(57600);
  delay(1000);
  
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  radio.printDetails();
  
  Serial.println("Started.");
}

void loop() {
  
  if ( radio.available() )
  {
    Serial.println("Reading");
    bool done = false;
    while (!done)
    {
      done = radio.read( data, sizeof(data) );
    }
    
    Serial.print("Sensor: ");
    Serial.println((int)data[0]);
    
    Serial.print("Temperature: ");
    Serial.println((String)data[1]);
  }
}
