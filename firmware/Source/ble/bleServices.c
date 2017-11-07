#include "stdAfx.h"
#include "bleServices.h"

#include "ble_dis.h"


/// device info
///
static void bleServiceDeviceInfoInit (void)
{
    // Initialize Device Information Service.
    ble_dis_init_t  dis_init;
    memset(&dis_init, 0, sizeof(dis_init));
    
    // strings
    static const char* strModel = APP_NAME_STR;   
    static const char* strFwVer = APP_VERSION_STR; 
    
    // model
    dis_init.model_num_str.length = strlen(strModel);
    dis_init.model_num_str.p_str = (uint8_t*)strModel;
    
    // firmware
    dis_init.fw_rev_str.length = strlen(strFwVer);
    dis_init.fw_rev_str.p_str = (uint8_t*)strFwVer;
  
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    uint32_t err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}
//--------------------------------------------------------------------------------------

/// password service initialization
///
static void bleServicePasswordInit (void)
{
    // init
    ble_svc_pw_init_t   pw_init;
    memset(&pw_init, 0, sizeof(pw_init));
    
    // password write handler
    pw_init.pw_write_handler   = appIsObtainedPasswordCorrect;
    pw_init.time_write_handler = appNewTimeObtained;
    
    // apply and check
    uint32_t err_code = ble_svc_pw_init(&g_appState.m_comm.m_ble_svc_pw_tm, &pw_init); 
    APP_ERROR_CHECK(err_code);    
}
//--------------------------------------------------------------------------------------

/// leds service initializaiont
///
static void bleServiceLedsInit (void)
{
    ble_svc_leds_init_t led_init;
    memset(&led_init, 0, sizeof(led_init));
    
    // handler
    led_init.led_write_handler = appNewLedStateObtained;
    
    // apply and check
    uint32_t err_code = ble_svc_leds_init(&g_appState.m_comm.m_ble_svc_leds, &led_init); 
    APP_ERROR_CHECK(err_code);  
}
//--------------------------------------------------------------------------------------

/// app services init
///
void bleServicesInit (void)
{
    // device info
    TRACE_MSG("device info\n");
    bleServiceDeviceInfoInit();   
    
    // leds
    TRACE_MSG("leds info\n");
    bleServiceLedsInit();
    
    // password
    TRACE_MSG("password and time\n");
    bleServicePasswordInit();
}
//--------------------------------------------------------------------------------------



