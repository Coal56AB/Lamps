#ifndef RTC_MONITOR_H
#define RTC_MONITOR_H

#include <stdint.h>

typedef enum
{
    RTC_MONITOR_HEALTHY = 0,
    RTC_MONITOR_SUSPECT,
    RTC_MONITOR_RECOVERING,
    RTC_MONITOR_LSE_FAILED
} RTC_MonitorStatus;

/* Start the independent HSE/DWT reference and take the first RTC sample. */
void RTC_Monitor_Init(void);

/* Call as often as possible from the main loop. */
void RTC_Monitor_Process(void);

/* Call after a deliberate user time change so the reference follows it. */
void RTC_Monitor_NotifyTimeChanged(void);

RTC_MonitorStatus RTC_Monitor_GetStatus(void);
uint32_t RTC_Monitor_GetRecoveryCount(void);
uint32_t RTC_Monitor_GetFailureCount(void);

#endif
