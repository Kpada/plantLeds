#include "stdAfx.h"
#include "leds.h"

#if defined LEDS_NUMBER && 4 == LEDS_NUMBER
    // gpio list
    static const uint32_t c_ledsPinList [] = {led0, led1, led2, led3};
#else
    #error
#endif // defined LEDS_NUMBER && 4 == LEDS_NUMBER
//--------------------------------------------------------------------------------------


/// assert
///
static void ledGpioPinAssert (uint32_t pin)
{
    if( DEAD_BEEF == pin ) {
        TRACE_MSG("pin value = %d is incorrect\n", pin);
        APP_ERROR_HANDLER(NRF_ERROR_NOT_SUPPORTED);
    }
}
//--------------------------------------------------------------------------------------

/// assert
///
static void ledGpioPtrAssert (void* ptr)
{
    if( NULL == ptr ) {
        TRACE_MSG("pointer is empty\n");
        APP_ERROR_HANDLER(NRF_ERROR_NOT_FOUND);
    }
}
//--------------------------------------------------------------------------------------

/// pin number get
///
__inline static uint32_t ledGpioGetPin (uint32_t idx)
{
    // get
    uint32_t pin = idx < LEDS_NUMBER ? c_ledsPinList[idx] : DEAD_BEEF;
    // assert
    ledGpioPinAssert(pin);
    // return
    return pin;
}
//--------------------------------------------------------------------------------------

/// reset
///
__inline static void ledStructReset (led_t* led)
{
    led->time.timeTurnOn = 0;
    led->time.timeTurnOff = 0;
    led->state = false;
}
//--------------------------------------------------------------------------------------

/// gpio config
///
__inline static void ledGpio_config (uint32_t idx) 
{
    nrf_gpio_cfg_output( ledGpioGetPin(idx) );    
}
//--------------------------------------------------------------------------------------

/// turn led on
///
__inline static void turnLedOn (led_t* led, uint32_t idx)
{
    nrf_gpio_pin_set( ledGpioGetPin(idx) );
    led->state = true;  
}
//--------------------------------------------------------------------------------------

/// turn led off
///
__inline static void turnLedOff (led_t* led, uint32_t idx)
{
    nrf_gpio_pin_clear( ledGpioGetPin(idx) );
    led->state = false;
}
//--------------------------------------------------------------------------------------

/// initializaion procedure
///
bool ledsInit (led_t* leds)
{  
    // check
    ledGpioPtrAssert(leds);
        
    // fill
    for( uint32_t idx = 0; idx < LEDS_NUMBER; idx++ ) {
        // get
        led_t* led = &leds[idx];
        // gpio
        ledGpio_config(idx);
        // turn led off
        turnLedOff(led, idx);
        // reset
        ledStructReset(led);
    }
    return true;
}
//--------------------------------------------------------------------------------------

/// we have 2 time_t values, check them and return true in case they are correct
///
__inline static bool checkBordersAndValuesForTime_t (time_t timeTurnOn, time_t timeTurnOff)
{
    // default condition
    bool condition1 = timeTurnOn  < TIME_T_MAX &&
        timeTurnOff < TIME_T_MAX &&
        timeTurnOn  < timeTurnOff;  
    
    // extra one. we will use it if we gonna turn the led off immediately
    bool condition2 = 0 == timeTurnOn && 0 == timeTurnOff;
    
    return condition1 || condition2;
}
//--------------------------------------------------------------------------------------

/// check a value
///
bool ledsValueCheck (led_t* led, uint32_t ledIdx)
{
    return  led &&                      // is led?
            ledIdx < LEDS_NUMBER &&     // is its number correct?
            checkBordersAndValuesForTime_t(led->time.timeTurnOn, led->time.timeTurnOff);
}
//--------------------------------------------------------------------------------------

/// routine
///
void ledsRoutine (led_t* leds, time_t currentTime)
{ 
    // check
    ledGpioPtrAssert(leds); 
          
    // fill
    int idx;
    for( idx = 0; idx < LEDS_NUMBER; idx++ ) {
        // get
        led_t* led = &leds[idx];
        // check
        if( led->time.timeTurnOn == led->time.timeTurnOff ) {
            // turn led off and then do nothing
            turnLedOff(led, idx);
        }
        else {
#ifdef DEBUG 
            bool prevState = led->state;
#endif // DEBUG      
            
            if( currentTime >= led->time.timeTurnOn && currentTime < led->time.timeTurnOff )
                turnLedOn(led, idx);
            else
                turnLedOff(led, idx);
            
#ifdef DEBUG           
            if( prevState != led->state ) {
                TRACE_MSG("led %d state updated, new state = %d\n", idx, (led->state ? 1 : 0) );
            }
#endif // DEBUG
        }
    } 
}
//--------------------------------------------------------------------------------------
