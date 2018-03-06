/* Arduino 101: timer and interrupts
   1: Timer1 compare match interrupt example
   created by RobotFreak
   modified by Prosper

   When the timer (TCNT1) match the compare restister (OCR1A),
   an interrupt will trigger and reset the timer.
*/

#include <Arduino.h>
#define ledPin 13

long timeout_count = 31250;

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  // initialize timer1
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 31250;            // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC (Clear Timer on Compare) mode
  TCCR1B |= (1 << CS12);    // 256 prescaler -> 62500Hz timer clock
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts

    Serial.print("TCCR1B: ");
    Serial.println(TCCR1B, BIN);
    Serial.print("TIMSK1: ");
    Serial.println(TIMSK1, BIN);
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  digitalWrite(ledPin, digitalRead(ledPin) ^ 1);   // toggle LED pin
}

void loop()
{
  delay(1000);
  if(Serial.available())
  {
    int input_ms = Serial.parseFloat();
    timeout_count = (65535 * input_ms / 1000);
    OCR1A = timeout_count;
  }
}
