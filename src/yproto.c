#include "yproto.h"

u16 decode_u16(const u8 *buf) {
    u16 v;
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)
    u8 *pv = (u8*)&v;
    pv[0] = buf[1];
    pv[1] = buf[0];
#else 
    v = *(u16*)buf;
#endif
    return v;
}

u32 decode_u32(const u8 *buf) {
    u32 v;
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)
    u8 *pv = (u8*)&v;
    pv[0] = buf[3];
    pv[1] = buf[2];
    pv[2] = buf[1];
    pv[3] = buf[0];
#else 
    v = *(u32*)buf;
#endif
    return v;
}

u64 decode_u64(const u8 *buf) {
    u64 v;
    u8 *pv = (u8*)&v;
#if (YPROTO_LOCAL_ENDIAN == YPROTO_LITTLE_ENDIAN)
    pv[0] = buf[7];
    pv[1] = buf[6];
    pv[2] = buf[5];
    pv[3] = buf[4];
    pv[4] = buf[3];
    pv[5] = buf[2];
    pv[6] = buf[1];
    pv[7] = buf[0];
#else 
    v = *(u64*)buf;
#endif
    return v;
}

