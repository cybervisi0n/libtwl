#include <nitro/os.h>

void OSi_IrqCallback(int dmaNo);

void OSi_IrqTimer0(void);
void OSi_IrqTimer1(void);
void OSi_IrqTimer2(void);
void OSi_IrqTimer3(void);

void OSi_IrqDma0(void);
void OSi_IrqDma1(void);
void OSi_IrqDma2(void);
void OSi_IrqDma3(void);

#ifdef SDK_ARM7
void OSi_IrqVBlank(void);
#endif

#ifdef SDK_TWL
void OSi_IrqNDma0(void);
void OSi_IrqNDma1(void);
void OSi_IrqNDma2(void);
void OSi_IrqNDma3(void);
#endif

#ifdef SDK_NITRO
#if defined(SDK_ARM9) || defined(SDK_PORT)
#include <nitro/dtcm_begin.h>
#endif // SDK_ARM9
OSIrqFunction OS_IRQTable[OS_IRQ_TABLE_MAX] = {
#ifdef SDK_ARM9
    OS_IrqDummy, //  0 VBlank (for ARM9)
#else
    OSi_IrqVBlank, //  0 VBlank (for ARM7)
#endif             // SDK_ARM9
    OS_IrqDummy,   //  1 HBlank
    OS_IrqDummy,   //  2 VCounter
    OSi_IrqTimer0, //  3 timer0
    OSi_IrqTimer1, //  4 timer1
    OSi_IrqTimer2, //  5 timer2
    OSi_IrqTimer3, //  6 timer3
    OS_IrqDummy,   //  7 serial communication (will not occur)
    OSi_IrqDma0,   //  8 DMA0
    OSi_IrqDma1,   //  9 DMA1
    OSi_IrqDma2,   // 10 DMA2
    OSi_IrqDma3,   // 11 DMA3
    OS_IrqDummy,   // 12 key
    OS_IrqDummy,   // 13 cartridge
    OS_IrqDummy,   // 14 (not used)
    OS_IrqDummy,   // 15 (not used)
    OS_IrqDummy,   // 16 sub processor
    OS_IrqDummy,   // 17 sub processor send FIFO empty
    OS_IrqDummy,   // 18 sub processor receive FIFO not empty
    OS_IrqDummy,   // 19 card data transfer finish
    OS_IrqDummy,   // 20 card IREQ
#ifdef SDK_ARM9
    OS_IrqDummy, // 21 geometry command FIFO
#else
    OS_IrqDummy, // 21 (not used)
    OS_IrqDummy, // 22 Power Management IC
    OS_IrqDummy, // 23 SPI data transfer
    OS_IrqDummy  // 24 Wireless module
#endif // SDK_ARM9
};
#if defined(SDK_ARM9) || defined(SDK_PORT)
#include <nitro/dtcm_end.h>
#endif // SDK_ARM9
#else

#if defined(SDK_ARM9) || defined(SDK_PORT)
#include <twl/dtcm_begin.h>
#endif // SDK_ARM9

#ifndef SDK_PORT
OSIrqFunction OS_IRQTable[OS_IRQ_TABLE_MAX] = {
#if defined(SDK_ARM9) || defined(SDK_PORT)
    OS_IrqDummy, //  0 VBlank (for ARM9)
#else
    OSi_IrqVBlank, //  0 VBlank (for ARM7)
#endif // SDK_ARM9
    OS_IrqDummy,   //  1 HBlank
    OS_IrqDummy,   //  2 VCounter
    OSi_IrqTimer0, //  3 timer0
    OSi_IrqTimer1, //  4 timer1
    OSi_IrqTimer2, //  5 timer2
    OSi_IrqTimer3, //  6 timer3
    OS_IrqDummy,   //  7 serial communication (will not occur)
    OSi_IrqDma0,   //  8 DMA0
    OSi_IrqDma1,   //  9 DMA1
    OSi_IrqDma2,   // 10 DMA2
    OSi_IrqDma3,   // 11 DMA3
    OS_IrqDummy,   // 12 key
    OS_IrqDummy,   // 13 cartridge
    OS_IrqDummy,   // 14 memory card(A) MC_DET low
    OS_IrqDummy,   // 15 memory card(B) MC_DET low
    OS_IrqDummy,   // 16 sub processor
    OS_IrqDummy,   // 17 sub processor send FIFO empty
    OS_IrqDummy,   // 18 sub processor receive FIFO not empty
    OS_IrqDummy,   // 19 card(A) data transfer finish
    OS_IrqDummy,   // 20 card(B) IREQ
#ifdef SDK_ARM9
    OS_IrqDummy, // 21 geometry command FIFO
    OS_IrqDummy, // 22 read debug channel RECV buffer
    OS_IrqDummy, // 23 write debug channel SEND buffer
    OS_IrqDummy, // 24 DSP
    OS_IrqDummy, // 25 camera
#else  // (for ARM7)
    OS_IrqDummy, // 21 read debug channel RECV buffer
    OS_IrqDummy, // 22 Power Management IC
    OS_IrqDummy, // 23 SPI data transfer
    OS_IrqDummy, // 24 Wireless module
    OS_IrqDummy, // 25 write debug channel SEND buffer
#endif // SDK_ARM9
    OS_IrqDummy,  // 26 card(B) data transfer finished
    OS_IrqDummy,  // 27 card(B) IREQ
    OSi_IrqNDma0, // 28 new DMA0
    OSi_IrqNDma1, // 29 new DMA1
    OSi_IrqNDma2, // 30 new DMA2
    OSi_IrqNDma3, // 31 new DMA3
};
#endif

#ifdef SDK_ARM7
OSIrqFunction OS_IRQTable2[OS_IRQ_TABLE2_MAX] = {
    OSi_IrqNDma3, //  0 GPIO18[0]
    OSi_IrqNDma3, //  1 GPIO18[1]
    OSi_IrqNDma3, //  2 GPIO18[2]
    OSi_IrqNDma3, //  3 (not used)
    OSi_IrqNDma3, //  4 GPIO33[0]
    OSi_IrqNDma3, //  5 GPIO33[1]
    OSi_IrqNDma3, //  6 GPIO33[2]
    OSi_IrqNDma3, //  7 GPIO33[3]
    OSi_IrqNDma3, //  8 SD card 1
    OSi_IrqNDma3, //  9 SDIO card 1 async
    OSi_IrqNDma3, // 10 SD card 2
    OSi_IrqNDma3, // 11 SDIO card 2 async
    OSi_IrqNDma3, // 12 AES process finished
    OSi_IrqNDma3, // 13 I2C transfer finished
    OSi_IrqNDma3, // 14 MIC
};
#endif
#ifdef SDK_ARM9
#include <twl/dtcm_end.h>
#endif // SDK_ARM9
#endif // SDK_NITRO

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include <nitro/dtcm_begin.h>
#endif
#ifdef SDK_NITRO

OSIrqCallbackInfo OSi_IrqCallbackInfo[OSi_IRQCALLBACK_NUM] = {
    {
        NULL,
        0,
        0,
    }, // dma0
    {
        NULL,
        0,
        0,
    }, // dma1
    {
        NULL,
        0,
        0,
    }, // dma2
    {
        NULL,
        0,
        0,
    }, // dma3

    {
        NULL,
        0,
        0,
    }, // timer0
    {
        NULL,
        0,
        0,
    }, // timer1
    {
        NULL,
        0,
        0,
    }, // timer2
    {
        NULL,
        0,
        0,
    }, // timer3
#ifdef SDK_ARM7
    {
        NULL,
        0,
        0,
    }  // vblank
#endif // SDK_ARM7
};

static u16 OSi_IrqCallbackInfoIndex[OSi_IRQCALLBACK_NUM] = {
    REG_OS_IE_D0_SHIFT, REG_OS_IE_D1_SHIFT, REG_OS_IE_D2_SHIFT,
    REG_OS_IE_D3_SHIFT, REG_OS_IE_T0_SHIFT, REG_OS_IE_T1_SHIFT,
    REG_OS_IE_T2_SHIFT, REG_OS_IE_T3_SHIFT,
#ifdef SDK_ARM7
    REG_OS_IE_VB_SHIFT
#endif // SDK_ARM7
};
#else // SDK_NITRO

OSIrqCallbackInfo OSi_IrqCallbackInfo[OSi_IRQCALLBACK_NUM] = {
    {
        NULL,
        0,
        0,
    }, // dma0
    {
        NULL,
        0,
        0,
    }, // dma1
    {
        NULL,
        0,
        0,
    }, // dma2
    {
        NULL,
        0,
        0,
    }, // dma3

    {
        NULL,
        0,
        0,
    }, // new dma0
    {
        NULL,
        0,
        0,
    }, // new dma1
    {
        NULL,
        0,
        0,
    }, // new dma2
    {
        NULL,
        0,
        0,
    }, // new dma3

    {
        NULL,
        0,
        0,
    }, // timer0
    {
        NULL,
        0,
        0,
    }, // timer1
    {
        NULL,
        0,
        0,
    }, // timer2
    {
        NULL,
        0,
        0,
    }, // timer3
#ifdef SDK_ARM7
    {
        NULL,
        0,
        0,
    }, // vblank
#endif // SDK_ARM7
};

static u16 OSi_IrqCallbackInfoIndex[OSi_IRQCALLBACK_NUM] = {
    REG_OS_IE_D0_SHIFT,  REG_OS_IE_D1_SHIFT,  REG_OS_IE_D2_SHIFT,
    REG_OS_IE_D3_SHIFT,  REG_OS_IE_ND0_SHIFT, REG_OS_IE_ND1_SHIFT,
    REG_OS_IE_ND2_SHIFT, REG_OS_IE_ND3_SHIFT, REG_OS_IE_T0_SHIFT,
    REG_OS_IE_T1_SHIFT,  REG_OS_IE_T2_SHIFT,  REG_OS_IE_T3_SHIFT,
#ifdef SDK_ARM7
    REG_OS_IE_VB_SHIFT
#endif // SDK_ARM7
};
#endif // SDK_NITRO
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include <nitro/dtcm_end.h>
#endif

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include <nitro/itcm_begin.h>
#endif

void OS_IrqDummy(void) {}

void OSi_IrqCallback(int index) {
  OSIrqMask imask = (1UL << OSi_IrqCallbackInfoIndex[index]);
  void (*callback)(void *) = OSi_IrqCallbackInfo[index].func;

  OSi_IrqCallbackInfo[index].func = NULL;

  if (callback) {
    (callback)(OSi_IrqCallbackInfo[index].arg);
  }

  OS_SetIrqCheckFlag(imask);

  if (!OSi_IrqCallbackInfo[index].enable) {
    (void)OS_DisableIrqMask(imask);
  }
}

void OSi_IrqDma0(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_DMA0); }

void OSi_IrqDma1(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_DMA1); }

void OSi_IrqDma2(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_DMA2); }

void OSi_IrqDma3(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_DMA3); }

void OSi_IrqTimer0(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_TIMER0); }

void OSi_IrqTimer1(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_TIMER1); }

void OSi_IrqTimer2(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_TIMER2); }

void OSi_IrqTimer3(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_TIMER3); }

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include <nitro/itcm_end.h>
#endif

#ifdef SDK_ARM7
void OSi_IrqVBlank(void) {
  void (*callback)(void) =
      (void (*)(void))OSi_IrqCallbackInfo[OSi_IRQCALLBACK_NO_VBLANK].func;

  (*(u32 *)HW_VBLANK_COUNT_BUF)++;

  if (callback) {
    (callback)();
  }

  OS_SetIrqCheckFlag(1UL << REG_OS_IE_VB_SHIFT);
}
#endif

#ifdef SDK_TWL

void OSi_IrqNDma0(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_NDMA0); }

void OSi_IrqNDma1(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_NDMA1); }

void OSi_IrqNDma2(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_NDMA2); }

void OSi_IrqNDma3(void) { OSi_IrqCallback(OSi_IRQCALLBACK_NO_NDMA3); }
#endif
