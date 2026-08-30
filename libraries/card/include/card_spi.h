#ifndef NITRO_LIBRARIES_CARD_SPI_H__
#define NITRO_LIBRARIES_CARD_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CSPI_CONTINUOUS_ON 0x0040
#define CSPI_CONTINUOUS_OFF 0x0000

#define COMM_WRITE_STATUS 0x01  /* Write status */
#define COMM_PROGRAM_PAGE 0x02  /* Page program */
#define COMM_READ_ARRAY 0x03    /* Read */
#define COMM_WRITE_DISABLE 0x04 /* Disable writing (not used) */
#define COMM_READ_STATUS 0x05   /* Read status */
#define COMM_WRITE_ENABLE 0x06  /* Enable writing */

#define COMM_PAGE_WRITE 0x0A
#define COMM_PAGE_ERASE 0xDB
#define COMM_SECTOR_ERASE 0xD8
#define COMM_SUBSECTOR_ERASE 0x20
#define COMM_CHIP_ERASE 0xC7
#define CARDFLASH_READ_BYTES_FAST 0x0B /* Unused */
#define CARDFLASH_DEEP_POWDOWN 0xB9    /* Unused */
#define CARDFLASH_WAKEUP 0xAB          /* Unused */

#define COMM_STATUS_WIP_BIT 0x01 /* WriteInProgress (bufy) */
#define COMM_STATUS_WEL_BIT 0x02 /* WriteEnableLatch */
#define COMM_STATUS_BP0_BIT 0x04
#define COMM_STATUS_BP1_BIT 0x08
#define COMM_STATUS_BP2_BIT 0x10
#define COMM_STATUS_WPBEN_BIT 0x80

void CARDi_InitStatusRegister(void);

u8 CARDi_CommandReadStatus(void);

void CARDi_ReadBackupCore(u32 src, void *dst, u32 len);

void CARDi_ProgramBackupCore(u32 dst, const void *src, u32 len);

void CARDi_WriteBackupCore(u32 dst, const void *src, u32 len);

void CARDi_VerifyBackupCore(u32 dst, const void *src, u32 len);

void CARDi_EraseBackupSectorCore(u32 dst, u32 len);

void CARDi_EraseBackupSubSectorCore(u32 dst, u32 len);

void CARDi_EraseChipCore(void);

void CARDi_SetWriteProtectCore(u16 stat);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NITRO_LIBRARIES_CARD_SPI_H__
