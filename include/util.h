#ifndef _UTIL_H
#define _UTIL_H

#define INLINE __attribute__((always_inline)) inline

#define min(a, b) ((a) < (b) ? (a) : (b))

#define _str(s) #s
#define str(s) _str(s)

#endif /* _UTIL_H */
