//FSM
#ifndef FSM_H
#define FSM_H
#include <utility> 
#define __ASSERT_USE_STDERR
#include <assert.h>

#define EEPROM_SIZE 32

#define   BUTTON_1_IN   12
#define   PULSE_IN      13

#define   PROD_COFFEE_OUT     15//red
#define   PROD_MILK_OUT       16//white
#define   PROD_CHOC_OUT       19//blue
#define   PUMP_OUT            2//green
#define   H2O_COFFEE_VLV_OUT  4//yelow
#define   H2O_MILK_VLV_OUT    5//orange
#define   H2O_CHOC_VLV_OUT    18//brown

enum class CoffeeType {cafe=0, cortado=1, cofmilk=2, capu=3, choc=4, end=5};
  
using coff_pair=std::pair<uint8_t, uint8_t>;//first parameter is product pulses, second parameter is h2o pulses

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

//pulse Configuration
struct CoffeePlParms {
  CoffeePlParms(){
    milk   = std::make_pair(0,0);
    coffee = std::make_pair(0,0);
    choc   = std::make_pair(0,0);
  }

  CoffeePlParms(coff_pair mi, coff_pair cof, coff_pair choc){
    milk   = std::make_pair (mi.first/2, mi.second/2);
    coffee = std::make_pair (cof.first/2, cof.second/2);
    choc   = std::make_pair (choc.first/2, choc.second/2);
    checkPars();
  }

  bool checkPars();
  void printConf();//print configuration
  
  coff_pair milk, coffee, choc;
};

extern CoffeePlParms coffeePars[];

class CoffeeMakerFSM {
  
  typedef enum {
    IDLE_ST,
    COFFEE_ST,  
    MILK_ST,
    CHOC_ST,
    END_ST,
    NO_WATER_ST,
    ERROR_ST
  } COFFEE_STs;

  const unsigned long PULSES_TIMEOUT_MSECS = 1000*10;//10 secs
  
  CoffeePlParms parms;
  static volatile unsigned char plConter; 

public:
  static unsigned services_num;//it must be read from eeprom (sporturno's dixit)
  CoffeeMakerFSM(){};
  static void ISRCountPulse();
    
  int prepareCoffee ();
    
  int justCoffee ();
  int cortado ();
  int capuccino();
};

// handle diagnostic informations given by assertion and abort program execution:
inline void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    abort();    // abort program execution.
}

#endif
