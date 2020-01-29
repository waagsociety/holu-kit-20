/*******************************************************************************
 * Emma Pareschi - June 2019
 * version 2.1
 * 
 * The MIT License (MIT)
 * Copyright 2019 Emma Pareschi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, 
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * 
 *******************************************************************************/

#include <SdFat.h> //https://github.com/greiman/SdFat
#include <Wire.h> //include Wire.h library
#include "RTClib.h" //include Adafruit RTC library
#include <Wire.h> 
#include "hl_device_id.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> //https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.h
#include <lmic.h> //https://github.com/matthijskooijman/arduino-lmic
#include <hal/hal.h>
#include <SPI.h>
#include <Adafruit_ADS1015.h> //https://github.com/adafruit/Adafruit_ADS1X15
#include <Adafruit_SleepyDog.h> //https://github.com/adafruit/Adafruit_SleepyDog

#define DEBUG 1
#define version 2.1

boolean GAS_SENSORS = true;
boolean BME_SENSOR = false;
boolean data_flag = false;
boolean flag_joined = false;
boolean flag_sdcard = false;
boolean flag_sent = false;

int timer_wd = 4000;

  static const byte DEVICE = SECRET_DEVICE;
  void os_getArtEui (u1_t* buf) { memcpy_P(buf, SECRET_APPEUI, 8);}
  void os_getDevEui (u1_t* buf) { memcpy_P(buf, SECRET_DEVEUI, 8);}
  void os_getDevKey (u1_t* buf) {  memcpy_P(buf, SECRET_APPKEY, 16);}


byte deviceID = DEVICE;
static byte mydata[26];
static osjob_t sendjob;

const unsigned TX_INTERVAL = 176;   //every three minutes

// Pin mapping Feather M0 
const lmic_pinmap lmic_pins = { 
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {3, 15, LMIC_UNUSED_PIN},  
};

const int cs_lora = 8;

//=======================================================
// *** SDS011 ***
unsigned int Pm25 = 0;
unsigned int Pm10 = 0;
int  Status_sds = -1;
int LastError_sds = 0;
float p10 = 0.0, p25 = 0.0;

int error;
int count = 0;
int cnt = 0;
int ecnt = 0;
int ecnt_s = 0;

float s25 = 0.0;
float s10 = 0.0;

float pm25 = 0.0;
float pm10 = 0.0;


//---------------//---------------//----------------//
const byte sds_period = 180; //measuring period of sds in seconds
const byte start_measure = 25;


//=======================================================
// *** SD CARD *** // 
int chipSelect = 10; //chip select pin for the MicroSD Card Adapter
File file; // file object that is used to read and write data
String header;
char file_name[16]; // make it long enough to hold your longest file name, plus a null terminator
boolean sdcard_ok = false;
SdFat SD;

//=======================================================
// ***RTC*** //
RTC_PCF8523 rtc;

int timestamp[30];

//=======================================================
// *** BME280s *** //
Adafruit_BME280 bme;

     float tempIN = 0;
     float tempIN_tot = 0;
     float tempIN_avg = 0;

     float humIN = 0;
     float humIN_tot = 0;
     float humIN_avg = 0;
     float last_humIN_avg = 0;

int bme_counter = 0;
int counter = 0;


//=======================================================
// *** OUTPUTs *** //
#include <Adafruit_NeoPixel.h>

#define LEDpin 9

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, LEDpin, NEO_RGB + NEO_KHZ800);

uint32_t red = pixel.Color(0, 0, 0);
uint32_t green = pixel.Color(0, 0, 0);
uint32_t blue = pixel.Color(0, 0, 0);
uint32_t off = pixel.Color(0, 0, 0);
uint32_t on = pixel.Color(0,0 ,0);

//=======================================================
// *** ADC *** //

 Adafruit_ADS1115 ads_no2;  //no2
 Adafruit_ADS1115 ads_o3(0x4A); //o3

int o3_op1 = 0;
int o3_op2 = 0;
int no2_op1 = 0;
int no2_op2 = 0;

int o3_op1_tot = 0;
int o3_op2_tot = 0;
int no2_op1_tot = 0;
int no2_op2_tot = 0;

int16_t o3_op1_avg = 0;
int16_t o3_op2_avg = 0;
int16_t no2_op1_avg = 0;
int16_t no2_op2_avg = 0;

int8_t no2[4];

//=======================================================
// *** Function for LORA *** //
void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            flag_joined = false;
            blinks(1, 200, off, red);
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            flag_joined = true;
            blinks(1, 200, off, green);
            Watchdog.enable(timer_wd);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            //blinks(1, 200, off, red);
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            //blinks(1, 200, off, red);           
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if(flag_joined == true) {
                blinks(2, 300, off, green);
                pixel.setPixelColor(0, green);
                pixel.show();
                delay(1);
            }
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
  
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        mydata[0] = deviceID;
        mydata[1] = timestamp[0] >> 8;      //YY_1
        mydata[2] = timestamp[0] & 0xFF;    //YY_2
        mydata[3] = timestamp[1];           //MM
        mydata[4] = timestamp[2];           //DD   
        mydata[5] = timestamp[3];           //h   
        mydata[6] = timestamp[4];           //m
        mydata[7] = timestamp[5];           //s     

        int pm25_aux = (pm25 * 100);        
        mydata[8] = pm25_aux >> 8;           //pm25 1
        mydata[9] = pm25_aux & 0xFF;         //pm25 2

        int pm10_aux = (pm10 * 100);        
        mydata[10] = pm10_aux >> 8;           //pm25 1
        mydata[11] = pm10_aux & 0xFF;         //pm25 2

        int tempIN_aux = round((tempIN_avg+100) * 100);
        mydata[12] = tempIN_aux >> 8;            //tempIN 1
        mydata[13] = tempIN_aux  & 0xFF;         //tempIN 2
        
        int humIN_aux = (humIN_avg * 100);
        mydata[14] = humIN_aux >> 8;              //humIN 1
        mydata[15] = humIN_aux  & 0xFF;           //humIN 2

    if (GAS_SENSORS == true){
        mydata[16] = no2_op1_avg >> 8;
        mydata[17] = no2_op1_avg & 0xFF;
        mydata[18] = no2_op2_avg >> 8;
        mydata[19] = no2_op2_avg & 0xFF;

        mydata[20] = o3_op1_avg >> 8;
        mydata[21] = o3_op1_avg & 0xFF;
        mydata[22] = o3_op2_avg >> 8;
        mydata[23] = o3_op2_avg & 0xFF;
    }
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        
        Serial.println(F("Packet queued"));
    }
}

//=======================================================
// SETUP
void setup () {

  pixel.begin();
  delay(10);
  pixel.setBrightness(100);
  pixel.setPixelColor(0, on);
  pixel.show();
  delay(100);
  
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(5000);

  Serial.println(F("Starting"));
  Serial.println(F("HoLu kit v1.2"));
  Serial.print("code version: "); Serial.println(version);
  Serial.print("device ID: "); Serial.println(DEVICE);
  Serial.println("Connected sensors:");
  Serial.println("-> sds011: pm25, pm10");
  Serial.println("-> bme280: temp, hum");
  
  if(GAS_SENSORS == true){
      Serial.println("-> NO2 alphasense");
      Serial.println("-> O3 alphasense");
  }

      
  // ** INITIALIZE CARD ** //
  
  pinMode(cs_lora, OUTPUT);
  digitalWrite(cs_lora, HIGH);
  
  pinMode(chipSelect, OUTPUT); // chip select pin must be set to OUTPUT mode
  digitalWrite(chipSelect, HIGH);
  
  sdcard_ok = SD.begin(chipSelect);
  delay(1);
   
  if(sdcard_ok == false) { // Initialize SD card
    Serial.println("");  
    Serial.println("***CARD********************************************");
    Serial.println("Could not initialize SD card."); // if return value is false, something went wrong.
    blinks(2, 200, off, red);
    flag_sdcard = false;
  //  return;    
  } else {
    Serial.println("");  
    Serial.println("***CARD********************************************");
    Serial.println("card initialized.");   
    flag_sdcard = true;     //set flag for sd card as true
    blinks(2, 200, off, blue);
  }

  int n = 0;
  snprintf(file_name, sizeof(file_name), "data%03d.txt", n); // includes a three-digit sequence number in the file name

 // if (flag_sdcard == true){
  while(SD.exists(file_name)) {
    n++;
    snprintf(file_name, sizeof(file_name), "data%03d.txt", n);
  }
  Serial.print("file name: "); Serial.println(file_name);

  delay(1000);

  // ** Write Log File Header 
  file = SD.open(file_name, FILE_WRITE); // open "file.txt" to write data
  if (file)
  {
     //file.println(" "); //Just a leading blank line, in case there was previous data
    if(GAS_SENSORS == true){
      header = "DeviceID, Timestamp, PM2.5, PM10, Temp (C), Humidity (%), NO2 op1, NO2 op2, O3 op1, O3 op2";
    } else {
      header = "DeviceID, Timestamp, PM2.5, PM10, Temp (C), Humidity (%)";
    }
    file.println(header);
    file.close();
    Serial.print("header: ");
    Serial.println(header);
    Serial.println();
    blinks(2, 200, off, blue);    
  }
  else
  {
    Serial.println("Couldn't open log file");
    blinks(2, 200, off, red);
  }
 // }

 digitalWrite(cs_lora, LOW);

  // ** INITIALIZE RTC ** //
  
   if (! rtc.begin()) {
   Serial.println("Couldn't find RTC");
   while (1);
  } else {
   Serial.println("RTC ready: ");
  }

  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

        DateTime now = rtc.now();
        Serial.print(now.year()); Serial.print("/");
        Serial.print(now.month()); Serial.print("/");
        Serial.print(now.day()); Serial.print(" - ");

        Serial.print(now.hour()-1); Serial.print(":");
        Serial.print(now.minute()); Serial.print(":");
        Serial.println(now.second());
        //Serial.println("********************************************************");
        Serial.println();

   bool status;    
   status = bme.begin(); 
   delay(1000); 
   
  if (!status) { 
    Serial.println("Could not find the BME280 IN sensor, check wiring!"); 
    BME_SENSOR = false;
   } else {
    Serial.println("***BME280******************************************");
    Serial.println("bme ready. ");
    BME_SENSOR = true;
    delay(5);
    Serial.print("Temp: "); Serial.println(bme.readTemperature()); 
    Serial.print("Hum: "); Serial.println(bme.readHumidity());
    Serial.println();
   }

   ads_no2.begin();
   ads_o3.begin();
   delay(200);
   data_ads();
   delay(10);

    Serial.println("***ALPHA NO2******************************************");
    Serial.println("no2 sensor ready? ");   
    delay(5);
    Serial.print("no2a: "); Serial.println(no2_op1); 
    Serial.print("no2b: "); Serial.println(no2_op2);
    Serial.println();    

    Serial.println("***ALPHA O3******************************************");
    Serial.println("o3 sensor ready? ");   
    delay(5);
    Serial.print("o3a: "); Serial.println(o3_op1); 
    Serial.print("o3b: "); Serial.println(o3_op2);
    Serial.println();    

   

// ** LORA ** //
  #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
  #endif

  // LMIC init
    os_init();
    
  // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    
  // Start job
    do_send(&sendjob);


}


void loop() {
  
   Watchdog.reset();

   delay(1);
   os_runloop_once();
    
    ProcessSerialData();

    float  x6 = LastError_sds;

    if (p25 > -1.0 && p10 > -1.0) {
      Serial.print("PM2.5 = "); Serial.println(p25);
      Serial.print("PM10 = "); Serial.println(p10);
       cnt += 1;
       ecnt =0;
       ecnt_s =0;
       data_flag = true;
    }
    else {
      x6 = (float) LastError_sds;
      //Serial.print("\tERROR= ");
      //Serial.print(LastError_sds);
      //Serial.print("\t ");
      ecnt++;

      data_flag = false;
    }

  // set to zero millisecond count
  if (ecnt > 999){
    ecnt = 0;
   Watchdog.reset();
    ecnt_s++;

  Serial.print("\tECOUNT SEC= "); Serial.println(ecnt_s); 
       
    // measure temp/hum
    if ( ecnt_s >= start_measure && ecnt_s <= sds_period){
        bme_counter++;
             
    if(BME_SENSOR == true){
     data_bme();
     tempIN_tot += tempIN;
     humIN_tot += humIN;
     } else {
       tempIN_tot = 0;
       humIN_tot = 0;     
     }

  if(GAS_SENSORS == true){
     data_ads();
      o3_op1_tot += o3_op1;
      o3_op2_tot += o3_op2;
      no2_op1_tot += no2_op1;
      no2_op2_tot += no2_op2;   }
    }      
  }
  
  if (ecnt_s > sds_period){
    ecnt_s = 0;

    tempIN_avg = 0;
    tempIN_tot = 0;
    
    humIN_avg = 0;
    humIN_tot = 0;

    no2_op1_avg = 0;
    no2_op2_avg = 0;
    o3_op1_avg = 0;
    o3_op2_avg = 0;
  }

  // if the data from sds are valid:  
  // - make the avarage of the temp/hum values
  // - save the data on sdcard
  
  if (data_flag == true){ 

        pm25 = p25;
        pm10 = p10;   
               
        data_time();         //get the timestamp
        
        delay(1);
        
        tempIN_avg = tempIN_tot/bme_counter;
        humIN_avg = humIN_tot/bme_counter;

        //FILTER TEMP
        int temp_aux = (tempIN_avg * 100);
        int temp_aux_1 = temp_aux >> 8;            //tempIN 1
        int temp_aux_2 = temp_aux  & 0xFF;         //tempIN 2
        
        if (temp_aux_1 > 254 && temp_aux_2 > 254){  
          data_bme();
          tempIN_avg = tempIN;
          humIN_avg = humIN;        
        }

        //FILTER HUM - under test
        if(humIN_avg > 100 || humIN_avg < 0){
            humIN_avg = last_humIN_avg;
        } else {
            last_humIN_avg = humIN_avg;
        }

        
     if(GAS_SENSORS == true){
        o3_op1_avg = o3_op1_tot/bme_counter;
        o3_op2_avg = o3_op2_tot/bme_counter;
        no2_op1_avg = no2_op1_tot/bme_counter;
        no2_op2_avg = no2_op2_tot/bme_counter;
     }

        Serial.print("Day: "); 
        for (int i = 0 ; i < 3; i++){
          Serial.print(timestamp[i]);Serial.print(" ");
        } Serial.println("");
        Serial.print("Time: ");
        for (int i = 3 ; i < 6; i++){
          Serial.print(timestamp[i]);Serial.print(" ");
        } Serial.println(""); 
        Serial.print("TempIN_avg: "); Serial.print(tempIN_avg); Serial.print("\t TempIN_tot: "); Serial.println(tempIN_tot); 
        Serial.print("HumIN_avg: "); Serial.print(humIN_avg); Serial.print("\t HumIN_tot: "); Serial.println(humIN_tot); 
      if(GAS_SENSORS == true){        
        Serial.print("o3_op1_avg: "); Serial.print(o3_op1_avg); Serial.print("\t o3_op1_tot: "); Serial.println(o3_op1_tot);
        Serial.print("o3_op2_avg: "); Serial.print(o3_op2_avg); Serial.print("\t o3_op2_tot: "); Serial.println(o3_op2_tot);  
        Serial.print("no2_op1_avg: "); Serial.print(no2_op1_avg); Serial.print("\t no2_op1_tot: "); Serial.println(no2_op1_tot);
        Serial.print("no2_op2_avg: "); Serial.print(no2_op2_avg); Serial.print("\t no2_op2_tot: "); Serial.println(no2_op2_tot);   
      } 
        Serial.print("bme counter: "); Serial.println(bme_counter);
        bme_counter = 0;
        tempIN_tot = 0;
        humIN_tot = 0;
        o3_op1_tot = 0;
        o3_op2_tot = 0;
        no2_op1_tot = 0;
        no2_op2_tot = 0;
             
        delay(10);

        if (flag_sdcard = true){
          data_log();
          delay(1);
        }       
        data_flag = false;
  
      } else {
        data_flag = false;
      }
      

}


// FUNCTION for SDS
void ProcessSerialData()
{

  uint8_t mData = 0;
  uint8_t i = 0;
  uint8_t mPkt[10] = {0};
  uint8_t mCheck = 0;

  Status_sds = -1;  // Default: no data availabe ...
  p10 = -1.0;
  p25 = -1.0;

  while (Serial1.available() > 0)
  {
    // from www.inovafitness.com
    // packet format: AA C0 PM25_Low PM25_High PM10_Low PM10_High 0 0 CRC AB

    Status_sds = 0; // There is data!

    mData = Serial1.read();
    delay(10);//wait until packet is received

    if (mData == 0xAA) // Head1 ok?
    {
      mPkt[0] =  mData;
      mData = Serial1.read();

      if (mData == 0xc0) // Head2 ok?
      {
        mPkt[1] =  mData;
        mCheck = 0;
        for (i = 0; i < 6; i++) // Get the content of the package!
        {
          mPkt[i + 2] = Serial1.read();
          delay(2);
          mCheck += mPkt[i + 2];
        }

        mPkt[8] = Serial1.read();
        delay(5);

        mPkt[9] = Serial1.read();
        if (mPkt[9] != 0xAB) {
          Serial.print("\nmData[9] != 0xAB,  but ");
          Serial.println(mPkt[9]);
          Status_sds = 4; LastError_sds = Status_sds; ecnt++;
        }

        if (mCheck == mPkt[8]) // Check CRC
        {
          Serial1.flush();
          //Serial.write(mPkt,10);

          Pm25 = (uint16_t)mPkt[2] | (uint16_t)(mPkt[3] << 8);
          Pm10 = (uint16_t)mPkt[4] | (uint16_t)(mPkt[5] << 8);

          if (Pm25 > 9999)
            Pm25 = 9999;
          if (Pm10 > 9999)
            Pm10 = 9999;

          p25 = Pm25 / 10.0;
          p10 = Pm10 / 10.0;

          while (Serial1.available() > 0) {  // Zap buffer ...
            mData = Serial1.read();
          }

          return;  // We now have one good packet :-))

        } // CRC ??
        else {
          Serial.print("\nCRC != OK,  ");
          Serial.print(mCheck);
          Serial.print("  !=  ");
          Serial.println(mPkt[8]);
          Status_sds = 3; LastError_sds = Status_sds; ecnt++;
        }

      } // Head2 ??
      else {
        Serial.print("\nmData != 0xc0,  but ");
        Serial.println(mData);
        Status_sds = 2; LastError_sds = Status_sds; ecnt++;
      }

    } // Head1 ??
    else {
      Serial.print("\nmData != 0xAA,  but ");
      Serial.println(mData);
      Status_sds = 1; LastError_sds = Status_sds; ecnt++;
    }
  }
}

void data_bme(){ 
  // ** get temp/hum
//  tempIN = -22.90;
//  humIN = 5.55;   

  tempIN = bme.readTemperature();
  humIN = bme.readHumidity();   
  delay(1);   
}


void data_ads(){ 
  
   o3_op1 = ads_o3.readADC_Differential_0_1();
   delay(1);
   o3_op2 = ads_o3.readADC_Differential_2_3();
   delay(1);
  
   no2_op1 = ads_no2.readADC_Differential_0_1();
   delay(1);
   no2_op2 = ads_no2.readADC_Differential_2_3();
   delay(1);
}

void data_time(){

  // ** get the timestamp
        DateTime now = rtc.now();
        timestamp[0] = now.year();
        timestamp[1] = now.month();
        timestamp[2] = now.day();

        timestamp[3] = now.hour()-1;
        timestamp[4] = now.minute();
        timestamp[5] = now.second();

        if(timestamp[3] == -2) timestamp[3] = 22;
        if(timestamp[3] == -1) timestamp[3] = 23;
}


//function to blink the led
void blinks(byte NUMBLINKS, int blink_delay, uint32_t color1, uint32_t color2){
      for (int i = 0; i<NUMBLINKS+1; i++){
      pixel.setPixelColor(0, color1);
      pixel.show();
      delay(blink_delay);
      pixel.setPixelColor(0, color2);
      pixel.show();
      delay(blink_delay);
      }
}

void data_log(){

  digitalWrite(cs_lora, HIGH);
         file = SD.open(file_name, FILE_WRITE); // open "file.txt" to write data
        if (file) {
        file.print(deviceID); file.print(",");
        file.print(timestamp[0]); file.print(":");
        file.print(timestamp[1]); file.print(":");
        file.print(timestamp[2]); file.print("/");
        file.print(timestamp[3]); file.print(":");
        file.print(timestamp[4]); file.print(":");
        file.print(timestamp[5]); file.print(",");
        file.print(p25); file.print(",");
        file.print(p10); file.print(","); 
        file.print(tempIN_avg); file.print(","); 
      if(GAS_SENSORS == false){
        file.println(humIN_avg);}
      if(GAS_SENSORS == true){
        file.print(humIN_avg); file.print(",");
        file.print(no2_op1_avg); file.print(","); 
        file.print(no2_op2_avg); file.print(","); 
        file.print(o3_op1_avg); file.print(","); 
        file.println(o3_op2_avg); }
        file.close(); // close file
        blinks(1, 200, off, blue);

       if(flag_joined == true){
          pixel.setPixelColor(0, green);
          pixel.show();
          }
        if(flag_joined == false) {
          pixel.setPixelColor(0, red);
          pixel.show();
          }
        
        } else {
          Serial.println("Could not open file (writing).");
        blinks(1, 200, off, red);
        
        if(flag_joined == true){
          pixel.setPixelColor(0, green);
          pixel.show();
        }
        if(flag_joined == false) {
          pixel.setPixelColor(0, red);
          pixel.show();
        }
        
         }
       delay(1);

       digitalWrite(cs_lora, LOW);
}

 

