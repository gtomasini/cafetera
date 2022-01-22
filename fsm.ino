//fsm implementatation
#include "fsm.h"

//Pulse parameters for each kind of coffee
//                leche       cafe      choc
//cafe(1):       (0, 0), (20, 100),   (0, 0)  
//cortado(2):    (5,20),  (18, 80),   (0, 0)
//cafeleche(3):  (12,50), (14, 50),   (0, 0)
//capuccino(4):  (10,40), (12, 30), (18, 30)
//chocolate(5):  (0, 0),    (0, 0),(60. 100)

CoffeePlParms coffeePars[]={
  CoffeePlParms (std::make_pair (0, 0), std::make_pair (20, 100), std::make_pair (0, 0)),//0. cafe
  CoffeePlParms (std::make_pair (5, 20), std::make_pair (18, 80), std::make_pair (0, 0)),//1. cortado
  CoffeePlParms (std::make_pair (10, 10), std::make_pair (14, 50), std::make_pair (0, 0)), //2. cafemilk
  CoffeePlParms (std::make_pair (10, 40), std::make_pair (12, 30), std::make_pair (18, 30)),//3. capu
  CoffeePlParms (std::make_pair (0, 0), std::make_pair (0, 0), std::make_pair (60, 100)) //4. choco
};

volatile uint8_t CoffeeMakerFSM::plConter;

void CoffeeMakerFSM::ISRCountPulse(){
    static unsigned char _plConterTotal=0;
    if (++_plConterTotal%2==0)  ++plConter;
}

bool CoffeePlParms::checkPars(){
    //check if the parameters are ok (preconditions, h2o_xxx>xxx)
    if (milk.second < milk.first){
      Serial.println ("milk-h2o pulses conf error!!!");  
      return false;
    }
    if (coffee.second < coffee.first){
      Serial.println ("coffee-h2o pulses conf error!!!");  
      return false;
    }
    if (choc.second >= choc.first){
      Serial.println ("choco-h2o pulses conf error!!!");  
      return false;
    }
    if  (!(milk.first || coffee.first || choc.first)){
      Serial.println ("not milk, not coffee neither choc pulses conf error!!!");  
      return false;
    }
    return true;
}

void CoffeePlParms::printConf(){
    Serial.println ("Coffee configuration (milk,coffee, choc):");
    Serial.print (milk.first);  
    Serial.print (", ");
    Serial.print (milk.second);
    Serial.print (", ");
    Serial.print (coffee.first);
    Serial.print (", ");
    Serial.print (coffee.second);
    Serial.print (", ");
    Serial.print (choc.first);
    Serial.print (", ");
    Serial.println (choc.second);
}

int CoffeeMakerFSM::prepareCoffee (){
    Serial.println(__PRETTY_FUNCTION__);  
    if (parms.checkPars() == false) return -1;
        
    parms.printConf();
    COFFEE_STs state = IDLE_ST;//first state
    turn_off_all_relays ();
    digitalWrite (PUMP_OUT, LOW); //TURN ON PUMP
    unsigned long last_inc_msecs = millis();//to timeout
    plConter = 0;  //reset pulses counter!!!!!
    uint8_t last_pulse_conter=0;

    for(;;){
      //timeout treatment
      if ((millis()-last_inc_msecs) > PULSES_TIMEOUT_MSECS) {
        Serial.print("ERROR in state ");  
        Serial.print(state);  
        Serial.println (", no input pulses, TIMEOUT!!!");
        state = NO_WATER_ST;//direct to error state
        last_pulse_conter = plConter;
        plConter = 0;
      }
      else if (plConter != last_pulse_conter){
          last_inc_msecs = millis();//reeset timeout because some pulse arrived
          last_pulse_conter = plConter;
      }
          
      switch (state){

        case IDLE_ST://start state
          Serial.print ("IDLE_ST ");
          if (plConter==0) break;//if not pulses do nothing
          Serial.print (plConter);
          if (parms.milk.first>0){
            state = MILK_ST;  //next state!!!
            turn_milk(LOW);   //turn on milk and h2o
            Serial.println (" pulses, IDLE_ST to MILK_ST transition!");
          }
          else if (parms.coffee.first>0){
            state = COFFEE_ST;//next state!!!
            turn_coffee(LOW); //turn on coffee and h2o
            Serial.println (" pulses, IDLE_ST to COFFEE_ST transition!");
          }
          else if (parms.choc.first>0){
              state = CHOC_ST;
              turn_choc(LOW); //turn on choc and h2o
              Serial.println (" pulses, IDLE_ST to CHOC_ST transition!");          
          }
          else{
              state = ERROR_ST;
              Serial.println (" pulses, IDLE_ST to ERROR_ST transition!");          
          }
          break;

        case MILK_ST:
          if (plConter >= parms.milk.second){
            turn_milk(HIGH);  //turn off milk and h2o
            Serial.print (plConter);
            if (parms.coffee.first>0){
              state = COFFEE_ST;//next state
              turn_coffee(LOW); //turn on coffee and h2o
              Serial.println (" pulses, MILK_ST to COFFEE_ST transition!");
            }
            else if (parms.choc.first>0){
              state = CHOC_ST;//next state
              turn_choc(LOW); //turn on coffee and h2o
              Serial.println (" pulses, MILK_ST to CHOC_ST transition!");
            }
            else{
              state = END_ST;
              Serial.println (" pulses, MILK_ST to END_ST transition!");
            }
            plConter = 0;
          }
          else if (plConter >= parms.milk.first){
            digitalWrite (PROD_MILK_OUT, HIGH);//TURN OFF MILK
            Serial.print (plConter);
            Serial.println (" pulses (milk-thr reached)");
          }
          break;

        case COFFEE_ST:
          if (plConter >= parms.coffee.second){
            turn_coffee(HIGH);  //turn off coffee and h2o
            Serial.print (plConter);
            if (parms.choc.first>0){
              state = CHOC_ST;
              turn_choc(LOW); //turn on choc and h2o
              Serial.println (" pulses, COFFEE_ST to CHOC_ST transition!");
            }
            else{
              state = END_ST; 
              Serial.println (" pulses, COFFEE_ST to END_ST transition!");
            }
            plConter = 0;
          }
          else if (plConter >= parms.coffee.first){
            digitalWrite (PROD_COFFEE_OUT, HIGH);//turn off cafe prod
            //Serial.print (pulseConter);
            //Serial.println (" pulses, (coffee-thr reached)");
          }
          break;

        case CHOC_ST:
          if (plConter >= parms.choc.second){
            state = END_ST;
            plConter = 0;
            turn_choc(HIGH);
            Serial.print (plConter);
            Serial.println (" pulses, NO_CHOC_ST to END_ST transition");
          }
          else if (plConter >= parms.choc.first){
            digitalWrite (PROD_CHOC_OUT, HIGH); //turn off choc
            Serial.print (plConter);
            Serial.println (" pulses, (choc-thr reached)");
          }
          break;

        case END_ST://fin del servicio!
          Serial.print (plConter);
          Serial.println (" pulses, CAFE READY, please TAKE IT!!!");
          turn_off_all_relays ();
          plConter = 0;
          services_num++; //incrementa servicios!!!!
          return 0;//retorna!!!!!
        
        case NO_WATER_ST:
        case ERROR_ST:
        default://se llega a este estado por timeout
          turn_off_all_relays ();
          Serial.print (plConter);
          Serial.println (" pulses, ERROR/NO_WATER state!");
          delay (3000);
          plConter = 0;
          return -1;//retorna con error!!!
      }
    }
    Serial.println ("ERROR, it must no pass here!");
    return 0;//no deberia salir por aqui
}

int CoffeeMakerFSM::justCoffee (){
  parms = coffeePars[static_cast<int>(CoffeeType::cafe)];
  Serial.println ("*** calling for a cafe ***");
  return prepareCoffee ();  
}

int CoffeeMakerFSM::cortado (){
   parms = coffeePars[static_cast<int>(CoffeeType::cortado)];
   Serial.println ("*** calling for a cortado ***");
   return prepareCoffee ();
}

int CoffeeMakerFSM::capuccino(){
   parms = coffeePars[static_cast<int>(CoffeeType::capu)];
   Serial.println ("*** calling for a capuccino ***");
   return prepareCoffee ();
}
