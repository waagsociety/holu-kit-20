
  // KEYS for the TTN app
  
  #include <lmic.h>
  #include <hal/hal.h>
  #include <SPI.h>

  #define SECRET_DEVICE 001 // insert ID
  
  static const u1_t PROGMEM SECRET_APPEUI[8]={ };  //the same
  
  #if SECRET_DEVICE == 001 
  //device hl_device_001
    static const u1_t PROGMEM SECRET_DEVEUI[8]= {  };
    static const u1_t PROGMEM SECRET_APPKEY[16] = {  };  
  #elif SECRET_DEVICE == 002
   //device hl_device_
    static const u1_t PROGMEM SECRET_DEVEUI[8]= {  };
    static const u1_t PROGMEM SECRET_APPKEY[16] = {  };  
  #endif
  

