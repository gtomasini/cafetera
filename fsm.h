//FSM
#ifndef FSM_H
#define FSM_H
#include <utility> 

//pulse parameters
struct CoffeePlParms {
  CoffeePlParms(){
    milk = std::make_pair(0,0);
    coffee = std::make_pair(0,0);
    choc = std::make_pair(0,0);
  }

  void check();
  void print();
  //first parameter is product pulses, second parameter is h2o pulses
  std::pair<uint8_t, uint8_t> milk;
  std::pair<uint8_t, uint8_t> coffee;
  std::pair<uint8_t, uint8_t> choc;
};

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
