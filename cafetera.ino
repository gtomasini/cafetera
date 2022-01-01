//cafetera esp32 
//ver 0.3
#include <EEPROM.h>
#define __ASSERT_USE_STDERR
#include <assert.h>

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

const unsigned long PULSES_TIMEOUT_MSECS = 1000*10;//10 secs
volatile int pulseConter;
int services_num;//it must be read from eprom

typedef enum {
  READY_ST,
  COFFEE_ST,  
  MILK_ST,
  CHOC_ST,
  END_ST,
  NO_WATER_ST,
  ERROR_ST
} COFFEE_STS;

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    // abort program execution.
    abort();
}

void turn_off_all_relays (){
  digitalWrite (PROD_COFFEE_OUT, HIGH);
  digitalWrite (PROD_MILK_OUT, HIGH);
  digitalWrite (PROD_CHOC_OUT, HIGH);
  digitalWrite (PUMP_OUT, HIGH);
  digitalWrite (H2O_COFFEE_VLV_OUT, HIGH);
  digitalWrite (H2O_MILK_VLV_OUT, HIGH);
  digitalWrite (H2O_CHOC_VLV_OUT, HIGH);
} 

void setup() {
  Serial.begin(115200);
  Serial.println("setup...");
  pinMode (PROD_COFFEE_OUT, OUTPUT);
  pinMode (PROD_MILK_OUT, OUTPUT);
  pinMode (PROD_CHOC_OUT, OUTPUT);
  pinMode (PUMP_OUT, OUTPUT);
  pinMode (H2O_COFFEE_VLV_OUT, OUTPUT);
  pinMode (H2O_MILK_VLV_OUT, OUTPUT);
  pinMode (H2O_CHOC_VLV_OUT, OUTPUT);
  pinMode (PULSE_IN, INPUT);
  pinMode (BUTTON_1_IN, INPUT);
  
  delay(1000);
  Serial.println ("apago la punetera bomba y todas las salidas!!!!!!");
  turn_off_all_relays ();
  EEPROM.begin (EEPROM_SIZE);// initialize EEPROM with predefined size
  //this just first time
  //EEPROM.write(0, 0);
  //EEPROM.commit();
  //delay (5000);
  services_num = EEPROM.read(0);
  Serial.print ("*** services num: ");
  Serial.println (services_num);

  //enable IRQ for pulses
  attachInterrupt (digitalPinToInterrupt(PULSE_IN), ISRCountPulse, RISING);
}

void ISRCountPulse(){
  pulseConter++;
}

inline void turn_milk(bool level){
  digitalWrite (PROD_MILK_OUT, level);    //TURN ON MILK
  digitalWrite (H2O_MILK_VLV_OUT, level); //TURN ON H2O-MILK
}

inline void turn_coffee(bool level){
  digitalWrite (PROD_COFFEE_OUT, level);  //turn on Coffee
  digitalWrite (H2O_COFFEE_VLV_OUT, level);//turn on h2o-coffee
}

inline void turn_choc(bool level){
  digitalWrite (PROD_CHOC_OUT, LOW);      //turn on choc
  digitalWrite (H2O_CHOC_VLV_OUT, LOW);   //turn on h2-o choc
}

//the core function!!!! no es un carrito de rulemanes!!!
int preparing_coffee (int milk, int h2o_milk, int coffee, int h2o_coffee, int choc, int h2o_choc){
  //check if the parameters are ok (preconditions, h2o_xxx>xxx)
  assert (h2o_milk>=milk);
  assert (h2o_coffee>=coffee);
  assert (h2o_choc>=choc);
  
  COFFEE_STS state = READY_ST;//first state
  turn_off_all_relays ();
  digitalWrite (PUMP_OUT, LOW);    //TURN ON PUMP
  unsigned long last_inc_msecs = millis();//to timeout
  Serial.print ("preparing_cofee (");
  Serial.print (milk);
  Serial.print (", ");
  Serial.print (h2o_milk);
  Serial.print (", ");
  Serial.print (coffee);
  Serial.print (", ");
  Serial.print (h2o_coffee);
  Serial.print (", ");
  Serial.print (choc);
  Serial.print (", ");
  Serial.print (h2o_choc);
  Serial.println (")");
  pulseConter = 0;        //reset pulses counter!!!!!
  int _last_pulse_conter=0;

  for(;;){//state machine, infinite loop!!!
    //timeout treatment
    if ((millis()-last_inc_msecs) > PULSES_TIMEOUT_MSECS) {
       Serial.print("ERROR in state ");  
       Serial.print(state);  
       Serial.println (", no pulses, TIMEOUT!");
       state = NO_WATER_ST;//direct to error state
      _last_pulse_conter = pulseConter;
       pulseConter = -1;
    }
    else if (pulseConter != _last_pulse_conter){
        last_inc_msecs = millis();//reeset timeout because some pulse arrived
        _last_pulse_conter = pulseConter;
    }
        
    switch (state){

      case READY_ST:
        Serial.println ("READY_ST state!");
        if (milk>0){
          state = MILK_ST;  //next state!!!
          turn_milk(LOW);   //turn on milk and h2o
          Serial.print (pulseConter);
          Serial.println (" pulses, READY_ST to MILK_ST transition!");
        }
        else if (coffee>0){
          state = COFFEE_ST;//next state!!!
          turn_coffee(LOW); //turn on coffee and h2o
          Serial.print (pulseConter);
          Serial.println (" pulses, READY_ST to COFFEE_ST transition!");
        }
        else if (choc>0){
            state = CHOC_ST;
            turn_choc(LOW); //turn on choc and h2o
            Serial.print (pulseConter);
            Serial.println (" pulses, READY_ST to CHOC_ST transition!");          
        }
        else{
            state = ERROR_ST;
            Serial.print (pulseConter);
            Serial.println (" pulses, READY_ST to ERROR_ST transition!");          
        }
        break;

      case MILK_ST:
        if (pulseConter >= h2o_milk){
          state = COFFEE_ST;//next state
          turn_milk(HIGH);  //turn off milk and h2o
          turn_coffee(LOW); //turn on coffee and h2o
          Serial.print (pulseConter);
          Serial.println (" pulses, MILK_ST to COFFEE_ST transition!");
          pulseConter = 0;
        }
        else if (pulseConter >= milk){
          digitalWrite (PROD_MILK_OUT, HIGH);//TURN OFF MILK
          Serial.print (pulseConter);
          Serial.println (" pulses (milk-thr reached)");
        }
        break;

      case COFFEE_ST:
        if (pulseConter >= h2o_coffee){
          turn_coffee(HIGH);  //turn off coffee and h2o
          if (choc>0){
            state = CHOC_ST;
            turn_choc(LOW); //turn on choc and h2o
            Serial.print (pulseConter);
            Serial.println (" pulses, COFFEE_ST to CHOC_ST transition!");
          }
          else{
            state = END_ST; 
            Serial.print (pulseConter);
            Serial.println (" pulses, COFFEE_ST to END_ST transition!");
          }
          pulseConter = 0;
        }
        else if (pulseConter >= coffee){
          digitalWrite (PROD_COFFEE_OUT, HIGH);//turn off cafe prod
          //Serial.print (pulseConter);
          //Serial.println (" pulses, (coffee-thr reached)");
        }
        break;

      case CHOC_ST:
        if (pulseConter >= h2o_choc){
          state = END_ST;
          pulseConter = 0;
          turn_choc(HIGH);
          Serial.print (pulseConter);
          Serial.println (" pulses, NO_CHOC_ST to END_ST transition");
        }
        else if (pulseConter >= choc){
          digitalWrite (PROD_CHOC_OUT, HIGH); //turn off choc
          Serial.print (pulseConter);
          Serial.println (" pulses, (choc-thr reached)");
        }
        break;

      case END_ST://fin del servicio!
        Serial.print (pulseConter);
        Serial.println (" pulses, CAFE READY, please TAKE IT!!!");
        turn_off_all_relays ();
        pulseConter = 0;
        services_num++; //incrementa servicios!!!!
        return 0;//retorna!!!!!
      
      case NO_WATER_ST:
      case ERROR_ST:
      default://se llega a este estado por timeout
        turn_off_all_relays ();
        Serial.print (pulseConter);
        Serial.println (" pulses, ERROR/NO_WATER state!");
        delay (3000);
        pulseConter = 0;
        return -1;//retorna con error!!!
    }
  }
  Serial.println ("ERROR, it must no pass here!");
  return 0;//no deberia salir por aqui
}

int just_coffee (){
  Serial.println ("*** calling for un cafe ***");
  return preparing_coffee (0, 0, 20, 100, 0, 0);  
}

int cortado (){
  Serial.println ("*** calling for un cortado ***");
  return preparing_coffee (18, 30, 6, 20, 0, 0);
}

int capuccino(){
  Serial.println ("*** calling for a capuccino ***");
  return preparing_coffee (12,30,12,30,10,30);
}

void loop() {
  turn_off_all_relays ();//para que Marcelo no me rompa más
  //Serial.print("loop, services_num: ");
  //Serial.println (services_num);
  //Serial.printl (", ");
  //delay(10*1000);
  if (digitalRead (BUTTON_1_IN) == LOW)
    just_coffee ();

  //  capuccino ();  

  //delay(10*1000);
  //delay(30*1000);
}
