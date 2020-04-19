#include "timer.h"

#include "universal/universal.h"

#include <stdint.h>

#ifdef KQ_PLATFORM_APPLE

#include <CoreServices/CoreServices.h>

#include <mach/mach.h>
#include <mach/mach_time.h>

struct TimerImpl
{
	uint64_t					start;
	uint64_t					stop;
    mach_timebase_info_data_t	timebaseInfo;
};

KQ_STATIC_ASSERT(sizeof(struct TimerImpl) < sizeof(struct Timer));

void Timer_Init(struct Timer* timer)
{
	struct TimerImpl* impl = (struct TimerImpl*)timer;
	mach_timebase_info(&impl->timebaseInfo);
}

void Timer_Start(struct Timer* timer)
{
	struct TimerImpl* impl = (struct TimerImpl*)timer;
	impl->start = mach_absolute_time();
}

void Timer_Stop(struct Timer* timer)
{
	struct TimerImpl* impl = (struct TimerImpl*)timer;
	impl->stop = mach_absolute_time();
}

double Timer_GetElapsedSeconds(const struct Timer* timer)
{
	const struct TimerImpl* impl = (const struct TimerImpl*)timer;

	const uint64_t elapsed = impl->stop - impl->start;

    const uint64_t elapsedNano =
		elapsed * impl->timebaseInfo.numer / impl->timebaseInfo.denom;

    return (double)elapsedNano / 1000000000.0;
}

#endif
