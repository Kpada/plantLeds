#include "stdAfx.h"
#include "main.h"

#include "ble/bleServices.h"


/* 
 *      Plant Leds Project for NRF51. 
 *
 *      Softdevice version: s130_nrf51_2.0.0-7.alpha
 *      Please mnote that I used the RTE.
 *
 *
 *      This is a BLE Time Delay Relay for 4 independed channels. 
 *  
 *
 *      Many thanks to Chizhov Alexander for his help with the Android app.
 *
 *
 *      Chizhov Nikolai
 *      2016
 */


// timers
#define APP_TIMER_PRESCALER             0              
#define APP_TIMER_OP_QUEUE_SIZE         4 
//--------------------------------------------------------------------------------------


// application state
appStateType                            g_appState;
//--------------------------------------------------------------------------------------

// wdt
static nrf_drv_wdt_channel_id           s_wdt;
//--------------------------------------------------------------------------------------

// wait for the password
APP_TIMER_DEF( tmrPassword );
#define TMR_PASSWORD_INTERVAL           APP_TIMER_TICKS(APP_WAIT_4_PASSWORD_MS, APP_TIMER_PRESCALER)
//--------------------------------------------------------------------------------------

// rtc
APP_TIMER_DEF( tmrAppTime );
#define TMR_APP_TIME_INTERVAL           APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)
//--------------------------------------------------------------------------------------


/// application state initialization
///
static void appStateInit (void)
{
    memset( &g_appState, 0, sizeof(g_appState) );       
    g_appState.m_comm.m_hBleConn = BLE_CONN_HANDLE_INVALID;    
}
//--------------------------------------------------------------------------------------

/// password
///
static bool passwordCheckAndHandle (void)
{
    if( g_appState.m_comm.m_isPasswordCorrect ) {
        // do nothing
    }       
    else {
        // disconnect
        if( g_appState.m_comm.m_isConnected ) {
            uint32_t err_code = sd_ble_gap_disconnect(g_appState.m_comm.m_hBleConn, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
        }
        TRACE_MSG("password incorrect\n");
    }    
    
    return g_appState.m_comm.m_isPasswordCorrect;
}
//--------------------------------------------------------------------------------------

/// password handler
///
void appIsObtainedPasswordCorrect (bool isCorrect)
{
    g_appState.m_comm.m_isPasswordCorrect = isCorrect;
    passwordCheckAndHandle();
}
//--------------------------------------------------------------------------------------

/// make an event
///
void tmrHandlerPw (void * p_context)
{
    UNUSED_PARAMETER(p_context);  
    
    passwordCheckAndHandle();
    TRACE_MSG("timer has been stopped\n");
}
//--------------------------------------------------------------------------------------

/// make an event
///
void tmrHandlerAppTime (void * p_context)
{
    UNUSED_PARAMETER(p_context);
    
    g_appState.m_appTime = g_appState.m_appTime < TIME_T_MAX ? g_appState.m_appTime + 1 : 0;       
    TRACE_MSG("time = %d\n", g_appState.m_appTime);
}
//--------------------------------------------------------------------------------------

/// start the timer
///
static void passwordTimerStart (void)
{
#if defined(USE_PASSWORD_PROTECTION) && 1 == USE_PASSWORD_PROTECTION
    
    TRACE_MSG("password timer has been started\n");
    
    // stop the timer (just in case)
    app_timer_stop(tmrPassword);
    
    // and start it
    uint32_t errCode = app_timer_start(tmrPassword, TMR_PASSWORD_INTERVAL, NULL);
    APP_ERROR_CHECK(errCode); 
    
#endif     
}
//--------------------------------------------------------------------------------------

/// Initialize timer module, making it use the scheduler
///
static void appTimersInit (void)
{
    APP_TIMER_INIT( APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false ); 
    
    uint32_t errCode;
       
    // password
    errCode = app_timer_create(
        &tmrPassword,
        APP_TIMER_MODE_SINGLE_SHOT,
        tmrHandlerPw
    );
    APP_ERROR_CHECK(errCode);
    
    // time
    errCode = app_timer_create(
        &tmrAppTime,
        APP_TIMER_MODE_REPEATED,
        tmrHandlerAppTime
    );
    APP_ERROR_CHECK(errCode);
    
    // start
    errCode = app_timer_start(tmrAppTime, TMR_APP_TIME_INTERVAL, NULL);
    APP_ERROR_CHECK(errCode); 
}   
//--------------------------------------------------------------------------------------

/// on BLE event handler
///
void appOnBleEvtHandle (ble_evt_t* evt)
{
    TRACE_MSG("onBleStackEvt: %d\n", evt->header.evt_id );
    
    switch( evt->header.evt_id ) {
        // connected
        case BLE_GAP_EVT_CONNECTED:
            TRACE_MSG("Connected\n\n");
            g_appState.m_comm.m_hBleConn = evt->evt.gap_evt.conn_handle;
            g_appState.m_comm.m_isConnected = true;                 
            passwordTimerStart();
            break;
        
        // disconnected
        case BLE_GAP_EVT_DISCONNECTED:
            TRACE_MSG("Disconnected\n\n");
            g_appState.m_comm.m_hBleConn = BLE_CONN_HANDLE_INVALID;
            g_appState.m_comm.m_isConnected = false;
            g_appState.m_comm.m_isPasswordCorrect = false;
            bleAdvertisingStart();
            break;
        
        // default
        default:
            break;
    }
}
//--------------------------------------------------------------------------------------

/// watchdog init
///
static void wdtInit (void)
{
    // init
    nrf_drv_wdt_config_t config = { 
        .behaviour          = NRF_WDT_BEHAVIOUR_RUN_SLEEP,    
        .reload_value       = APP_WDT_RELOAD_VALUE_MS, 
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH, 
    };
    
    uint32_t err_code = nrf_drv_wdt_init(&config, NULL);
    APP_ERROR_CHECK(err_code);
    
    // alloc
    err_code = nrf_drv_wdt_channel_alloc(&s_wdt);
    APP_ERROR_CHECK(err_code);
    
    // start
    nrf_drv_wdt_enable();
}
//--------------------------------------------------------------------------------------

/// check adn apply new time
///
void appNewTimeObtained (time_t time)
{
    // check the password 
    if( passwordCheckAndHandle() ) {
        // check the borders
        if( time < TIME_T_MAX )
            g_appState.m_appTime = time;
    }
}
//--------------------------------------------------------------------------------------

/// check and apply a new state
///
void appNewLedStateObtained (led_t* led, uint32_t ledIdx)
{    
    // check
    if( passwordCheckAndHandle() && ledsValueCheck(led, ledIdx) && ledIdx < LEDS_NUMBER ) {
        // apply
        g_appState.m_leds[ledIdx] = *led;
        //
        TRACE_MSG("led %d has been updated: on = %d, off = %d\n", ledIdx, led->time.timeTurnOn, led->time.timeTurnOff);
    }
}
//--------------------------------------------------------------------------------------

/// main
///
int main (void)
{     
    // wdt
    wdtInit();
    
    // application state
    appStateInit();
    
    // trace
    TRACE_INIT();
    
    // timer
    appTimersInit(); 
    
    // ble stack
    bleStackInit();
    // ble services
    bleServicesInit();
    // start 
    bleAdvertisingStart();
    
    // init
    ledsInit(g_appState.m_leds);
        
    // while
    for ever {
        // wait for an event
        sd_app_evt_wait();
        
        // feed a wdt
        nrf_drv_wdt_channel_feed(s_wdt);
             
        // leds
        ledsRoutine(g_appState.m_leds, g_appState.m_appTime);
        
        // time & leds
        if( g_appState.m_comm.m_isConnected ) {
            // time
            ble_svc_pw_tm_time_update(&g_appState.m_comm.m_ble_svc_pw_tm, g_appState.m_appTime);
            // leds
            ble_svc_leds_update(&g_appState.m_comm.m_ble_svc_leds, g_appState.m_leds);
        }
    }        
}
//--------------------------------------------------------------------------------------

