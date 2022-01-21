//FSM
#ifndef FSM_H
#define FSM_H
#include <utility> 
#define __ASSERT_USE_STDERR
#include <assert.h>

enum class CoffeeType {cafe=0, cortado=1, cofmilk=2, capu=3, choc=4, end=5};
  
using coff_pair=std::pair<uint8_t, uint8_t>;

//pulse parameters
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
  void print();
  //first parameter is product pulses, second parameter is h2o pulses
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
