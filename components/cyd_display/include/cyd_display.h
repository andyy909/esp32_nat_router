#pragma once

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_CYD_DISPLAY
void cyd_display_init(void);
#else
static inline void cyd_display_init(void) {}
#endif

#ifdef __cplusplus
}
#endif
