#ifndef NITRO_SPI_COMMON_USERINFO_TS_200_H_
#define NITRO_SPI_COMMON_USERINFO_TS_200_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NVRAM_CONFIG_DATA_VERSION 5

#define NVRAM_CONFIG_NICKNAME_LENGTH 10    // Nickname Length
#define NVRAM_CONFIG_COMMENT_LENGTH 26     // Comment Length
#define NVRAM_CONFIG_FAVORITE_COLOR_MAX 16 // Maximum number of favorite colors

#define NVRAM_CONFIG_LANG_JAPANESE 0 // Japanese
#define NVRAM_CONFIG_LANG_ENGLISH 1  // English
#define NVRAM_CONFIG_LANG_FRENCH 2   // French
#define NVRAM_CONFIG_LANG_GERMAN 3   // German
#define NVRAM_CONFIG_LANG_ITALIAN 4  // Italian
#define NVRAM_CONFIG_LANG_SPANISH 5  // Spanish
#define NVRAM_CONFIG_LANG_CODE_MAX                                             \
  6 // Number of types of language setting codes

typedef struct NVRAMConfigDate {
  u8 month; // Month: 01 - 12
  u8 day;   // Day: 01 - 31

} NVRAMConfigDate;

typedef struct NVRAMConfigNickname {
  u16 str[NVRAM_CONFIG_NICKNAME_LENGTH]; // Nickname  (Maximum of 10 characters
                                         // in Unicode (UTF16), no terminal
                                         // code)
  u8 length;                             // Number of characters
  u8 rsv;

} NVRAMConfigNickname;

typedef struct NVRAMConfigComment {
  u16 str[NVRAM_CONFIG_COMMENT_LENGTH]; // Comment (a maximum of 26 characters
                                        // in Unicode (UTF16), no terminal code)
  u8 length; // Number of characters
  u8 rsv;

} NVRAMConfigComment;

typedef struct NVRAMConfigOwnerInfo {
  u8 favoriteColor : 4;     // Favorite color
  u8 rsv : 4;               // Reserved
  NVRAMConfigDate birthday; // Birthdate
  u8 pad;
  NVRAMConfigNickname nickname; // Nickname
  NVRAMConfigComment comment;   // Comment

} NVRAMConfigOwnerInfo;

typedef struct NVRAMConfigAlarm {
  u8 hour;   // Alarm Hour: 00 - 23
  u8 minute; // Alarm Minute: 00 - 59
  u8 second; // Alarm Seconds: 00 - 59
  u8 pad;
  u16 enableWeek : 7; // Flag for which days of the week the alarm is enabled
                      // (bit0: Sunday, bit1: Monday, etc..."1" means "enabled")
  u16 alarmOn : 1;    // Alarm ON/OFF (0: OFF, 1: ON)
  u16 rsv : 8;        // Reserved

} NVRAMConfigAlarm;

typedef struct NVRAMConfigTpCalibData {
  u16 raw_x1; // TP-obtained x value of the first calibration point
  u16 raw_y1; // TP-obtained y value of the first calibration point
  u8 dx1;     // LCD x coordinate of the first calibration point
  u8 dy1;     // LCD y coordinate of the first calibration point
  u16 raw_x2; // TP-obtained x value of the second calibration point
  u16 raw_y2; // TP-obtained y value of the second calibration point
  u8 dx2;     // LCD x coordinate of the second calibration point
  u8 dy2;     // LCD y coordinate of the second calibration point

} NVRAMConfigTpCalibData;

typedef struct NVRAMConfigOption {
  u16 language : 3; // Language code
  u16 agbLcd : 1; // Start up on which LCD when booting in AGB mode? (0: TOP, 1:
                  // BOTTOM)
  u16 detectPullOutCardFlag
      : 1; // Flag that indicates that the card has been pulled out
  u16 detectPullOutCtrdgFlag
      : 1; // Flag that indicates that the Game Pak has been pulled out
  u16 autoBootFlag : 1; // Whether or not to automatically start up without
                        // stopping at the menu in the startup sequence.
  u16 rsv : 4;          // Reserved
  u16 input_favoriteColor : 1; // Was a favorite color input?
  u16 input_tp : 1;            // Has the Touch Screen been calibrated? ( " )
  u16 input_language
      : 1; // Was a language input?         (0: Not configured, 1: Configured)
  u16 input_rtc : 1;      // Was the RTC configured?          (       "       )
  u16 input_nickname : 1; // Was a nickname input? (       "        )
  u8 timezone;            // Time Zone (currently reserved)
  u8 rtcClockAdjust;      // RTC Clock adjustment value
  s64 rtcOffset; // Offset value when configuring the RTC (each time the user
                 // changes the configuration of the RTC, this fluctuates in
                 // response to that value)

} NVRAMConfigOption;

typedef struct NVRAMConfigData {
  u8 version; // Flash storage data format version
  u8 pad;
  NVRAMConfigOwnerInfo owner; // Owner Information
  NVRAMConfigAlarm alarm;     // Alarm clock data for IPL
  NVRAMConfigTpCalibData tp;  // Touch Panel Calibration Data
  NVRAMConfigOption option;   // Options

} NVRAMConfigData;

typedef struct NVRAMConfig {
  NVRAMConfigData ncd; // Various types of configuration data
  u16 saveCount; // Counts 0x00-0x7f by looping; valid if the count is new data
  u16 crc16;     // 16-bit CRC of various types of configuration data

} NVRAMConfig;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SPI_COMMON_USERINFO_TS_200_H_ */
