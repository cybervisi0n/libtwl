#ifndef NITRO_MEMORYMAP_H_
#define NITRO_MEMORYMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL

#ifdef SDK_PORT
#ifdef SDK_PORT
#include <nitro/hw/X86/mmap_global.h>
#include <nitro/hw/X86/mmap_main.h>
#include <nitro/hw/X86/mmap_tcm.h>
#include <nitro/hw/X86/mmap_vram.h>
#include <nitro/hw/X86/mmap_shared.h>
#include <nitro/hw/X86/ioreg.h>
#endif
#else
#ifdef SDK_ARM9
#include <nitro/hw/ARM9/mmap_global.h>
#include <nitro/hw/ARM9/mmap_main.h>
#include <nitro/hw/ARM9/mmap_tcm.h>
#include <nitro/hw/ARM9/mmap_vram.h>
#include <nitro/hw/common/mmap_shared.h>
#include <nitro/hw/ARM9/ioreg.h>

#else // SDK_ARM7
#include <nitro/hw/ARM7/mmap_global.h>
#include <nitro/hw/ARM7/mmap_main.h>
#include <nitro/hw/ARM7/mmap_wram.h>
#include <nitro/hw/common/mmap_shared.h>
#include <nitro/hw/ARM7/ioreg.h>
#endif
#endif

#endif // SDK_NITRO

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
