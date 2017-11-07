#ifndef __MAIN_H__
#define __MAIN_H__


/// app communications
///
typedef struct 
{   
    // ble    
    dm_application_instance_t   m_hBleApp;              // dm 
    ble_svc_pw_tm_t             m_ble_svc_pw_tm;        // password and time service
    ble_svc_leds_t              m_ble_svc_leds;
    uint16_t                    m_hBleConn;             // connection
    
    // utils
    bool                        m_isConnected;          // has device connected?
    bool                        m_isPasswordCorrect;    // is password correct?
} appCommType;


/// app state
///
typedef struct 
{  
    time_t          m_appTime;
    led_t           m_leds [LEDS_NUMBER];
    appCommType     m_comm;         // communications   
} appStateType;

// app state
extern appStateType g_appState;

// events
void appOnBleEvtHandle (ble_evt_t* evt);


void appIsObtainedPasswordCorrect (bool isCorrect);
void appNewTimeObtained (time_t time);
void appNewLedStateObtained (led_t* led, uint32_t ledIdx);


#endif // __MAIN_H__
