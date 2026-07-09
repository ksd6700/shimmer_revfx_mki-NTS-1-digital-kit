#include "userrevfx.h"

extern uint8_t _bss_start;
extern uint8_t _bss_end;

extern void (*__init_array_start []) (void);
extern void (*__init_array_end []) (void);

typedef void (*__init_fptr)(void);

__attribute__((used, section(".hooks")))
static const user_revfx_hook_table_t s_hook_table = {
  .magic = {'U','R','E','V'},
  .api = USER_API_VERSION,
  .platform = USER_TARGET_PLATFORM>>8,
  .reserved0 = {0},
  .func_entry = _entry,
  .func_process = _hook_process,
  .func_suspend = _hook_suspend,
  .func_resume = _hook_resume,
  .func_param = _hook_param,
  .reserved1 = {0}
};

__attribute__((used))
void _entry(uint32_t platform, uint32_t api)
{
  uint8_t * __restrict bss_p = (uint8_t *)&_bss_start;
  const uint8_t * const bss_e = (uint8_t *)&_bss_end;

  for (; bss_p != bss_e;) {
    *(bss_p++) = 0;
  }

  const size_t count = __init_array_end - __init_array_start;
  for (size_t i = 0; i < count; ++i) {
    __init_fptr init_p = (__init_fptr)__init_array_start[i];
    if (init_p != NULL) {
      init_p();
    }
  }

  _hook_init(platform, api);
}

__attribute__((weak))
void _hook_init(uint32_t platform, uint32_t api)
{
  (void)platform;
  (void)api;
}

__attribute__((weak))
void _hook_process(float *xn, uint32_t frames)
{
  (void)xn;
  (void)frames;
}

__attribute__((weak))
void _hook_suspend(void)
{
}

__attribute__((weak))
void _hook_resume(void)
{
}

__attribute__((weak))
void _hook_param(uint8_t index, int32_t value)
{
  (void)index;
  (void)value;
}
