#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

// app version string
#define APP_VERSION_STR                 "1.2"           // software version string
// app name string
#define APP_NAME_STR                    "plantLeds"     // software name string
//--------------------------------------------------------------------------------------

// config 
#define USE_PASSWORD_PROTECTION         1               // you can disable this timer (if you gonna test the target)
//--------------------------------------------------------------------------------------

// number of leds
#define LEDS_NUMBER                     4               // number of leds
//--------------------------------------------------------------------------------------

#if 4 == LEDS_NUMBER
    // gpio
    typedef enum {
        led0    = 8,    
        led1    = 10,   
        led2    = 6,
        led3    = 12,
    } led_list; 
#endif
//--------------------------------------------------------------------------------------

// adc pin
#define PIN_ADC_PWR                     NRF_ADC_CONFIG_INPUT_3  // P0.02, the feature is now available
//--------------------------------------------------------------------------------------

// timers
#define APP_WDT_RELOAD_VALUE_MS         8000                    // watchdog timeout, msec
#define APP_WAIT_4_PASSWORD_MS          10000                   // password timeout, msec
//--------------------------------------------------------------------------------------

// specific
#define DEVICE_NAME                     "Leds"                  // advertizing name
//--------------------------------------------------------------------------------------
    
// low freq source external/internal rc
// PS: Don't judge me. I was using 2 different boards for debug and release
#ifdef DEBUG
    #define CLOCK_LFCL_SRC              NRF_CLOCK_LFCLKSRC_RC_250_PPM_1000MS_CALIBRATION    // an internal RC
#else
    #define CLOCK_LFCL_SRC              NRF_CLOCK_LFCLKSRC_XTAL_20_PPM                      // an external crystal
#endif
//--------------------------------------------------------------------------------------

// advertising period
#define BLE_ADVERTISING_PERIOD_MSEC     500                 // advertising period, msec
// connection intervals
// attention. be careful cuz slow device won't wokrs well
#define BLE_CONN_INTRVL_MIN_MSEC        100                 // min connevtion period, msec
#define BLE_CONN_INTRVL_MAX_MSEC        200                 // max connection period, msec
//--------------------------------------------------------------------------------------

// tx gain level
#define BLE_TX_GAIN_dB                  4                   // see sd_ble_gap_tx_power_set
//--------------------------------------------------------------------------------------


/// UIDs
///

// base
#define SVC_UUID_BASE                   {   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }
//--------------------------------------------------------------------------------------

// password and time
#define SVC_PW_UUID_SERVICE             ( 0x1000 )                      // base
#define SVC_PW_UUID_CHAR                ( SVC_PW_UUID_SERVICE  + 1 )    // password set
#define SVC_TIME_UUID_CHAR              ( SVC_PW_UUID_CHAR     + 1 )    // time set & get
//--------------------------------------------------------------------------------------

// leds                                    
#define SVC_LEDS_UUID_SERVICE           ( 0x2000 )                      // base
#define SVC_LEDS_UIID_CHAR_BASE         ( SVC_LEDS_UUID_SERVICE + 1 )   // the first uuid value
//--------------------------------------------------------------------------------------
  
                                 
#endif // __APP_CONFIG_H__
