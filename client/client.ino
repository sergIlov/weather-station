#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <avr/sleep.h>
#include <avr/power.h>

#define LED 3
#define ONE_WIRE_BUS 5

#define CE_PIN   9
#define CSN_PIN 10

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const uint64_t pipe = 0xE8E8F0F0E1LL; 
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
RF24 radio(CE_PIN, CSN_PIN); 

float data[2];

volatile int f_wdt=1;
int loops = 0;

ISR(WDT_vect)
{
  if(f_wdt == 0)
  {
    f_wdt = 1;
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  
  Serial.begin(57600);
  initTimer();
  log("Setup");
  sensors.begin();

  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[0]);
  //radio.openReadingPipe(1,pipes[1]);  
}

void loop() {
  
  if(f_wdt == 1)
  {
    f_wdt = 0;  
  
    if (loops == 0) {
      radio.startListening();
      radio.stopListening();
      log("READING TEMPERATURE...");
      float temperature = getTemperature();
      log("Temperature is " + (String)temperature);
  
      data[0] = 1;
      data[1] = temperature;
   
      bool ok = radio.write( &data, sizeof(data) );

      if (ok) {
        log("Ok");
      }
      else {
        log("Fail");
      }
  
      //int blinks = round(temperature) - 20;
      //blink(blinks);
      loops = 7*5; //4 - 35 c
    }

    loops = loops -1;
    enterSleep();
  }
}

float getTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

void blink(int times) {
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
  delay(500);
  for(int i = 1; i <= times; i++) {
    digitalWrite(LED, HIGH);
    delay(300);
    digitalWrite(LED, LOW);
    delay(200);
  }
}

void log(String message) {
  Serial.println(message);
  delay(1000);
}

void initTimer() {
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
}

void enterSleep()
{
  //log("sleep");
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  sleep_enable();

  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 

  ADCSRA = 0;

  /* Disable all of the unused peripherals. This will reduce power
   * consumption further and, more importantly, some of these
   * peripherals may generate interrupts that will wake our Arduino from
   * sleep!
   */
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  
  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the timer timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}

