#ifndef __JJAT2__UTIL_H__
#define __JJAT2__UTIL_H__

/* Copy the string into the current input mapping */
err setInputStr(const char *str, int len);

#define staticSetInputStr(__STR__) \
    setInputStr(__STR__, sizeof(__STR__) - 1)

#endif /* __JJAT2__UTIL_H__ */
