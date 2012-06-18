#ifndef BROOV_TYPES_H
#define BROOV_TYPES_H

typedef unsigned long ULONG;
typedef long LONG;

#define BROOV_TRUE                 1
#define BROOV_FALSE                0

#ifndef INT64_MIN
#define INT64_MIN       (-0x7fffffffffffffffLL - 1)
#endif

#ifndef INT64_MAX
#define INT64_MAX INT64_C(9223372036854775807)
#endif

#endif /* #ifndef BROOV_TYPES_H */
