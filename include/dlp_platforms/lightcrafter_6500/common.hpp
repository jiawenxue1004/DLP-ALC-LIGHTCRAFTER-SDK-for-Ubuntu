#ifndef COMMON_H_
#define COMMON_H_

/**
 * This module provides the common defines used by all the modules
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#ifdef __cplusplus
extern "C" {
#endif

#define TRUE                    1
#define FALSE                   0

typedef enum
{
#define MAKE_FORMAT(b3, b2, b1, b0) (((b3)<<24)|((b2)<<16)|((b1)<<8)|(b0))


	IMAGE_PIX_FORMAT_RGB32  = MAKE_FORMAT('R', 'G', 'B', 32),
	IMAGE_PIX_FORMAT_GREY8  = MAKE_FORMAT('G', 'R', 'E',  8),
	IMAGE_PIX_FORMAT_GREY10 = MAKE_FORMAT('G', 'R', 'E', 10),
	IMAGE_PIX_FORMAT_UYVY16 = MAKE_FORMAT('U', 'Y', 'V', 16),
	IMAGE_PIX_FORMAT_RGB16  = MAKE_FORMAT('R', 'G', 'B', 16),
	IMAGE_PIX_FORMAT_SBGGR  = MAKE_FORMAT('B', 'Y', 'R',  8),
	IMAGE_PIX_FORMAT_RGB24  = MAKE_FORMAT('R', 'G', 'B', 24),
	IMAGE_PIX_FORMAT_RGB565 = MAKE_FORMAT('R', 'G', 'B', 16),
} ImagePixFormat_t;

typedef struct
{
	unsigned char *Buffer;
	unsigned Width;
	unsigned Height;
	unsigned LineWidth;
	ImagePixFormat_t PixFormat;
}Image_t;


typedef int BOOL;
typedef unsigned uint32;
typedef unsigned char uint8;
typedef unsigned short uint16;

#ifndef MIN
    #define MIN(a, b)					((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
    #define MAX(a, b)					((a) > (b) ? (a) : (b))
#endif

#define ALIGN_BYTES_PREV(x, b)      ((x) & ~(uint32)((b) - 1))
#define ALIGN_BYTES_NEXT(x, b)      (((x) + ((b)-1)) & ~(uint32)((b) - 1))


#define BYTE0(x)					(uint8)(x)
#define BYTE1(x)					(uint8)((x) >> 8)
#define BYTE2(x)					(uint8)((x) >> 16)
#define BYTE3(x)					(uint8)((x) >> 24)
#define WORD0(x)					(uint16)(x)
#define WORD1(x)					(uint16)((x) >> 16)

#define PARSE_WORD16_LE(Arr)        MAKE_WORD16((Arr)[1], (Arr)[0])
#define PARSE_WORD16_BE(Arr)        MAKE_WORD16((Arr)[0], (Arr)[1])
#define PARSE_WORD24_LE(Arr)        MAKE_WORD32(0, (Arr)[2], (Arr)[1], (Arr)[0])
#define PARSE_WORD24_BE(Arr)        MAKE_WORD32(0, (Arr)[0], (Arr)[1], (Arr)[2])
#define PARSE_WORD32_LE(Arr)        MAKE_WORD32((Arr)[3], (Arr)[2], (Arr)[1], (Arr)[0])
#define PARSE_WORD32_BE(Arr)        MAKE_WORD32((Arr)[0], (Arr)[1], (Arr)[2], (Arr)[3])

#define MAKE_WORD16(b1, b0)         (((b1) << 8) | (b0))
#define MAKE_WORD32(b3, b2, b1, b0) (((uint32)(b3)<<24)|((uint32)(b2)<<16)|((uint32)(b1)<<8)|(b0))

#define ARRAY_SIZE(x)               (sizeof(x)/sizeof(*x))
#define DIV_ROUND(x, y)             (((x)+(y)/2)/(y))
#define DIV_CEIL(x, y)              (((x)+(y)-1)/(y))
#define POW_OF_2(x)                 (1ul << (x))

#define IS_POW_OF_2(x)              (((x) & ((x)-1)) == 0)

// Generate bit mask of n bits starting from s bit
#define GEN_BIT_MASK(s, n)          (((1ul << (n)) - 1) << (s))

// Merge bits b into a according to mask
#define MERGE_BITS(a, b, mask)      ((a) ^ (((a) ^ (b)) & (mask)))

#define STRUCT_CLEAR(s)				(memset(&(s), 0, sizeof(s)))
#define BIT_REVERSE(x)				CMN_BitRevLUT[(uint8)(x)]

#define STR(x)	STR_(x)
#define STR_(x)	#x

#define IMG_FMT_TO_BPP(Fmt)		((Fmt) & 0x3F)

#ifdef __cplusplus
}
#endif


#endif /* COMMON_H_ */
