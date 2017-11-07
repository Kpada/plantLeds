#include "stdAfx.h"
#include "bleService_password_time.h"


#include "ble_srv_common.h"
#include "sdk_common.h"


#ifndef BLE_CONN_PASSWORD
    #error the password has not defined
#endif

/// on connect event handler
///
static __inline void on_connect (ble_svc_pw_tm_t * p_svc_pw, ble_evt_t * p_ble_evt)
{
    p_svc_pw->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}
//--------------------------------------------------------------------------------------

/// on disconnect event handler
///
static __inline void on_disconnect (ble_svc_pw_tm_t * p_svc_pw, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_svc_pw->conn_handle = BLE_CONN_HANDLE_INVALID;
}
//--------------------------------------------------------------------------------------

/// extract a password
///
static __inline ble_svc_password_t extractPasswordValue (ble_gatts_evt_write_t* p_evt_write)
{
    ble_svc_password_t pwObtained = DEAD_BEEF;
    
    // get a password value if possible
    if( p_evt_write && sizeof(ble_svc_password_t) == p_evt_write->len ) {
        pwObtained = (p_evt_write->data[0] << 0)  | 
                     (p_evt_write->data[1] << 8)  | 
                     (p_evt_write->data[2] << 16) | 
                     (p_evt_write->data[3] << 24); 
    }
    
    TRACE_MSG("pw = %d = 0x%08X\n", pwObtained, pwObtained);
    return pwObtained;
}
//--------------------------------------------------------------------------------------

/// time
///
static __inline time_t extractTimeValue (ble_gatts_evt_write_t* p_evt_write)
{
    time_t timeObtained = DEAD_BEEF;
    if( p_evt_write && sizeof(time_t) == p_evt_write->len )
        timeObtained =  (p_evt_write->data[0] << 0)  | 
                        (p_evt_write->data[1] << 8)  | 
                        (p_evt_write->data[2] << 16) | 
                        (p_evt_write->data[3] << 24); 
    
    TRACE_MSG("time has been obtained. time = %d\n", timeObtained);
    return timeObtained;    
}
//--------------------------------------------------------------------------------------

/// on write event
///
static __inline void on_write (ble_svc_pw_tm_t * p_svc_pw, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    TRACE_MSG("p_evt_write->handle = %d\n", p_evt_write->handle);
    
    // password
    if( p_evt_write->handle == p_svc_pw->pw_char_handles.value_handle && p_svc_pw->pw_write_handler ) {
        ble_svc_password_t pwObtained = extractPasswordValue(p_evt_write);
        p_svc_pw->pw_write_handler( pwObtained == BLE_CONN_PASSWORD );
    }  
    
    // time 
    if( p_evt_write->handle == p_svc_pw->time_char_handles.value_handle && p_svc_pw->time_write_handler ) {
        time_t time = extractTimeValue(p_evt_write);
        p_svc_pw->time_write_handler(time); 
    }
}
//--------------------------------------------------------------------------------------

/// service callback
/// connected, disconnected, write
///
void ble_svc_pw_on_ble_evt (ble_svc_pw_tm_t * p_svc_pw, ble_evt_t * p_ble_evt)
{
    switch( p_ble_evt->header.evt_id ) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_svc_pw, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_svc_pw, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(p_svc_pw, p_ble_evt);
            break;

        default:
            // Default. No implementation needed
            break;
    }
}
//--------------------------------------------------------------------------------------

/// add a new characteristic
///
static __inline uint32_t pw_char_add (ble_svc_pw_tm_t * p_svc_pw, const ble_svc_pw_init_t * p_svc_pw_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // clear GATT characteristic metadata
    memset(&char_md, 0, sizeof(char_md));
    // config it
    char_md.char_props.read   = NULL;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    // uuid
    ble_uuid.type = p_svc_pw->uuid_type;
    ble_uuid.uuid = SVC_PW_UUID_CHAR;
    
    // clear attribute metadata
    memset(&attr_md, 0, sizeof(attr_md));
    // config it
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    // clear GATT attribute
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    // config it
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(ble_svc_password_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(ble_svc_password_t);
    attr_char_value.p_value      = NULL;

    // add the new characteristic
    return sd_ble_gatts_characteristic_add( p_svc_pw->service_handle,
        &char_md,
        &attr_char_value,
        &p_svc_pw->pw_char_handles);
}
//--------------------------------------------------------------------------------------

/// time update
///
void ble_svc_pw_tm_time_update (ble_svc_pw_tm_t* p_svc_pw, time_t newVal)
{
    ble_gatts_value_t       gatts_value;
    
    // clear
    memset(&gatts_value, 0, sizeof(gatts_value));
    // init
    gatts_value.len     = sizeof(time_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&newVal;
     
    // Update database.
    sd_ble_gatts_value_set(p_svc_pw->conn_handle,
        p_svc_pw->time_char_handles.value_handle,
        &gatts_value);    
}
//--------------------------------------------------------------------------------------

/// add time ch
///
static uint32_t time_char_add (ble_svc_pw_tm_t * p_svc_pw, const ble_svc_pw_init_t * p_svc_pw_init)
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
    ble_uuid.type = p_svc_pw->uuid_type;
    ble_uuid.uuid = SVC_TIME_UUID_CHAR;
    
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
    attr_char_value.init_len     = sizeof(time_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(time_t);
    attr_char_value.p_value      = NULL;

    // add the new characteristic
    return sd_ble_gatts_characteristic_add(p_svc_pw->service_handle,
        &char_md,
        &attr_char_value,
        &p_svc_pw->time_char_handles);
}
//--------------------------------------------------------------------------------------

/// service initialization 
///
uint32_t ble_svc_pw_init (ble_svc_pw_tm_t * p_svc_pw, const ble_svc_pw_init_t * p_svc_pw_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_svc_pw->conn_handle           = BLE_CONN_HANDLE_INVALID;
    p_svc_pw->pw_write_handler      = p_svc_pw_init->pw_write_handler;
    p_svc_pw->time_write_handler    = p_svc_pw_init->time_write_handler;
    
    // Add service
    ble_uuid128_t base_uuid = {SVC_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_svc_pw->uuid_type);
    VERIFY_SUCCESS(err_code);

    // uuid
    ble_uuid.type = p_svc_pw->uuid_type;
    ble_uuid.uuid = SVC_PW_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_svc_pw->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add characteristics
    err_code = pw_char_add(p_svc_pw, p_svc_pw_init);
    VERIFY_SUCCESS(err_code);
    
    err_code = time_char_add(p_svc_pw, p_svc_pw_init);
    VERIFY_SUCCESS(err_code);
    
    return NRF_SUCCESS;       
}
//--------------------------------------------------------------------------------------

