#ifndef __BLE_SERVICE_LEDS_H__
#define __BLE_SERVICE_LEDS_H__
 
typedef void (*ble_svc_led_write_handler_t) (led_t* led, uint32_t ledIdx);
//--------------------------------------------------------------------------------------

// declaration of the pw struct type
typedef struct {
    uint16_t                        service_handle;   
    uint8_t                         uuid_type;          
    uint16_t                        conn_handle; 
    ble_gatts_char_handles_t        char_handles [LEDS_NUMBER];      
    // write handler
    ble_svc_led_write_handler_t     led_write_handler;  
} ble_svc_leds_t;
//--------------------------------------------------------------------------------------


// Service init structure. 
// This structure contains all options and data needed for initialization of the service.
typedef struct {
    ble_svc_led_write_handler_t     led_write_handler; 
} ble_svc_leds_init_t;
//--------------------------------------------------------------------------------------


// service initializaion
uint32_t ble_svc_leds_init (ble_svc_leds_t* p_svc_leds, const ble_svc_leds_init_t* p_svc_leds_init);
//--------------------------------------------------------------------------------------

// service callback
void ble_svc_leds_on_ble_evt (ble_svc_leds_t* p_svc_leds, ble_evt_t* p_ble_evt);
//--------------------------------------------------------------------------------------

// update the values
void ble_svc_leds_update (ble_svc_leds_t* p_svc_leds, led_t* ledsBuff);
//--------------------------------------------------------------------------------------

#endif // __BLE_SERVICE_LEDS_H__
