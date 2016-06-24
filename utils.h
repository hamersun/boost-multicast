#ifndef _MULTICAST_UTILS_H_
#define _MULTICAST_UTILS_H_

inline uint16_t endian_swap(uint16_t& x)
{
    x = (x>>8) |
    (x<<8);
    return x;
}

inline uint32_t endian_swap(uint32_t &x)
{
    x = (x>>24) |
    ((x<<8) & 0x00FF0000) |
    ((x>>8) & 0x0000FF00) |
    (x<<24);
    return x;
}

inline int64_t endian_swap(int64_t &x)
{
    x = (x>>56) |
    ((x<<40) & 0x00FF000000000000) |
    ((x<<24) & 0x0000FF0000000000) |
    ((x<<8)  & 0x000000FF00000000) |
    ((x>>8)  & 0x00000000FF000000) |
    ((x>>24) & 0x0000000000FF0000) |
    ((x>>40) & 0x000000000000FF00) |
    (x<<56);
    return x;
}

#endif // _MULTICAST_UTILS_H_
