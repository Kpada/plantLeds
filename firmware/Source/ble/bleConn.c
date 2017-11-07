#include "stdAfx.h"
#include "bleConn.h"

#define CENTRAL_LINK_COUNT                  0   /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT               1   /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

/// GAP connection params
///
#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS(BLE_CONN_INTRVL_MIN_MSEC, UNIT_1_25_MS)      /**< Minimum acceptable connection interval */
#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(BLE_CONN_INTRVL_MAX_MSEC, UNIT_1_25_MS)      /**< Maximum acceptable connection interval */
#define SLAVE_LATENCY                       0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory time-out */

/// Advertising params
///
#define APP_ADV_INTERVAL                    MSEC_TO_UNITS(BLE_ADVERTISING_PERIOD_MSEC, UNIT_0_625_MS)/**< The advertising interval (in units of 0.625 ms; this) */
#define APP_ADV_TIMEOUT_IN_SECONDS          BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED       /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */


/// security
///
#define SEC_PARAM_BOND                      1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                      0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                                         /**< Maximum encryption key size. */


#define FIRST_CONN_PARAMS_UPDATE_DELAY      5000                                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY       30000                                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_FEATURE_NOT_SUPPORTED           BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */




/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
__inline static void bleGapParamsInit (void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t*)DEVICE_NAME, strlen(DEVICE_NAME) );
    APP_ERROR_CHECK(err_code);

    // reset it
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    // config
    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;
    // apply
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}
//--------------------------------------------------------------------------------------


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void bleOnAdvEvt (ble_adv_evt_t ble_adv_evt)
{
    switch( ble_adv_evt ) {
        case BLE_ADV_EVT_FAST:
            break;
        case BLE_ADV_EVT_IDLE:
            break;
        default:
            break;
    }
}
//--------------------------------------------------------------------------------------


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init (void)
{
    ble_advdata_t advdata;  // Struct containing advertising parameters

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.short_name_len          = strlen(DEVICE_NAME);
    advdata.include_appearance      = false;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = 0;

    //sd_ble_gap_appearance_set(DEVICE_APPEARANCE);
    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;
    
    uint32_t err_code = ble_advertising_init(&advdata, NULL, &options, bleOnAdvEvt, NULL);
    APP_ERROR_CHECK(err_code);   
}
//--------------------------------------------------------------------------------------

static void bleEvtDispatchToModules (ble_evt_t * p_ble_evt)
{  
    // Dispatch BLE events to OS event queue
    appOnBleEvtHandle(p_ble_evt);
    
    // Device manager module events handling
    dm_ble_evt_handler(p_ble_evt);
        
    // custom services
    ble_svc_pw_on_ble_evt(&g_appState.m_comm.m_ble_svc_pw_tm, p_ble_evt);
    ble_svc_leds_on_ble_evt(&g_appState.m_comm.m_ble_svc_leds, p_ble_evt);
         
    // BLE Connection events handling
    ble_conn_params_on_ble_evt(p_ble_evt);
  
    // BLE advertizing module events handling
    ble_advertising_on_ble_evt(p_ble_evt);
}
//--------------------------------------------------------------------------------------


static void bleOnSysEvtDispatch (uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}
//--------------------------------------------------------------------------------------


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void bleInit (void)
{
    // Initialize SoftDevice.
    SOFTDEVICE_HANDLER_INIT( CLOCK_LFCL_SRC, NULL);

    ble_enable_params_t     ble_enable_params;
    uint32_t err_code;
    
      
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT, &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    ble_enable_params.common_enable_params.vs_uuid_count = 2;//BLE_SPECIFIC_UUID_CNT;
    
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(bleEvtDispatchToModules);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(bleOnSysEvtDispatch);
    APP_ERROR_CHECK(err_code);
}
//--------------------------------------------------------------------------------------

///
///
static void onBleConnParamsEvt (ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
        err_code = sd_ble_gap_disconnect(g_appState.m_comm.m_hBleConn, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}
//--------------------------------------------------------------------------------------


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}
//--------------------------------------------------------------------------------------

///
///
static void bleConnParamsInit(void)
{
    uint32_t err_code;
    
    ble_conn_params_init_t cp_init;
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = onBleConnParamsEvt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}
//--------------------------------------------------------------------------------------


///
///
static uint32_t bleOnDeviceManagerEvt(dm_handle_t const * p_handle,
                                     dm_event_t const  * p_event,
                                     ret_code_t        event_result)
{
    APP_ERROR_CHECK(event_result);
    return NRF_SUCCESS;
}
//--------------------------------------------------------------------------------------

///
///
static void bleDevmgrInit (bool erase_bonds)
{
    uint32_t err_code;
    dm_init_param_t init_param = {
        .clear_persistent_data = erase_bonds
    };
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = bleOnDeviceManagerEvt;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(
        &g_appState.m_comm.m_hBleApp, 
        &register_param
    );
    APP_ERROR_CHECK(err_code);
}
//--------------------------------------------------------------------------------------

/// config
///
void bleStackInit (void)
{
    // init
    bleInit();
    
    // device manager
    bleDevmgrInit(false);
    
    // gap
    bleGapParamsInit();
    
    // device 
    advertising_init();  
    
    // connection
    bleConnParamsInit();
}
//--------------------------------------------------------------------------------------

/// start
///
void bleAdvertisingStart (void)
{
    uint32_t err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    
    // tx gain 
    sd_ble_gap_tx_power_set(BLE_TX_GAIN_dB);
}
//--------------------------------------------------------------------------------------

