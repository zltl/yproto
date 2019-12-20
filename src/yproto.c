#include "yproto.h"

int decodeU16(const u8 *buf, void *v) {
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)
    u8 *pv = (u8*)v;
    pv[0] = buf[1];
    pv[1] = buf[0];
#else 
    *(u16*)v = *(u16*)buf;
#endif
    return 2;
}

int decodeU32(const u8 *buf, void *v) {
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)
    u8 *pv = (u8*)v;
    pv[0] = buf[3];
    pv[1] = buf[2];
    pv[2] = buf[1];
    pv[3] = buf[0];
#else 
    *(u32*)v = *(u32*)buf;
#endif
    return 4;
}

int decodeU64(const u8 *buf, void *v) {
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)
    u8 *pv = (u8*)v;
    pv[0] = buf[7];
    pv[1] = buf[6];
    pv[2] = buf[5];
    pv[3] = buf[4];
    pv[4] = buf[3];
    pv[5] = buf[2];
    pv[6] = buf[1];
    pv[7] = buf[0];
#else 
    *(u64*)v = *(u64*)buf;
#endif
    return 8;
}

int encodeU16(u8 *buf, u16 v) {
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)    
    u8 *pv = (u8*)&v;
    buf[0] = pv[1];
    buf[1] = pv[0];
#else 
    *((u16*)buf) = v;
#endif
    return 2;
}

int encodeU32(u8 *buf, u32 v) {
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)    
    u8 *pv = (u8*)&v;
    buf[0] = pv[3];
    buf[1] = pv[2];
    buf[2] = pv[1];
    buf[3] = pv[0];
#else 
    *((u32*)buf) = v;
#endif
    return 4;
}

int encodeU64(u8 *buf, u64 v) {
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)    
    u8 *pv = (u8*)&v;
    buf[0] = pv[7];
    buf[1] = pv[6];
    buf[2] = pv[5];
    buf[3] = pv[4];
    buf[4] = pv[3];
    buf[5] = pv[2];
    buf[6] = pv[1];
    buf[7] = pv[0];
#else 
    *((u64*)buf) = v;
#endif
    return 8;
}

