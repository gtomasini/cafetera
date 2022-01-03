//cafetera esp32 
//ver 0.50
//author: pablo tomasini
//tester: marcelo spoturno

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

class coffee_maker{
  
  typedef enum {
    READY_ST,
    COFFEE_ST,  
    MILK_ST,
    CHOC_ST,
    END_ST,
    NO_WATER_ST,
    ERROR_ST
  } COFFEE_STS;

  const unsigned long PULSES_TIMEOUT_MSECS = 1000*10;//10 secs

  unsigned char milk, h2o_milk, 
                coffee, h2o_coffee,
                choc, h2o_choc;
  static volatile unsigned char pulseConter; 
  static unsigned services_num;//it must be read from eprom               

  static void ISRCountPulse(){
    static unsigned char _pulse_conter=0;
    if (++_pulse_conter%2==0)  ++pulseConter;
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

public:
  static void init(){
    //enable IRQ for pulses
    attachInterrupt (digitalPinToInterrupt(PULSE_IN),  coffee_maker::ISRCountPulse, RISING);

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
    turn_off_all_relays ();

    EEPROM.begin (EEPROM_SIZE);// initialize EEPROM with predefined size
    //this just first time
    //EEPROM.write(0, 0);
    //EEPROM.commit();
    //delay (5000);
    services_num = EEPROM.read(0);
    Serial.print ("*** services num: ");
    Serial.println (services_num);
  }

  coffee_maker():milk(0), h2o_milk(0), 
                coffee(0), h2o_coffee(0),
                choc(0), h2o_choc(0){};

  int preparing_coffee (){
    //check if the parameters are ok (preconditions, h2o_xxx>xxx)
    assert (h2o_milk >= milk);
    assert (h2o_coffee >= coffee);
    assert (h2o_choc >= choc);
    assert (milk || coffee || choc);
    
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
    unsigned char _last_pulse_conter=0;

    for(;;){
      //timeout treatment
      if ((millis()-last_inc_msecs) > PULSES_TIMEOUT_MSECS) {
        Serial.print("ERROR in state ");  
        Serial.print(state);  
        Serial.println (", no input pulses, TIMEOUT!!!");
        state = NO_WATER_ST;//direct to error state
        _last_pulse_conter = pulseConter;
        pulseConter = 0;
      }
      else if (pulseConter != _last_pulse_conter){
          last_inc_msecs = millis();//reeset timeout because some pulse arrived
          _last_pulse_conter = pulseConter;
      }
          
      switch (state){

        case READY_ST://start state
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
            turn_milk(HIGH);  //turn off milk and h2o
            Serial.print (pulseConter);
            if (coffee>0){
              state = COFFEE_ST;//next state
              turn_coffee(LOW); //turn on coffee and h2o
              Serial.println (" pulses, MILK_ST to COFFEE_ST transition!");
            }
            else if (choc>0){
              state = CHOC_ST;//next state
              turn_choc(LOW); //turn on coffee and h2o
              Serial.println (" pulses, MILK_ST to CHOC_ST transition!");
            }
            else{
              state = END_ST;
              Serial.println (" pulses, MILK_ST to END_ST transition!");
            }
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
            Serial.print (pulseConter);
            if (choc>0){
              state = CHOC_ST;
              turn_choc(LOW); //turn on choc and h2o
              Serial.println (" pulses, COFFEE_ST to CHOC_ST transition!");
            }
            else{
              state = END_ST; 
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

  static void turn_off_all_relays (){
    Serial.println("turning off all outputs (high level)...");
    digitalWrite (PROD_COFFEE_OUT, HIGH);
    digitalWrite (PROD_MILK_OUT, HIGH);
    digitalWrite (PROD_CHOC_OUT, HIGH);
    digitalWrite (PUMP_OUT, HIGH);
    digitalWrite (H2O_COFFEE_VLV_OUT, HIGH);
    digitalWrite (H2O_MILK_VLV_OUT, HIGH);
    digitalWrite (H2O_CHOC_VLV_OUT, HIGH);
  } 

  int just_coffee (){
    milk = 0; h2o_milk = 0;
    coffee = 20; h2o_coffee = 100;
    choc = 0, h2o_choc = 0;
    Serial.println ("*** calling for a cafe ***");
    return preparing_coffee ();  
  }

  int cortado (){
    milk = 18; h2o_milk = 30;
    coffee = 6; h2o_coffee = 20;
    choc = 0, h2o_choc = 0;

    Serial.println ("*** calling for a cortado ***");
    return preparing_coffee ();
  }

  int capuccino(){
    milk = 12; h2o_milk = 30;
    coffee = 12; h2o_coffee = 30;
    choc = 10, h2o_choc = 30;

    Serial.println ("*** calling for a capuccino ***");
    return preparing_coffee ();
  }
};

volatile unsigned char coffee_maker::pulseConter;  
unsigned coffee_maker::services_num;//it must be read from eprom               

void setup() {
  Serial.begin(115200);
  Serial.println("setup...");
  


  coffee_maker::init();
}

void loop() {
  coffee_maker::turn_off_all_relays ();//apago todo!!!!!!

  coffee_maker cafetera;
  //Serial.print("loop, services_num: ");
  //Serial.println (services_num);
  //Serial.printl (", ");
  //delay(10*1000);
  if (digitalRead (BUTTON_1_IN) == LOW)
    cafetera.just_coffee ();

  //  capuccino ();  

  delay(10*1000);
  //delay(30*1000);
}
