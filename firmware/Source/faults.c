#include "stdAfx.h"

/// hard fault
///
void app_error_fault_handler (uint32_t id, uint32_t pc, uint32_t info)
{
    
#ifndef DEBUG
    // release. reset the target
	sd_nvic_SystemReset();
#else
    // debug. show a message and then do nothing
	TRACE_MSG("FAULT! APP ERROR: ID: %d, PC: %d, Info: %d\n", id, pc, info);
	for ever {}
#endif
        
}
//--------------------------------------------------------------------------------------
