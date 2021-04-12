#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "Timer.h"                     //http://github.com/JChristensen/Timer
#include <Average.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

Timer t;

// Reserve space for 20 entries in the average bucket.
// Change the type between < and > to change the entire way the library works.
byte entries = 30;
Average<float> voltavg(entries);

float vmeas;
float vbatt;
#include "pitches.h"

// notes in the melody:
int melody[] = {

  NOTE_DS8, NOTE_E7, NOTE_F5, NOTE_DS8
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  16, 8, 8, 4,
};

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int interval;
long currentmillis;
float Vcc;
float v;

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(8, OUTPUT);

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < (sizeof(melody) / sizeof(int)); thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.50;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  beep();
  //delay(500);
  Vcc = readVcc() / 1000;

  currentmillis = millis();
}

void loop() {

  Vcc = readVcc() / 1000;
  digitalWrite(2, LOW);
  v = analogRead(A0) * Vcc / 1024;
  voltavg.push(v);
  vmeas = voltavg.mean();
  vbatt = vmeas * 5;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("VIN  Vmeas Vbatt");
  lcd.setCursor(0, 1);
  lcd.print(Vcc);
  lcd.setCursor(5, 1);
  lcd.print(vmeas);
  lcd.setCursor(11, 1);
  lcd.print(vbatt);

  if (vbatt >= 12.62) { //>90%
    interval = 10000;
  }
  if (vbatt >= 11.96 && vbatt < 12.62) { // 40%-90%
    interval = 6000;
  }
  if (vbatt >= 11.66 && vbatt < 11.96) { // 20%-40%
    interval = 2500;
  }
  if (vbatt >= 11.51 && vbatt < 11.66) { // <20%
    interval = 500;
  }
  if (vbatt < 11.51) { // <10%
    digitalWrite(8, HIGH);
    digitalWrite(2, HIGH);
    while (1);
  }

  if ( millis() > (currentmillis + interval) && vbatt > 11.51) {
    beep();
    currentmillis = millis();
  }
  delay(50);

}

long readVcc() {

  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(5); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;

}

void beep() {
  tone(8, 5000, 15);
}

