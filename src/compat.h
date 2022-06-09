#ifndef __COMPAT_H_
#define __COMPAT_H_

#define size_t unsigned int

int memcmp (const void *, const void *, size_t);
char *strchr(const char *, int);

#endif // __COMPAT_H_
