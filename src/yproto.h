#ifndef YPROTO_H_
#define YPROTO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* use big endian in binary */
#define YPROTO_LITTLE_ENDIAN 0
#define YPROTO_BIG_ENDIAN 1

/* TODO: change to YPROTO_BIG_ENDIAN if run on big endian machine. */
#define YPROTO_LOCAL_ENDIAN YPROTO_LITTLE_ENDIAN

/* TODO: set malloc and free function. */
#include <malloc.h>
#define YPROTO_MALLOC malloc
#define YPROTO_FREE free

typedef signed char i8;
typedef unsigned char u8;
typedef signed short i16;
typedef unsigned short u16;
typedef signed int i32;
typedef unsigned int u32;
typedef signed long long i64;
typedef unsigned long long u64;

u16 decode_u16(const u8 *buf);
u32 decode_u32(const u8 *buf);
u64 decode_u64(const u8 *buf);

#ifdef __cplusplus
}
#endif

#endif /* YPROTO_H_ */

