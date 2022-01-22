//cafetera esp32 
//ver 0.70
//author: pablo tomasini
//tester/itegrator: marcelo spoturno 

#include <EEPROM.h>
#include "fsm.h"

void  writeConf2EEPROM(){
  EEPROM.begin (EEPROM_SIZE);// initialize EEPROM with predefined size
  Serial.println (__PRETTY_FUNCTION__);
  int eeAddress = 0;   //offset we want the data to be put.
  for (int i=0; i< static_cast<int>(CoffeeType::end); ++i){
      EEPROM.put (eeAddress, coffeePars[i]);
      eeAddress += sizeof (coffeePars[0]);
  }
  EEPROM.put (eeAddress, CoffeeMakerFSM::services_num);
  delay(100);
}

void readConfFromEEPROM (){
  Serial.println ( "__PRETTY_FUNCTION__" );
  int eeAddress = 0; //EEPROM offset to start reading from
  for (int i=0; i < static_cast<int>(CoffeeType::end); ++i){
      EEPROM.get (eeAddress, coffeePars[i]);
      eeAddress += sizeof (coffeePars[0]);
      coffeePars[i].printConf();
  }
  EEPROM.get (eeAddress, CoffeeMakerFSM::services_num);
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

  Serial.println("__PRETTY_FUNCTION__");

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

  delay (1000);
  
  //writeConf2EEPROM();//just the first time or after reconfiguration
  delay (1000);
  readConfFromEEPROM();
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
  if (digitalRead (BUTTON_1_IN) == LOW)   cafetera.justCoffee ();

  //delay(10*1000);
}
