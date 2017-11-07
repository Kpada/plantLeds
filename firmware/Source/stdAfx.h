#ifndef __STD_AFX_H__
#define __STD_AFX_H__

// std libraries
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//--------------------------------------------------------------------------------------

// data types declaration
#include "typedef.h"
//--------------------------------------------------------------------------------------

// config
#include "config/boardConfig.h"
//--------------------------------------------------------------------------------------

// nrf common
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_drv_wdt.h"
#include "app_timer.h"
#include "pstorage.h"
#include "device_manager.h"
#include "app_util_platform.h"
//--------------------------------------------------------------------------------------

// nrf ble
#include "ble.h"
#include "ble_hci.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_gap.h"
#include "ble_advertising.h"
//--------------------------------------------------------------------------------------

// utils
#include "trace.h"
#include "bleConn.h"
#include "leds.h"
//--------------------------------------------------------------------------------------

// services and their configurations
#include "config/ble_password.h"
#include "ble/bleService_password_time.h"
#include "ble/bleService_leds.h"
//--------------------------------------------------------------------------------------

// application
#include "main.h"
//--------------------------------------------------------------------------------------

#endif // __STD_AFX_H__
