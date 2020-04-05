#pragma once

#define KQ_ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

// TODO(cj): This is gcc specific.
#ifdef __GNUC__
#define KQ_STATIC_ASSERT(cond) _Static_assert(cond, "no message")
#else
#define KQ_STATIC_ASSERT(cond) assert(cond)
#endif

