#ifndef NITRO_OS_ARM9_CHINA_H_
#define NITRO_OS_ARM9_CHINA_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OS_BURY_STRING_FORCHINA "[SDK+NINTENDO:FORCHINA]"
#define OS_BUSY_STRING_LEN_FORCHINA 23

typedef enum {
  OS_CHINA_ISBN_DISP,
  OS_CHINA_ISBN_NO_DISP,
  OS_CHINA_ISBN_CHECK_ROM
} OSChinaIsbn;

void OS_InitChina(const char **isbn, OSChinaIsbn param);

void OS_ShowAttentionChina(const char **isbn, OSChinaIsbn param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_OS_ARM9_CHINA_H_ */
