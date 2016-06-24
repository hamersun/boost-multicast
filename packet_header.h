#ifndef _MULTICAST_PACKET_HEADER_
#define _MULTICAST_PACKET_HEADER_

typedef struct _packet_header {
    int64_t SN; // big endian
    uint8_t segment_count; // big endian
    uint8_t order; // big endian
} packet_header;

#endif // _MULTICAST_PACKET_HEADER_
