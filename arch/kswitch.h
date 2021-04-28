
#ifndef __KSWITCH_H__
#define __KSWITCH_H__

#ifdef SEPARATE_COMPILE

#if defined(ARCH_RISCV)
#include "risc-v/shared/kswitch.h"
#endif

#if defined(ARCH_ARM)
#include "arm/cortex-m4/kswitch.h"
#endif

#endif

#endif