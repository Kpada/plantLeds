#ifndef __TRACE_H__
#define __TRACE_H__

// trace redeclaration

// we will use the Segger RTT only

// is trace enabled?
#if defined(NRF_LOG_USES_RTT) && 1 == NRF_LOG_USES_RTT
    // initialization macro
    #define TRACE_INIT()        log_rtt_init()
    // trace a message macro
    #define TRACE_MSG(...)      log_rtt_printf(LOG_TERMINAL_NORMAL, ##__VA_ARGS__)
#else 
    // do nothing
    #define TRACE_INIT()
    #define TRACE_MSG(...)
#endif // NRF_LOG_USES_RTT
//--------------------------------------------------------------------------------------


// check for the debug state and warn a coder if necessary
#if defined(NRF_LOG_USES_RTT) && 1 == NRF_LOG_USES_RTT && !defined(DEBUG) 
    #warning Attention. Trace is enabled. 
#endif // defined(NRF_LOG_USES_RTT) & !defined(DEBUG) 
//--------------------------------------------------------------------------------------


#endif // __TRACE_H__
