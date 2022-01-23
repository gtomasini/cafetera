//cafetera esp32 
//ver 0.70
//author: pablo tomasini
//tester/itegrator: marcelo spoturno 

#include <EEPROM.h>
#include "fsm.h"

#define EEPROM_SIZE 32

//Pulse parameters for each kind of coffee
//                leche       cafe      choc
//cafe(1):       (0, 0), (20, 100),   (0, 0)  
//cortado(2):    (5,20),  (18, 80),   (0, 0)
//cafeleche(3):  (12,50), (14, 50),   (0, 0)
//capuccino(4):  (10,40), (12, 30), (18, 30)
//chocolate(5):  (0, 0),    (0, 0),(60. 100)

//these configuration will be read from eeprom, so it will be overwrite
CoffeePlParms coffeePars[]={
  CoffeePlParms (std::make_pair (0, 0), std::make_pair (20, 100), std::make_pair (0, 0)),//0. cafe
  CoffeePlParms (std::make_pair (5, 20), std::make_pair (18, 80), std::make_pair (0, 0)),//1. cortado
  CoffeePlParms (std::make_pair (10, 10), std::make_pair (14, 50), std::make_pair (0, 0)), //2. cafemilk
  CoffeePlParms (std::make_pair (10, 40), std::make_pair (12, 30), std::make_pair (18, 30)),//3. capu
  CoffeePlParms (std::make_pair (0, 0), std::make_pair (0, 0), std::make_pair (60, 100)) //4. choco
};

void  writeConf2EEPROM(){
  Serial.println (__PRETTY_FUNCTION__);
  // initialize EEPROM with predefined size
  if (!EEPROM.begin (EEPROM_SIZE)){
      Serial.println("Failed to initialise EEPROM ERROR!!!!!");
      Serial.println("Restarting...");
      delay(1000);
      ESP.restart();
  }
  int eeAddress = 0;   //offset we want the data to be put.
  for (int i=0; i< static_cast<int>(CoffeeType::end); ++i){
      EEPROM.put (eeAddress, coffeePars[i]);
      coffeePars[i].printConf();
      eeAddress += sizeof (coffeePars[0]);
  }
  EEPROM.put (eeAddress, CoffeeMakerFSM::services_num);
  EEPROM.end();
  delay(100);
}

void readConfFromEEPROM (){
  Serial.println (__PRETTY_FUNCTION__);
  if (!EEPROM.begin (EEPROM_SIZE)){
      Serial.println("Failed to initialise EEPROM ERROR!!!!!");
      Serial.println("Restarting...");
      delay(1000);
      ESP.restart();
  }
  int eeAddress = 0; //EEPROM offset to start reading from
  for (int i=0; i < static_cast<int>(CoffeeType::end); ++i){
      EEPROM.get (eeAddress, coffeePars[i]);
      eeAddress += sizeof (coffeePars[0]);
      coffeePars[i].printConf();
  }
  EEPROM.get (eeAddress, CoffeeMakerFSM::services_num);
}

void turn_off_all_relays (){
  Serial.println (__PRETTY_FUNCTION__);
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

  Serial.println(__PRETTY_FUNCTION__);

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
  Serial.println("pulse algun boton para cafe...");
}

void loop() {
  //turn_off_all_relays ();//apago todo!!!!!!
  static String serialBuffer;
  String cmd="";
  if (Serial.available()) {
    char c=Serial.read();
    if (c=='\n' || c=='\r' || c==';'){
        cmd=serialBuffer;
        serialBuffer="";
        Serial.print("***** cmd received: ");
        Serial.println(cmd);
        //TODO: parse command here
    }
    else serialBuffer.concat(c);
  }

  CoffeeMakerFSM cafetera;
  //Serial.print("loop, services_num: ");
  //Serial.println (services_num);
  //Serial.printl (", ");
  if (digitalRead (BUTTON_1_IN) == LOW || cmd.equals("cafe") )   cafetera.justCoffee ();
  delay(1*1000);
}
