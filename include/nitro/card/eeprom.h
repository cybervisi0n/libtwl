#ifndef NITRO_CARD_EEPROM_H_
#define NITRO_CARD_EEPROM_H_

#include <nitro/card/backup.h>

#ifdef __cplusplus
extern "C" {
#endif

SDK_INLINE BOOL CARD_IsBackupEeprom(void) {
  const CARDBackupType t = CARD_GetCurrentBackupType();
  return (((t >> CARD_BACKUP_TYPE_DEVICE_SHIFT) &
           CARD_BACKUP_TYPE_DEVICE_MASK) == CARD_BACKUP_TYPE_DEVICE_EEPROM);
}

SDK_INLINE BOOL CARD_ReadEeprom(u32 src, void *dst, u32 len) {
  return CARD_ReadBackup(src, dst, len);
}

SDK_INLINE void CARD_ReadEepromAsync(u32 src, void *dst, u32 len,
                                     MIDmaCallback callback, void *arg) {
  CARD_ReadBackupAsync(src, dst, len, callback, arg);
}

SDK_INLINE BOOL CARD_WriteEeprom(u32 dst, const void *src, u32 len) {
  return CARD_ProgramBackup(dst, src, len);
}

SDK_INLINE void CARD_WriteEepromAsync(u32 dst, const void *src, u32 len,
                                      MIDmaCallback callback, void *arg) {
  CARD_ProgramBackupAsync(dst, src, len, callback, arg);
}

SDK_INLINE BOOL CARD_VerifyEeprom(u32 dst, const void *src, u32 len) {
  return CARD_VerifyBackup(dst, src, len);
}

SDK_INLINE void CARD_VerifyEepromAsync(u32 dst, const void *src, u32 len,
                                       MIDmaCallback callback, void *arg) {
  CARD_VerifyBackupAsync(dst, src, len, callback, arg);
}

SDK_INLINE BOOL CARD_WriteAndVerifyEeprom(u32 dst, const void *src, u32 len) {
  return CARD_ProgramAndVerifyBackup(dst, src, len);
}

SDK_INLINE void CARD_WriteAndVerifyEepromAsync(u32 dst, const void *src,
                                               u32 len, MIDmaCallback callback,
                                               void *arg) {
  CARD_ProgramAndVerifyBackupAsync(dst, src, len, callback, arg);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NITRO_CARD_EEPROM_H_
