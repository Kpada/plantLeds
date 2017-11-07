#ifndef __LEDS_H__
#define __LEDS_H__
 

typedef struct {
    time_t      timeTurnOn;         
    time_t      timeTurnOff;    
} led_time_t;


#pragma pack(push, 1) 

// led
typedef struct {
    led_time_t  time;
    bool        state;          // is working?
} led_t;

#pragma pack(pop)

// initializaion
bool ledsInit (led_t* leds);

// check a value
bool ledsValueCheck (led_t* led, uint32_t ledIdx);

// routine
void ledsRoutine (led_t* leds, time_t currentTime);


#endif // __LEDS_H__
