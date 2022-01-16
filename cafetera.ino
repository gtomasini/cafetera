//cafetera esp32 
//ver 0.60
//author: pablo tomasini
//tester/itegrator: marcelo spoturno 

#include <EEPROM.h>
#define __ASSERT_USE_STDERR
#include <assert.h>
#include "fsm.h"

#define EEPROM_SIZE 5

const int BUTTON_1_IN = 12;
const int PULSE_IN = 13;

const int PROD_COFFEE_OUT    = 15;//red
const int PROD_MILK_OUT      = 16;//white
const int PROD_CHOC_OUT      = 19;//blue
const int PUMP_OUT           = 2;//green
const int H2O_COFFEE_VLV_OUT = 4;//yelow
const int H2O_MILK_VLV_OUT   = 5;//orange
const int H2O_CHOC_VLV_OUT   = 18;//brown

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    abort();    // abort program execution.
}

//auxiliar functions to turn on or off, LOW is ON!
inline void turn_milk(bool level){
  digitalWrite (PROD_MILK_OUT, level);    //TURN ON/off MILK
  digitalWrite (H2O_MILK_VLV_OUT, level); //TURN ON/off H2O-MILK
}

inline void turn_coffee (bool level){
  digitalWrite (PROD_COFFEE_OUT, level);  //turn on/off Coffee
  digitalWrite (H2O_COFFEE_VLV_OUT, level);//turn on/off h2o-coffee
}

inline void turn_choc (bool level){
  digitalWrite (PROD_CHOC_OUT, LOW);      //turn on/off choc
  digitalWrite (H2O_CHOC_VLV_OUT, LOW);   //turn on/off h2-o choc
}

void turn_off_all_relays (){
  Serial.println("turning off all outputs (high level)...");
  digitalWrite (PROD_COFFEE_OUT, HIGH);
  digitalWrite (PROD_MILK_OUT, HIGH);
  digitalWrite (PROD_CHOC_OUT, HIGH);
  digitalWrite (PUMP_OUT, HIGH);
  digitalWrite (H2O_COFFEE_VLV_OUT, HIGH);
  digitalWrite (H2O_MILK_VLV_OUT, HIGH);
  digitalWrite (H2O_CHOC_VLV_OUT, HIGH);
} 

unsigned CoffeeMakerFSM::services_num;//it must be read from eeprom               


void setup () {
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for native USB port only

  Serial.println("setup...");

  //enable IRQ for pulses
  attachInterrupt (digitalPinToInterrupt(0), CoffeeMakerFSM::ISRCountPulse, RISING);

  pinMode (PROD_COFFEE_OUT, OUTPUT);
  pinMode (PROD_MILK_OUT, OUTPUT);
  pinMode (PROD_CHOC_OUT, OUTPUT);
  pinMode (PUMP_OUT, OUTPUT);
  pinMode (H2O_COFFEE_VLV_OUT, OUTPUT);
  pinMode (H2O_MILK_VLV_OUT, OUTPUT);
  pinMode (H2O_CHOC_VLV_OUT, OUTPUT);
  pinMode (PULSE_IN, INPUT);
  pinMode (BUTTON_1_IN, INPUT);
  
  delay (1000);
  turn_off_all_relays ();

  EEPROM.begin (EEPROM_SIZE);// initialize EEPROM with predefined size
  //this just first time
  //EEPROM.write(0, 0);
  //EEPROM.commit();
  //delay (5000);
  CoffeeMakerFSM::services_num = EEPROM.read(0);
  Serial.print ("*** services num: ");
  Serial.println (CoffeeMakerFSM::services_num);
}

void loop() {
  turn_off_all_relays ();//apago todo!!!!!!

  CoffeeMakerFSM cafetera;
  //Serial.print("loop, services_num: ");
  //Serial.println (services_num);
  //Serial.printl (", ");
  //delay(10*1000);
  if (digitalRead (BUTTON_1_IN) == LOW)
    cafetera.justCoffee ();

  //  capuccino ();  

  //delay(10*1000);
  //delay(30*1000);
}
