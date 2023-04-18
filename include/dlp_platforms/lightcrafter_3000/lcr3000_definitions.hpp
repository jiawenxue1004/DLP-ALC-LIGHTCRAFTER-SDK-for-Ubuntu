#ifndef LCR3000_DEFINITIONS_H
#define LCR3000_DEFINITIONS_H

#define LCR_CMD_VERSION_STR_LEN		32
#define LCR_CMD_SOLUTION_NAME_LEN	32

#define	ONE_BPP_PTN_SIZE		52046
#define	TWO_BPP_PTN_SIZE		208006
#define	THREE_BPP_PTN_SIZE		208022
#define	FOUR_BPP_PTN_SIZE		208054
#define	FIVE_BPP_PTN_SIZE		416054
#define	SIX_BPP_PTN_SIZE		416182
#define	SEVEN_BPP_PTN_SIZE		416438
#define	EIGHT_BPP_PTN_SIZE		416950

#define HEADER_SIZE             6
#define MAX_PACKET_SIZE         0xFFFF
#define CHECKSUM_SIZE           1

typedef int BOOL;
typedef unsigned uint32;
typedef unsigned char uint8;
typedef unsigned short uint16;

#endif // LCR3000_DEFINITIONS_H
