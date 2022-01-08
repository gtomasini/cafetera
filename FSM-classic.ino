#include <memory>

// just_coffee 
//  coffee = 20; h2o_coffee = 100;

// cortado
//  milk = 18; h2o_milk = 30;
//  coffee = 6; h2o_coffee = 20;

// capuccino
// milk = 12; h2o_milk = 30;
//    coffee = 12; h2o_coffee = 30;
//    choc = 10, h2o_choc = 30;

extern void turn_off_all_relays ();
volatile unsigned char pulseConter;//global!

const unsigned long PULSES_TIMEOUT_MSECS = 1000*10;//10 secs
//FSM Classic Design pattern implementation

/* ---------------------------------  Events ------------------------------------------ */
enum event { pulse, timeout };

//State Interface
struct State {
  State():_milk(0), _h2o_milk(0), 
          _coffee(0), _h2o_coffee(0),
          _choc(0), _h2o_choc(0), _pulseConter(0), _fin(false){};
          
  virtual std::unique_ptr<State> on_event(event e) = 0;

  bool getFin(){
    return _fin;  
  }
  
  void setMilk (unsigned char milk, unsigned char h2o){
    _milk=milk; _h2o_milk=h2o;
  }
  void setCoffee (unsigned char coffee, unsigned char h2o){
    _coffee=coffee; _h2o_milk=h2o;
  }

  void setChoc (unsigned char choc, unsigned char h2o){
    _choc=choc; _h2o_milk=h2o;
  }

protected:
  inline void turn_milk (bool level){
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

  unsigned char _pulseConter; 
  unsigned char _milk, _h2o_milk, _coffee, _h2o_coffee, _choc, _h2o_choc;
  bool _fin;
};

//first state
struct Idle_St : State {
  Idle_St(){
    turn_off_all_relays ();
    digitalWrite (PUMP_OUT, LOW);    //TURN ON PUMP

    _pulseConter = 0;  //reset pulses counter!!!!!
    Serial.print ("Idle ST... ");
    Serial.print (_milk);
    Serial.print (", ");
    Serial.print (_h2o_milk);
    Serial.print (", ");
    Serial.print (_coffee);
    Serial.print (", ");
    Serial.print (_h2o_coffee);
    Serial.print (", ");
    Serial.print (_choc);
    Serial.print (", ");
    Serial.print (_h2o_choc);
    Serial.println ("");
  }
  std::unique_ptr<State> on_event(event e);
};

struct Coffee_St : State {
  Coffee_St(){
     Serial.print ("Coffee ST... ");
     Serial.print (_milk);
     Serial.print (", ");
     Serial.print (_h2o_milk);
     Serial.print (", ");
     Serial.print (_coffee);
     Serial.print (", ");
     Serial.print (_h2o_coffee);
     Serial.print (", ");
     Serial.print (_choc);
     Serial.print (", ");
     Serial.print (_h2o_choc);
     Serial.println ("");
  }
  std::unique_ptr<State> on_event(event e);
};
    
struct Milk_St : State {
  Milk_St(){
     Serial.print ("Milk ST... ");
  }  

  std::unique_ptr<State> on_event(event e);
};

struct Choc_St : State {
  Choc_St(){
     Serial.print ("Choc ST... ");
  }

  std::unique_ptr<State> on_event(event e);
};

struct End_St : State {
  End_St(){
     Serial.print ("End ST... ");
  }

  std::unique_ptr<State> on_event(event e);
};
    
struct Error_St : State {
  Error_St(){
     Serial.print ("Error ST... ");
  }

  std::unique_ptr<State> on_event(event e);
};

std::unique_ptr<State> Idle_St::on_event(event e) {
  if (event::timeout){
      Serial.println (" pulses, timeout, transition to ERROR_ST.");   
      return std::make_unique<Error_St>();     
  }
  _pulseConter++;
  if (_milk>0){
    turn_milk (LOW);   //turn on milk and h2o
    Serial.print (_pulseConter);
    Serial.println (" pulses, transition to MILK_ST!");
    return std::make_unique<Milk_St>();
  }
  else if (_coffee>0){
    turn_coffee(LOW); //turn on coffee and h2o
    Serial.print (_pulseConter);
    Serial.println (" pulses, transition to COFFEE_ST!");
    return std::make_unique<Coffee_St>();
  }
  else if (_choc>0){
    turn_choc(LOW); //turn on choc and h2o
    Serial.print (_pulseConter);
    Serial.println (" pulses, transition to CHOC_ST!");  
    return std::make_unique<Choc_St>();       
  }
  Serial.println (" pulses, on event pulse");
  return nullptr;
}

std::unique_ptr<State> Milk_St::on_event(event e){
  if (event::timeout){
    Serial.println (" pulses, timeout, transition MILK_ST to ERROR_ST.");   
    return std::make_unique<Error_St>();     
  }
  _pulseConter++;
  if (_pulseConter >= _h2o_milk){
    turn_milk(HIGH);  //turn off milk and h2o
    Serial.print (_pulseConter);
    if (_coffee>0){
      turn_coffee(LOW); //turn on coffee and h2o
      Serial.println (" pulses, MILK_ST to COFFEE_ST transition!");
      return std::make_unique<Coffee_St>();
    }
    else if (_choc>0){
      turn_choc(LOW); //turn on coffee and h2o
      Serial.println (" pulses, MILK_ST to CHOC_ST transition!");
      _pulseConter = 0;
      return std::make_unique<Choc_St>();
    }
    else{
      Serial.println (" pulses, MILK_ST to END_ST transition!");
      return std::make_unique<End_St>();
    }
  }
  else if (_pulseConter >= _milk){
    digitalWrite (PROD_MILK_OUT, HIGH);//TURN OFF MILK
    Serial.print (_pulseConter);
    Serial.println (" pulses (milk-thr reached)");
   }
   return nullptr;
}

std::unique_ptr<State> Coffee_St::on_event(event e){
  if (event::timeout){
    Serial.println (" pulses, timeout, transition Coffee_ST to ERROR_ST.");   
    return std::make_unique<Error_St>();     
  }
  if (_pulseConter >= _h2o_coffee){
    turn_coffee(HIGH);  //turn off coffee and h2o
    Serial.print (_pulseConter);
    if (_choc>0){
      turn_choc(LOW); //turn on choc and h2o
      Serial.println (" pulses, COFFEE_ST to CHOC_ST transition!");
      _pulseConter = 0;
      return std::make_unique<Choc_St>();
    }
    else{
      Serial.println (" pulses, COFFEE_ST to END_ST transition!");
      return std::make_unique<End_St>();
    }
  }
  else if (_pulseConter >= _coffee){
    digitalWrite (PROD_COFFEE_OUT, HIGH);//turn off cafe prod
    //Serial.print (pulseConter);
    //Serial.println (" pulses, (coffee-thr reached)");
  }
  return nullptr;
}

std::unique_ptr<State> Choc_St::on_event(event e){
  if (event::timeout){
    Serial.println (" pulses, timeout, transition Choc_ST to ERROR_ST.");   
    return std::make_unique<Error_St>();     
  }
  if (_pulseConter >= _h2o_choc){
    turn_choc(HIGH);
    Serial.print (_pulseConter);
    Serial.println (" pulses, CHOC_ST to END_ST transition");
    return std::make_unique<End_St>();
  }
  else if (_pulseConter >= _choc){
    digitalWrite (PROD_CHOC_OUT, HIGH); //turn off choc
    Serial.print (_pulseConter);
    Serial.println (" pulses, (choc-thr reached)");
  }  
  return nullptr;
}

std::unique_ptr<State> End_St::on_event(event e){
  Serial.print (_pulseConter);
  Serial.println (" pulses, CAFE READY, please TAKE IT!!!");
  turn_off_all_relays ();
  _pulseConter = 0;
  _fin = true;
  return nullptr;
}

std::unique_ptr<State> Error_St::on_event(event e){
  turn_off_all_relays ();
  Serial.print (_pulseConter);
  Serial.println (" pulses, ERROR/NO_WATER state!");
  delay (3000);
  _pulseConter = 0;
  _fin = true;
  return nullptr;
}

struct doCoffee {
   std::unique_ptr<State> m_curr_state;
   bool _fin;

   doCoffee():_fin(false){
      m_curr_state = std::make_unique<Idle_St>();
      m_curr_state->setCoffee(20, 100);
   }
   
   void dispatch(event e) {
     auto new_state = m_curr_state->on_event(e);
     if (new_state)
        m_curr_state = move(new_state);

     _fin = m_curr_state->getFin();
   }

   void doit(){
     unsigned char last_pulse_conter=pulseConter;
     unsigned long last_inc_msecs = millis();//to timeout
     
     for(;;){
        //timeout treatment
        Serial.println(pulseConter);
        if ((millis()-last_inc_msecs) > PULSES_TIMEOUT_MSECS) {
          Serial.println ("  pulses, TIMEOUT!!!");
          last_pulse_conter = pulseConter;
          pulseConter = 0;
          dispatch (event::timeout);
        }
        else if (pulseConter != last_pulse_conter){
           last_inc_msecs = millis();//reeset timeout because some pulse arrived
           last_pulse_conter = pulseConter;
           Serial.println ("  pulses arrived!");
           dispatch (event::pulse);
        }    
        if (_fin) return;
     }
   }
};

void loop() {
  doCoffee cafetera;
  //Serial.print("loop, services_num: ");
  //Serial.println (services_num);
  //Serial.printl (", ");
  delay(10*1000);
  //if (digitalRead (BUTTON_1_IN) == LOW)
    cafetera.doit();
  //  capuccino ();  

  delay(10*1000);
  //delay(30*1000);
}
