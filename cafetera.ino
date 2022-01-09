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

struct pulseConter{
  static void ISRCountPulse(){
    static unsigned char _total_conter=0;
      if (++_total_conter%2==0)  
        ++_conter;
      _pulse_arrived = true;
  }

  static unsigned char getPulses(){
      return _conter;
  }

  static bool getPulseArrived(){
    bool st=_pulse_arrived;
    _pulse_arrived = false;
    return st;
  }
  
private:
  static volatile unsigned char _conter;
  static bool _pulse_arrived;
};
 
void init(){
  //enable IRQ for input pulses from sensor
  attachInterrupt (digitalPinToInterrupt(0), pulseConter::ISRCountPulse, RISING);

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
  //services_num = EEPROM.read(0);
  //Serial.print ("*** services num: ");
  //Serial.println (services_num);
}



void setup() {
  Serial.begin(115200);
  Serial.println("setup...");

  init();
}
