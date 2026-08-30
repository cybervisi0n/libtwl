#include <nitro.h>

extern u16 CTRDGi_PollingSR1MCOMMON(u16 phase, u8 *adr, u16 lastData);

extern u16 CTRDGi_EraseFlashChipMX(void);
extern u16 CTRDGi_EraseFlashSectorMX(u16 secNo);
extern u16 CTRDGi_WriteFlashSectorMX(u16 secNo, u8 *src);
extern void CTRDGi_EraseFlashChipAsyncMX(CTRDG_TASK_FUNC callback);
extern void CTRDGi_EraseFlashSectorAsyncMX(u16 secNo, CTRDG_TASK_FUNC callback);
extern void CTRDGi_WriteFlashSectorAsyncMX(u16 secNo, u8 *src,
                                           CTRDG_TASK_FUNC callback);

static const u16 LeMaxTime[] = {
    10,   // Common       10ms
    10,   // Program      10ms(exactly:20usec)
    2000, // Sector erase 2s
    5000, // Chip   erase 5s
};

const CTRDGiFlashTypePlus LE26FV10N1TS_10 = {
    CTRDGi_WriteFlashSectorMX,
    CTRDGi_EraseFlashChipMX,
    CTRDGi_EraseFlashSectorMX,
    CTRDGi_WriteFlashSectorAsyncMX,
    CTRDGi_EraseFlashChipAsyncMX,
    CTRDGi_EraseFlashSectorAsyncMX,
    CTRDGi_PollingSR1MCOMMON,
    LeMaxTime,
    {

        0x00020000,              // ROM size
        {0x00001000, 12, 32, 0}, // Sector size, shift, count, top

        {MI_CTRDG_RAMCYCLE_18,
         MI_CTRDG_RAMCYCLE_8}, // agb read cycle=8, write cycle=3
        0x62,                  // Maker ID
        0x13,                  // Device ID
    },
};
