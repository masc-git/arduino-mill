
#include <QuickPID.h> // https://github.com/Dlloydev/QuickPID
/*
 * autotune: https://github.com/osPID/osPID-Firmware/blob/master/osPID_Firmware/PID_AutoTune_v0.cpp
 * 
 * from https://github.com/br3ttb/Arduino-PID-Library/blob/master/examples/PID_PonM/PID_PonM.ino
 * descrition http://brettbeauregard.com/blog/2017/06/introducing-proportional-on-measurement/
 * 
 */
/*
#include <PID_v1.h>

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint,2,5,1,P_ON_M, DIRECT); //P_ON_M specifies that Proportional on Measurement be used
                                                            //P_ON_E (Proportional on Error) is the default behavior

void setup()
{
  //initialize the variables we're linked to
  Input = analogRead(0);
  Setpoint = 100;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
}

void loop()
{
  Input = analogRead(0);
  myPID.Compute();
  analogWrite(3,Output);
}
*/

/*
 * 338cm = 14 pulses
 * 941 = 1° = 1,75%, 118 = 7,2° = 12,63%
 * 
 * 
 */


#define CLOCKFREQUENCY  160000000
// custom frequency is 1/10Hz
#define MAXFREQUENCY          200
#define MINFREQUENCY           10
#define DEFAULTFREQ           100
#define DEFAULTINC              5

#define HEIGHTMIN             941
#define HEIGHTMAX             118

volatile long increment;
volatile long frequency;  // in 1/10
volatile long oldFrequency;
volatile long period;
volatile long time0;
volatile long time1;
volatile long time2;

//Define Variables we'll be connecting to
//float Setpoint, Input, Output;

float speedtarget = 4.00;
float speedsoll;
float speedkm;

#define heartMaxArray 10 
uint8_t heartCount;
volatile long heartTime;
uint16_t heartArray[heartMaxArray];


//Define the aggressive and conservative and POn Tuning Parameters
float aggKp = 4, aggKi = 0.1, aggKd = 0.5;
float consKp = 2, consKi = 0.05, consKd = 0.25;

//Specify the links
QuickPID myPID(&speedkm, &speedsoll, &speedtarget);

const byte ENABLE_PIN     = A2;
const byte FAST_PIN       = 9;
const byte SLOW_PIN       = 11;
const byte HIGH_PIN       = 7;
const byte LOW_PIN        = 5;
const byte analogPin      = A0;
const byte safetyPin      = A4;
const byte speedPin       = 3;  // input pin that the interruption will be attached to

const byte FAST_BTN       = 19;
const byte SLOW_BTN       = 17;
const byte HIGH_BTN       = 13;
const byte LOW_BTN        = 15;

const byte START_BTN      = 23;
const byte STOP_BTN       = 21;
const byte UP_BTN         = 29;
const byte DOWN_BTN       = 27;
const byte ENTER_BTN      = 25;

const byte LED_PIN        = 31;

const byte RATE_PIN       = 2;
const byte RATE_POLAR_PIN = 4;
const byte BUZZ_PIN       = 6;

String pin_str = "";        // reading from serial input


bool enable = false; 
bool target = false; 
bool acc = false; 
bool dec = false; 
bool up = false; 
bool down = false; 
bool speedStart = false; 


void InitTimer() {
  // Clear Timer/Counter Control Register for Interrupt 1
  TCCR1A = 0;       // Clear TCCR1A/B registers
  TCCR1B = 0;
  TCCR2A = 0;       // Clear TCCR1A/B registers
  TCCR2B = 0;
  TCNT1 = 0;        // Initialize counter to 0

  pinMode(FAST_PIN, OUTPUT);
  pinMode(SLOW_PIN, OUTPUT);

  // Compare register for TIMER1: (16mHz / frequency - `) = 15999 = 0x3E7F / 2^6
  period = (CLOCKFREQUENCY / ( frequency * 16 * 128) ) -1;
  OCR1A = period;
  
  // Timer/Counter Control Register for Interrupt 1 on register B
  TCCR1B |= (1 << WGM12);    // Mode 4, CTC--Clear Timer on Compare
  TCCR1B |= (1 << CS10);     // Clock Select Bit, no prescaling
  TCCR1B |= (1 << CS12);     // Clock Select Bit, no prescaling
  TIMSK1 |= (1 << OCIE1A);   // The value in OCR1A is used for compare
}

void setup() {
  Serial.begin(115200);

  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(FAST_PIN, OUTPUT);
  pinMode(SLOW_PIN, OUTPUT);
  pinMode(HIGH_PIN, OUTPUT);
  pinMode(LOW_PIN, OUTPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  pinMode(FAST_BTN, INPUT);
  pinMode(SLOW_BTN, INPUT);
  pinMode(HIGH_BTN, INPUT);
  pinMode(LOW_BTN, INPUT);
  pinMode(START_BTN, INPUT);
  pinMode(STOP_BTN, INPUT);

  pinMode(ENTER_BTN, INPUT);
  pinMode(DOWN_BTN, INPUT);
  pinMode(UP_BTN, INPUT);

  
  pinMode(analogPin, INPUT);
  pinMode(safetyPin, INPUT);
  pinMode(speedPin, INPUT_PULLUP);

  pinMode(RATE_PIN, INPUT);
  pinMode(RATE_POLAR_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(speedPin), speedSensor, FALLING);
  attachInterrupt(digitalPinToInterrupt(RATE_PIN), heartSensor, RISING);

  frequency = DEFAULTFREQ;  // Start off with 100Hz
  
  cli();                    // Turn interrupts off
  InitTimer();              // Set up timer interrupt
  sei();                    // Turn interrupts on

  oldFrequency = frequency - 1;
  CheckFrequency();
  increment = DEFAULTINC;

  digitalWrite(LED_PIN, HIGH);

  //turn the PID on
  myPID.SetMode(myPID.Control::automatic);

}

void loop() {
  // every 10s diagnostics
  if (time1 < millis()) {
    time1 = millis() + 10000;
    Serial.print(" steigung: ");           
    Serial.print(heightSensor(),1);         
    Serial.print(" puls: ");           
    Serial.println(heartBeat());         
  }

  if (digitalRead(START_BTN)) {
    if (digitalRead(safetyPin)) {
      enable = true; 
      speedStart = false; 
      digitalWrite(ENABLE_PIN, HIGH);
//      Serial.println("running... ");
    } else {
//      Serial.println("safety circuit open! ");
    }
  }        
  if (digitalRead(STOP_BTN)) {
    enable = false; 
    speedStart = false; 
    target = false;
    digitalWrite(ENABLE_PIN, LOW);
//    Serial.println("stop. ");
        
  }
  if (digitalRead(SLOW_BTN)) {
    if (enable) {
      if (!acc) {
          acc = false;
          dec = true;
//          Serial.print("slowerBTN... ");
      }
    }
  }
  else {
    dec = false;
    //acc = false;
  }
  if (digitalRead(FAST_BTN)) {
    if (enable) {
      if (!dec) {
          dec = false;
          acc = true;
 //         Serial.println("fasterBTN... ");
      }
    }
  }
  else {
    //dec = false;
    acc = false;
  }
  
  if (digitalRead(HIGH_BTN)) {
    if (!down && (digitalRead(safetyPin))) {
        digitalWrite(HIGH_PIN, HIGH);
        up = true;
//        Serial.println("higherBTN... ");
    }
  }
  else {
    digitalWrite(HIGH_PIN, LOW);
    up = false;
  }        
  
  if (digitalRead(LOW_BTN)) {
    if (!up && (digitalRead(safetyPin))) {
        digitalWrite(LOW_PIN, HIGH);
        down = true;
//        Serial.println("lowerBTN... ");
    }
  }
  else {
    digitalWrite(LOW_PIN, LOW);
    down = false;
  }        

/*           
  if (digitalRead(UP_BTN))
    Serial.println(" highW ");         
  if (digitalRead(DOWN_BTN))
    Serial.println(" downW ");         
  if (digitalRead(ENTER_BTN))
    Serial.println(" enterW ");         
*/   

  if (!digitalRead(safetyPin)) {
    enable = false; 
    speedStart = false; 
    target = false;
    digitalWrite(ENABLE_PIN, LOW);
    Serial.println("safety circuit open! ");
    Serial.println("stop. ");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(500);
  }
  else {
    digitalWrite(LED_PIN, HIGH);
  }

  // every 0.5s speed control
  if (time2 < millis()) {
    time2 = millis() + 500;
    if (target) {
//      speedControl(); 
    }
  }

  while (Serial.available()){
    char in_char = Serial.read();
    if (int(in_char)!=-1){
      pin_str+=in_char;
    }
    if (in_char=='\n'){
      int pin_num = pin_str.toInt();
      switch (pin_num) {
        // enable 
        case 0: 
          enable = false; 
          speedStart = false; 
          target = false;
          digitalWrite(ENABLE_PIN, LOW);
          Serial.println("stop. ");
          break; 
        case 1: 
          if (digitalRead(safetyPin)) {
            enable = true; 
            speedStart = false; 
            digitalWrite(ENABLE_PIN, HIGH);
            Serial.println("running... ");
          } else {
            Serial.println("safety circuit open! ");
          }
          break; 
        case 2: 
          if (enable) {
            if (!acc) {
                dec = false;
                acc = true;
                Serial.println("faster... ");
            } else {
                dec = false;
                acc = false;
                Serial.println("stop acc. ");
            }
          }
          break; 
        case 3: 
          if (enable) {
            if (!dec) {
                acc = false;
                dec = true;
                Serial.println("slower... ");
            } else {
                dec = false;
                acc = false;
                Serial.println("stop dec. ");
            }
          }
          break; 
        case 4: 
          if (!up) {
            if (down) {
              digitalWrite(LOW_PIN, LOW);
              down = false;
              Serial.println("lower stop. ");
            } else {
              digitalWrite(LOW_PIN, HIGH);
              down = true;
              Serial.println("lower... ");
            }
          }
          break; 
        case 5: 
          if (!down) {
            if (up) {
              digitalWrite(HIGH_PIN, LOW);
              up = false;
              Serial.println("upper stop. ");
            } else {
              digitalWrite(HIGH_PIN, HIGH);
              up = true;
              Serial.println("upper... ");
            }
          }
          break;
        case 6: 
          if (enable) {
            if (target) {
              target = false;
              dec = false;
              acc = false;
              Serial.println("stop target speed. ");
            }
            else {
              target = true; 
              Serial.println("starting target pid controller. ");
            }
          }
          else
            Serial.println("start inverter first! ");
          break;
        case 10:
          frequency -= increment;
          CheckFrequency();
          break;
        case 11:
          frequency += increment;
          CheckFrequency();
          break;
        default:
          if (pin_num >=20 && pin_num <= 100) {
            speedtarget = float(pin_num) / 10 + 0.1; 
            Serial.print("target speed: ");
            Serial.println(speedtarget);
          }
          break; 
      }
      pin_str = "";
    } 
  }
}

void speedSensor() {
  long countTime = millis() - time0;
  time0 = millis();
  if (speedStart == false)
    speedStart = true; 
  else { 
    long speedmm = 241428 / countTime; 
    speedkm = (float)(speedmm) / 1000 * 3.6; 
    
    Serial.print("time running: ");
    Serial.print(countTime);
    Serial.print(" speed: ");
    Serial.print(speedmm);
    Serial.print(" mm/s ");
    Serial.print(speedkm);
    Serial.print(" km/h ");
    Serial.println(speedsoll);
  }
}


void heartSensor() {
  long countTimeH = millis() - heartTime;
  heartTime = millis();
  if (heartCount < heartMaxArray) {
     heartArray[heartCount] = 6000 / (countTimeH);
     heartCount++;
  }
  else {
    heartCount = 0;
  }
  
  Serial.print("time counting: ");
  Serial.print(countTimeH);
  Serial.print(" beat: ");
  Serial.println(60000 / countTimeH);
}

uint16_t heartBeat() {
  uint16_t heartBeatVal;
  for (int i = 0; i < heartMaxArray; i++) 
    heartBeatVal = heartBeatVal + heartArray[i];
  heartBeatVal = heartBeatVal / (10 * heartMaxArray); 
  return heartBeatVal;
}


float heightSensor() {
  int incr = analogRead(analogPin); 
  Serial.print("höhe: "); 
  Serial.print(incr);         
//  float incrDeg = 7.2 - ((float)(incr)-118) / (941-118) * 6.2;
  float incrRel = 12.63 - ((float)(incr)-HEIGHTMAX) / (HEIGHTMIN-HEIGHTMAX) * 10.88;  // relative %
  return incrRel;
}

/*
void speedControl() {
  float speedgap = abs(speedtarget - speedkm);  
if (speedgap < 5) {                             // if we're close to setpoint, use conservative tuning parameters
    myPID.SetTunings(consKp, consKi, consKd);
  } else {
    myPID.SetTunings(aggKp, aggKi, aggKd);
  }
  myPID.Compute();    
  // calculate the acceleration according to the speddifference
  frequency = MINFREQUENCY + ((MAXFREQUENCY - MINFREQUENCY)/10) * abs(speedsoll - speedtarget);
  CheckFrequency();
  Serial.print("PID out: ");
  Serial.print(speedsoll);
  Serial.print("  frequency: ");
  Serial.print(frequency);
  if (speedsoll > speedkm) {
    dec = false;
    acc = true;
//      Serial.println("   faster... ");
  }
  else {
    dec = true;
    acc = false;
//      Serial.println("   slower... ");
  }
}

*/
void CheckFrequency()
{
  if (frequency != oldFrequency) {        // change
    if (frequency > MAXFREQUENCY) {       // max reached?
      frequency = MAXFREQUENCY;
    }
    if (frequency < MINFREQUENCY) {       // min reached?
      frequency = MINFREQUENCY;
    }
    period = (CLOCKFREQUENCY / ( frequency * 16 * 128) ) -1;
    cli();
    OCR1A = period;
    sei();
    oldFrequency = frequency;  
  }
  Serial.print("frequency: ");
  Serial.println(frequency);
}


ISR(TIMER1_COMPA_vect)
{
  // timer for frequency
  if (acc)
    digitalWrite(FAST_PIN, !digitalRead(FAST_PIN));
  else
    digitalWrite(FAST_PIN, LOW);
  if (dec)
    digitalWrite(SLOW_PIN, !digitalRead(SLOW_PIN));
  else
    digitalWrite(SLOW_PIN, LOW);
}
