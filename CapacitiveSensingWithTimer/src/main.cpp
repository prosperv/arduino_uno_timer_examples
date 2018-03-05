/*
  Capacitive sensing using timer and timer interrupts.

  Hardware Requirements:
    - Arduino Uno
    - Resistor(s) 100k-10M ohm
    - Capacitive element (ig wire, foil, coins, fruits)
    - Very small capacitor 22pF-1nF (optional)

  Setup:
  Connect the resistor between the send and recieve pin.
  Connect a capacitive element to the recieve pin.
  Upload the sketch and open the serial port.


  How capacitive sensing work.
  When the send pin goes high, it takes time for the recieve pin to detect that
  change. That time is base on the resistance between the send and receieve pin
  and the capacitance of the recieve pin. As the resistance or capacitance 
  increases, the long it takes for the the recieve pin to detect that change.
  By touching the capacitive element, you are introducing more capacitance
  because your body acts like a capacitor. Thus there will be a time difference
  between when you're not touching the capacitive element or 'sensor' and when
  you are.
*/

// Registers are defined in the toolchain folder.
// Specifically for the atmega328p you can find the defines here.
// .platformio\packages\toolchain-atmelavr\avr\include\avr\iom328p.h


#include <Arduino.h>

#define prt Serial.print
#define prtln Serial.println

// The WGM control bits are part of TCCR1A and TCCR1B registers
// These bits control the behavior of the timer.
// For now see datasheet for more description.
#define TIMER1_WGM_10 ((0<<WGM11)|(0<<WGM10))
#define TIMER1_WGM_32 ((0<<WGM13)|(0<<WGM12))

/*
  The Clock Select bits controls the source of the Clock as well as the
  precaler which will be the focus of this sketch. Setting Prescaler to 1
  with a max frequency of ~16Mhz or prescaler to 8 with a frequency of 2Mhz,
  seems work best.
  Prescaling | CS12 | CS11 | CS10
    clk/1    |   0  |   0  |  1
    clk/8    |   0  |   1  |  0
    clk/64   |   0  |   1  |  1
    clk/256  |   1  |   1  |  0
    clk/1024 |   1  |   0  |  1
*/
#define TIMER1_CS ((0<<CS12)|(1<<CS11)|(0<<CS10))

// The ICES1 controls if an event will be triggered on a rising edge (set 1) or
// falling edge (clear 0).
// The ICNC1 controls hardware filtering for the input capture pin.
#define TIMER1_IC ((1<<ICNC1)|(1<<ICES1))

// Variable to hold the capture timestamp
volatile long isr_capturedCount = 0;
// Not very necessary but it helps to prevent print the same
// reading multiple times
volatile bool readyToRead = false;

// The input capture pin is tied to pin 8 so it cannot be used.
const int recievePin = 8;
// This pin could just be any other pin
const int sendPin = 3;


inline void disableInputCapture()
{
  TIMSK1 &= ~(1<<ICIE1);
}
inline void enableInputCapture()
{
  TIMSK1 |= (1<<ICIE1);
}

inline void resetPins()
{
  // Discharge capacitor
  digitalWrite(sendPin, LOW);

  pinMode(recievePin, OUTPUT);
  digitalWrite(recievePin, LOW);
  delay(1);
  pinMode(recievePin, INPUT);
  //Reset
  ICR1 = 0;
  digitalWrite(sendPin, HIGH);
}

void setup() {
  // Disable interrups until we're setup and ready.
  noInterrupts();
  Serial.begin(9600);

  TCCR1A = TIMER1_WGM_10;
  prt("TCCR1A: "); prtln(TCCR1A, BIN);
  TCCR1B = (TIMER1_IC|TIMER1_WGM_32|TIMER1_CS);
  prt("TCCR1B: "); prtln(TCCR1B, BIN);
  TIMSK1 = ((1<<ICIE1)|(1<<TOIE1));
  prt("TIMSK1: "); prtln(TIMSK1, BIN);

  delay(2000);
  // Enable interupts
  interrupts();
  prtln("Ready Player One");
}

void loop()
{
  if (readyToRead)
  {
    prt(" ");
    prtln(isr_capturedCount);
    readyToRead = false;
  }
  delay(50);
}

// This ISR function is called whenever timer1 overflows occurs. This
// essentially become the timeout for capacitance sensing and reset the system.
ISR(TIMER1_OVF_vect)
{
  // Prep the recieve pin and
  pinMode(recievePin, INPUT);
  digitalWrite(sendPin, HIGH);
  // prt("R");
  enableInputCapture();
}

// This ISR function is called whenever the input capture pin transision from
// LOW to HIGH, as long as the interrupt is enabled.
ISR(TIMER1_CAPT_vect)
{
  // Disable he capture pin so we don't overwrite the first captured timestamp
  // before the counter resets
  disableInputCapture();
  //Read count
  isr_capturedCount = ICR1;
  // prt("C");
  readyToRead = true;

  // Discharge capacitor
  digitalWrite(sendPin, LOW);
  pinMode(recievePin, OUTPUT);
  digitalWrite(recievePin, LOW);
}
