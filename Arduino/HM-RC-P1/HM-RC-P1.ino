//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-07-26 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

/*Defnies to reduce code size

The following defines can be used to reduce the size of the code. All defines has to set in the sketch before any header file is included.

    SENSOR_ONLY - save some byte by exclude code for actor devices
    NORTC - removes code for RTC support
    NOCRC - removes CRC calculation during startup - The EEPROM will no longer initialized when channel configuration is changed. A extra RESET is needed to initialize it again.
    SIMPLE_CC1101_INIT - simple CC1101 initialization - No error reporting in case of errors.
    NDEBUG - no serial (debug) output; no hardware serial initialized/used at all
*/
/*
#define SENSOR_ONLY
#define NORTC
#define NOCRC
#define SIMPLE_CC1101_INIT
#define NODEBUG
*/

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <Remote.h>

// Arduino pin for the config button
// PB0 == PIN 8
#define CONFIG_BUTTON_PIN 8
//#define CONFIG_BUTTON_PIN 11 //fuer AskSin auf mysensors batterypowered by Get Sanders

// Arduino pins for the buttons
// A0 == PIN 14
#define BTN1_PIN 14


// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x00,0x1a,0x00},       // Device ID
    "HMRC001A00",           // Device Serial
    {0x00,0x1a},            // Device Model
    0x11,                   // Firmware Version
    as::DeviceType::Remote, // Device Type
    {0x00,0x00}             // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef DualStatusLed<5,4> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> HalType;
class Hal : public HalType {
  // extra clock to count button press events
  AlarmClock btncounter;
public:
  void init (const HMID& id) {
    HalType::init(id);
    // get new battery value after 50 key press
    battery.init(50,btncounter);
    battery.low(22);
    battery.critical(19);
  }

  void sendPeer () {
    --btncounter;
  }

  bool runready () {
    return HalType::runready() || btncounter.runready();
  }
};

typedef RemoteChannel<Hal,PEERS_PER_CHANNEL,List0> ChannelType;
typedef MultiChannelDevice<Hal,ChannelType,1> RemoteType;

Hal hal;
RemoteType sdev(devinfo,0x20);
ConfigButton<RemoteType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  remoteISR(sdev,1,BTN1_PIN);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  sdev.initDone();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    hal.activity.savePower<Sleep<>>(hal);   
  }
}
