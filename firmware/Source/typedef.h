#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

// dead beef
#define DEAD_BEEF                   ( 0xDEADBEEF )

// for ever
#define ever                        ( ; ; )

// time
typedef uint32_t                    time_t;         // a number of seconds since 00:00, 0 .. 86399        
#define TIME_T_MAX                  ( 86400 - 1 )   // max
#define TIME_T_MIN                  ( 0 )           // min

// convert time (hour, min, sec) to time_t
#define TIME_2_TIME_T( h, m, s )    ( h*3600 + m*60 + s )

#endif  // __TYPEDEF_H__ 
