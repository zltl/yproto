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

int decodeU16(const u8 *buf, void *v);
int decodeU32(const u8 *buf, void *v);
int decodeU64(const u8 *buf, void *v);
int encodeU16(u8 *buf, u16 v);
int encodeU32(u8 *buf, u32 v);
int encodeU64(u8 *buf, u64 v);

#define __DEF_YPROTO_VECTOR(lt) struct vector##lt { lt length; void *value; }
__DEF_YPROTO_VECTOR(u8);
__DEF_YPROTO_VECTOR(i8);
__DEF_YPROTO_VECTOR(u16);
__DEF_YPROTO_VECTOR(i16);
__DEF_YPROTO_VECTOR(u32);
__DEF_YPROTO_VECTOR(i32);
__DEF_YPROTO_VECTOR(u64);
__DEF_YPROTO_VECTOR(i64);

#ifdef __cplusplus
}
#endif

#endif /* YPROTO_H_ */

