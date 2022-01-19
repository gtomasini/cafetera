//FSM
#ifndef FSM_H
#define FSM_H
#include <utility> 

enum class CoffeeType {milk, cortado, cafe, capu, choc};

using coff_pair=std::pair<uint8_t, uint8_t>;

//pulse parameters
struct CoffeePlParms {
  CoffeePlParms(){
    milk = std::make_pair(0,0);
    coffee = std::make_pair(0,0);
    choc = std::make_pair(0,0);
  }

  CoffeePlParms(coff_pair mi, coff_pair co, coff_pair ch){
    milk = std::make_pair (mi.first, mi.second);
    coffee = std::make_pair (co.first, co.second);
    choc = std::make_pair (ch.first, ch.second);
  }

  void check();
  void print();
  //first parameter is product pulses, second parameter is h2o pulses
  coff_pair milk;
  coff_pair coffee;
  coff_pair choc;
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
  static unsigned services_num;//it must be read from eprom 
  CoffeeMakerFSM(){};
  static void ISRCountPulse();
    
  int prepareCoffee ();
    
  int justCoffee ();
  int cortado ();
  int capuccino();
};

#endif
