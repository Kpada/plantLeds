#ifndef __BLE_SERVICE_PASSWORD_AND_TIME_H__
#define __BLE_SERVICE_PASSWORD_AND_TIME_H__

// password type
typedef uint32_t                ble_svc_password_t;
//--------------------------------------------------------------------------------------
   
// handlers
typedef void (*ble_svc_pw_write_handler_t) (bool isPasswordCorrect);
typedef void (*ble_svc_time_write_handler_t) (time_t time);
//--------------------------------------------------------------------------------------

// declaration of the pw struct type
typedef struct
{
    uint16_t                        service_handle;         // Handle
    ble_gatts_char_handles_t        pw_char_handles;        // Handles related to the PASSWORD Characteristic
    ble_gatts_char_handles_t        time_char_handles;      // Handles related to the TIME Characteristic
    uint8_t                         uuid_type;              // UUID type for the Service
    uint16_t                        conn_handle;            // Handle of the current connection (as provided by the BLE stack). BLE_CONN_HANDLE_INVALID if not in a connection
    
    // write handlers
    ble_svc_pw_write_handler_t      pw_write_handler;       // password
    ble_svc_time_write_handler_t    time_write_handler;     // time

} ble_svc_pw_tm_t;
//--------------------------------------------------------------------------------------

// Service init structure. 
// This structure contains all options and data needed for initialization of the service.
typedef struct {
    ble_svc_pw_write_handler_t      pw_write_handler;   // Event handler to be called when the PASSWORD Characteristic is written.
    ble_svc_time_write_handler_t    time_write_handler; // Event handler to be called when the TIME Characteristic is written.
} ble_svc_pw_init_t;
//--------------------------------------------------------------------------------------

// service initialization 
uint32_t ble_svc_pw_init (ble_svc_pw_tm_t * p_svc_pw, const ble_svc_pw_init_t * p_svc_pw_init);
//--------------------------------------------------------------------------------------

// service callback
void ble_svc_pw_on_ble_evt (ble_svc_pw_tm_t * p_svc_pw, ble_evt_t * p_ble_evt);
//--------------------------------------------------------------------------------------

// set a new time value
void ble_svc_pw_tm_time_update (ble_svc_pw_tm_t* p_svc_pw, time_t newVal);
//--------------------------------------------------------------------------------------

#endif // __BLE_SERVICE_PASSWORD_AND_TIME_H__
