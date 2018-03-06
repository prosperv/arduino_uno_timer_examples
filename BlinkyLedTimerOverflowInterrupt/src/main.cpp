/*
 * Arduino 101: timer and interrupts
 * 2: Timer1 overflow interrupt example
 * created by RobotFreak

Using timer overfluw interrupt to toggle led.
The timer is preload to it can reach max value and overflow
sooner or later.
 */
#include <Arduino.h>
#define ledPin 13

long timeout_count = 34286;

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  // initialize timer1
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = 34286;            // preload timer 65536-16MHz/256/2Hz into counter
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}
// interrupt service routine that wraps a user defined function
// supplied by attachInterrupt
ISR(TIMER1_OVF_vect)
{
  TCNT1 = timeout_count;            // preload timer
  digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
}

void loop()
{
  delay(1000);
  if(Serial.available())
  {
    int input_ms = Serial.parseFloat();
    timeout_count = 65535 - (65536 * input_ms / 1000);
  }
}
