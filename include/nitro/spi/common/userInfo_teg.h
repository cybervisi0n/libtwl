#ifndef NITRO_SPI_COMMON_USERINFO_TEG_H_
#define NITRO_SPI_COMMON_USERINFO_TEG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NVRAM_CONFIG_DATA_VERSION 0

#define NVRAM_CONFIG_BOOT_GAME_LOG_NUM 8 // Number of saved boot game logs
#define NVRAM_CONFIG_NICKNAME_LENGTH 8   // Nickname Length

#define NVRAM_CONFIG_LANG_JAPANESE 0 // Japanese
#define NVRAM_CONFIG_LANG_ENGLISH 1  // English
#define NVRAM_CONFIG_LANG_FRENCH 2   // French
#define NVRAM_CONFIG_LANG_GERMAN 3   // German
#define NVRAM_CONFIG_LANG_ITALIAN 4  // Italian
#define NVRAM_CONFIG_LANG_SPANISH 5  // Spanish
#define NVRAM_CONFIG_LANG_CODE_MAX                                             \
  6 // Number of types of language setting codes

typedef struct NVRAMConfigNickname {
  u16 name[NVRAM_CONFIG_NICKNAME_LENGTH]; // Nickname (Maximum of 8 characters
                                          // in UNICODE, no terminating code)
  u16 length;                             // Number of characters
  u16 padding;

} NVRAMConfigNickname;

typedef enum NVRAMConfigSexCode {
  NVRAM_CONFIG_SEX_MALE = 0, // Male
  NVRAM_CONFIG_SEX_FEMALE,   // Female
  NVRAM_CONFIG_SEX_CODE_MAX
} NVRAMConfigSexCode;

typedef enum NVRAMConfigBloodType {
  NVRAM_CONFIG_BLOOD_A = 0, // Type A
  NVRAM_CONFIG_BLOOD_B,     // Type B
  NVRAM_CONFIG_BLOOD_AB,    // Type AB
  NVRAM_CONFIG_BLOOD_O,     // Type O
  NVRAM_CONFIG_BLOOD_TYPE_MAX
} NVRAMConfigBloodType;

typedef struct NVRAMConfigDate {
  u16 year; // Year: 1800 - 2100
  u8 month; // Month: 01 - 12
  u8 day;   // Day: 01 - 31

} NVRAMConfigDate;

typedef struct NVRAMConfigOwnerInfo {
  NVRAMConfigNickname nickname;   // Nickname
  NVRAMConfigSexCode sex;         // Gender
  NVRAMConfigBloodType bloodType; // Blood type
  NVRAMConfigDate birthday;       // Birthdate

} NVRAMConfigOwnerInfo;

typedef struct NVRAMConfigBootGameLog {
  u32 gameCode[NVRAM_CONFIG_BOOT_GAME_LOG_NUM]; // List of game abbreviation
                                                // codes for the games started
                                                // up in the past (Ring Buffer)
  u16 top;                                      // Index number of the last log
  u16 num;                                      // Number of logs

} NVRAMConfigBootGameLog; // 36 bytes

typedef struct NVRAMConfigTpCData {
  u16 calib_data[6]; // Calibration Information

} NVRAMConfigTpCData;

typedef struct NVRAMConfigData {
  u8 version;    // Flash storage data format version
  u8 timezone;   // Time zone
  u16 agbLcd;    // Start up on which LCD when booting in AGB mode?
                 // (0:TOP,1:BOTTOM)
  u32 rtcOffset; // Offset value when configuring the RTC (each time the user
                 // changes the configuration of the RTC, this fluctuates in
                 // response to that value)
  u32 language;  // Language code
  NVRAMConfigOwnerInfo owner;         // Owner Information
  NVRAMConfigTpCData tp;              // Touch Panel Calibration Data
  NVRAMConfigBootGameLog bootGameLog; // Boot Game Log

} NVRAMConfigData;

typedef struct NVRAMConfig {
  NVRAMConfigData ncd; // Various types of configuration data
  u16 saveCount; // Counts 0x00-0x7f by looping; valid if the count is new data
  u16 crc16;     // 16-bit CRC of various types of configuration data

} NVRAMConfig;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SPI_COMMON_USERINFO_TEG_H_ */
