#pragma once
#include <3ds.h>

u64     Timer_Restart(void);
bool    Timer_HasTimePassed(u32 nbmsecToWait, u64 timer);
u64		getTimeInMsec(u64 timer);