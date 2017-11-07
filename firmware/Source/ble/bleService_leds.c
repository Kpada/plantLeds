#include "stdAfx.h"
#include "bleService_leds.h"

#include "ble_srv_common.h"
#include "sdk_common.h"

/// get an idx from the event
///
__inline static uint32_t getLedIdxFromEvt (ble_svc_leds_t * p_svc_leds, ble_gatts_evt_write_t* p_evt_write)
{
    uint32_t charIdx = 0;
    bool weGotIt = false;
    
    while( !weGotIt && charIdx < LEDS_NUMBER ) 
        weGotIt = p_evt_write->handle == p_svc_leds->char_handles[charIdx++].value_handle;
    
    return weGotIt ? charIdx - 1 : DEAD_BEEF;
}
//--------------------------------------------------------------------------------------

/// extract an obtained value
///
__inline static led_time_t extractWriteValue (ble_gatts_evt_write_t* p_evt_write)
{
    led_time_t ledTime = {DEAD_BEEF, DEAD_BEEF};

    if( p_evt_write && sizeof(ledTime) == p_evt_write->len ) {
        // copy
        memcpy(&ledTime, p_evt_write->data, p_evt_write->len);
        TRACE_MSG("new led state has been obtained. on = %d, off = %d\n", ledTime.timeTurnOn, ledTime.timeTurnOff);
    }
      
    return ledTime;    
}
//--------------------------------------------------------------------------------------

/// on connect
///
__inline static void on_connect (ble_svc_leds_t * p_svc, ble_evt_t * p_ble_evt)
{
    p_svc->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}
//--------------------------------------------------------------------------------------

/// on disconnect
///
__inline static void on_disconnect (ble_svc_leds_t * p_svc, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_svc->conn_handle = BLE_CONN_HANDLE_INVALID;
}
//--------------------------------------------------------------------------------------

/// on write
///
__inline static void on_write (ble_svc_leds_t * p_svc, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    uint32_t ledIdx = getLedIdxFromEvt(p_svc, p_evt_write);
    if( DEAD_BEEF !=  ledIdx) {   
        // extract a value
        led_time_t ledTime = extractWriteValue(p_evt_write);
        // is it correct?
        if( ledTime.timeTurnOn != DEAD_BEEF && ledTime.timeTurnOff != DEAD_BEEF ) {
            led_t led;
            led.time = ledTime;
            if( p_svc->led_write_handler ) {
                p_svc->led_write_handler(&led, ledIdx);
            }
            else {
                TRACE_MSG("a write handler isn't available\n");
            }
        }
    }
}
//--------------------------------------------------------------------------------------

/// service callback
/// connected, disconnected, write
///
void ble_svc_leds_on_ble_evt (ble_svc_leds_t * p_svc, ble_evt_t * p_ble_evt)
{
    switch( p_ble_evt->header.evt_id ) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_svc, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_svc, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(p_svc, p_ble_evt);
            break;

        default:
            // Default. No implementation needed.
            break;
    }
}
//--------------------------------------------------------------------------------------

/// update a value with an idx (template for a single led)
///
static void ble_svc_leds_update_template (ble_svc_leds_t* p_svc_leds, led_t* led, int ledIdx)
{
    ble_gatts_value_t       gatts_value;
    
    // clear
    memset(&gatts_value, 0, sizeof(gatts_value));
    // init
    gatts_value.len     = sizeof(led_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)led;
     
    // Update database.
    sd_ble_gatts_value_set(p_svc_leds->conn_handle,
        p_svc_leds->char_handles[ledIdx].value_handle,
        &gatts_value);     
}
//--------------------------------------------------------------------------------------

/// update the values
///
void ble_svc_leds_update (ble_svc_leds_t* p_svc_leds, led_t* ledsBuff)
{
    // check
    if( !p_svc_leds || !ledsBuff )
        return;
    
    // update
    int ledIdx;
    for( ledIdx = 0; ledIdx < LEDS_NUMBER; ledIdx++ )
        ble_svc_leds_update_template( p_svc_leds, &ledsBuff[ledIdx], ledIdx );
}
//--------------------------------------------------------------------------------------

/// add a new characteristic
///
static uint32_t led_char_add (ble_svc_leds_t * p_svc_leds, ble_gatts_char_handles_t* p_char_hndls, uint16_t uuid)
{
    ble_gatts_char_md_t     char_md;
    ble_gatts_attr_t        attr_char_value;
    ble_uuid_t              ble_uuid;
    ble_gatts_attr_md_t     attr_md;
    
    // clear GATT characteristic metadata
    memset(&char_md, 0, sizeof(char_md));
    // init it
    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    // uuid
    ble_uuid.type = p_svc_leds->uuid_type;
    ble_uuid.uuid = uuid;
    
    // clear attribute metadata
    memset(&attr_md, 0, sizeof(attr_md));
    // init it
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    // clear GATT attribute
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    // init it
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(led_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(led_t);
    attr_char_value.p_value      = NULL;

    // add the new characteristic
    return sd_ble_gatts_characteristic_add(p_svc_leds->service_handle,
        &char_md,
        &attr_char_value,
        p_char_hndls);
}
//--------------------------------------------------------------------------------------

/// service initialization 
///
uint32_t ble_svc_leds_init (ble_svc_leds_t* p_svc_leds, const ble_svc_leds_init_t* p_svc_leds_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure.
    p_svc_leds->conn_handle       = BLE_CONN_HANDLE_INVALID;
    p_svc_leds->led_write_handler = p_svc_leds_init->led_write_handler;
 
    // Add service.
    ble_uuid128_t base_uuid = {SVC_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_svc_leds->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    // uuid
    ble_uuid.type = p_svc_leds->uuid_type;
    ble_uuid.uuid = SVC_LEDS_UUID_SERVICE;
    
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_svc_leds->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add characteristics
    int ledIdx;
    uint16_t charUuid = SVC_LEDS_UIID_CHAR_BASE;
    for( ledIdx = 0; ledIdx < LEDS_NUMBER; ledIdx++, charUuid++ ) {
        err_code = led_char_add(p_svc_leds, &p_svc_leds->char_handles[ledIdx], charUuid);
        VERIFY_SUCCESS(err_code);    
    }
    
    return NRF_SUCCESS;       
}
//--------------------------------------------------------------------------------------
