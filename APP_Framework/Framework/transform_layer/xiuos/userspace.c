#include <stdio.h>
#include <stdint.h>

extern int main(void);
extern void UserTaskQuit(void);
extern uintptr_t _ustext;
extern uintptr_t _uetext;
extern uintptr_t _ueronly;
extern uintptr_t _usdata;
extern uintptr_t _uedata;
extern uintptr_t _usbss;
extern uintptr_t _uebss;
typedef int (*main_t)(int argc, char *argv[]);
typedef void (*exit_t)(void);
struct userspace_s
{
  main_t    us_entrypoint;
  exit_t    us_taskquit;
  uintptr_t us_textstart;
  uintptr_t us_textend;
  uintptr_t us_datasource;
  uintptr_t us_datastart;
  uintptr_t us_dataend;
  uintptr_t us_bssstart;
  uintptr_t us_bssend;
  uintptr_t us_heapend;
};

const struct userspace_s userspace __attribute__ ((section (".userspace"))) =
{
  /* General memory map */

  .us_entrypoint    = (main_t)main,
  .us_taskquit      = (exit_t)UserTaskQuit,
  .us_textstart     = (uintptr_t)&_ustext,
  .us_textend       = (uintptr_t)&_uetext,
  .us_datasource    = (uintptr_t)&_ueronly,
  .us_datastart     = (uintptr_t)&_usdata,
  .us_dataend       = (uintptr_t)&_uedata,
  .us_bssstart      = (uintptr_t)&_usbss,
  .us_bssend        = (uintptr_t)&_uebss,

};