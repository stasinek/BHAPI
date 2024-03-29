#ifndef LENDIAN_BITFIELD_H
#define LENDIAN_BITFIELD_H

#include <kits/netByteOrder.h>

#if B_HOST_IS_BENDIAN

#define  LBITFIELD8_2(b1,b2)					uint8 b2,b1
#define  LBITFIELD8_3(b1,b2,b3)					uint8 b3,b2,b1
#define  LBITFIELD8_4(b1,b2,b3,b4)				uint8 b4,b3,b2,b1
#define  LBITFIELD8_5(b1,b2,b3,b4,b5)			uint8 b5,b4,b3,b2,b1
#define  LBITFIELD8_6(b1,b2,b3,b4,b5,b6)		uint8 b6,b5,b4,b3,b2,b1
#define  LBITFIELD8_7(b1,b2,b3,b4,b5,b6,b7)		uint8 b7,b6,b5,b4,b3,b2,b1
#define  LBITFIELD8_8(b1,b2,b3,b4,b5,b6,b7,b8)	uint8 b8,b7,b6,b5,b4,b3,b2,b1


#define  LBITFIELD2(b1,b2)													uint16 b2,b1
#define  LBITFIELD3(b1,b2,b3)												uint16 b3,b2,b1
#define  LBITFIELD4(b1,b2,b3,b4)											uint16 b4,b3,b2,b1
#define  LBITFIELD5(b1,b2,b3,b4,b5)											uint16 b5,b4,b3,b2,b1
#define  LBITFIELD6(b1,b2,b3,b4,b5,b6)										uint16 b6,b5,b4,b3,b2,b1
#define  LBITFIELD7(b1,b2,b3,b4,b5,b6,b7)							 		uint16 b7,b6,b5,b4,b3,b2,b1
#define  LBITFIELD8(b1,b2,b3,b4,b5,b6,b7,b8)								uint16 b8,b7,b6,b5,b4,b3,b2,b1
#define  LBITFIELD9(b1,b2,b3,b4,b5,b6,b7,b8,b9)								uint16 b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD10(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10)							uint16 b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD11(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11)						uint16 b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD12(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12)					uint16 b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD13(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13)				uint16 b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD14(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14)			uint16 b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD15(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15)		uint16 b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD16(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16)	uint16 b16,b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1

#define  LBITFIELD32_2(b1,b2)													uint32_t b2,b1
#define  LBITFIELD32_3(b1,b2,b3)												uint32_t b3,b2,b1
#define  LBITFIELD32_4(b1,b2,b3,b4)												uint32_t b4,b3,b2,b1
#define  LBITFIELD32_5(b1,b2,b3,b4,b5)											uint32_t b5,b4,b3,b2,b1
#define  LBITFIELD32_6(b1,b2,b3,b4,b5,b6)										uint32_t b6,b5,b4,b3,b2,b1
#define  LBITFIELD32_7(b1,b2,b3,b4,b5,b6,b7)							 		uint32_t b7,b6,b5,b4,b3,b2,b1
#define  LBITFIELD32_8(b1,b2,b3,b4,b5,b6,b7,b8)									uint32_t b8,b7,b6,b5,b4,b3,b2,b1
#define  LBITFIELD32_9(b1,b2,b3,b4,b5,b6,b7,b8,b9)								uint32_t b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD32_10(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10)							uint32_t b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD32_11(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11)						uint32_t b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD32_12(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12)					uint32_t b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD32_13(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13)				uint32_t b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD32_14(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14)			uint32_t b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD32_15(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15)		uint32_t b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1
#define LBITFIELD32_16(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16)	uint32_t b16,b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1

#elif B_HOST_IS_LENDIAN

#define  LBITFIELD8_2(b1,b2)					uint8 b1,b2
#define  LBITFIELD8_3(b1,b2,b3)					uint8 b1,b2,b3
#define  LBITFIELD8_4(b1,b2,b3,b4)				uint8 b1,b2,b3,b4
#define  LBITFIELD8_5(b1,b2,b3,b4,b5)			uint8 b1,b2,b3,b4,b5
#define  LBITFIELD8_6(b1,b2,b3,b4,b5,b6)		uint8 b1,b2,b3,b4,b5,b6
#define  LBITFIELD8_7(b1,b2,b3,b4,b5,b6,b7)		uint8 b1,b2,b3,b4,b5,b6,b7
#define  LBITFIELD8_8(b1,b2,b3,b4,b5,b6,b7,b8)	uint8 b1,b2,b3,b4,b5,b6,b7,b8

#define  LBITFIELD2(b1,b2)													uint16 b1,b2
#define  LBITFIELD3(b1,b2,b3)												uint16 b1,b2,b3
#define  LBITFIELD4(b1,b2,b3,b4)											uint16 b1,b2,b3,b4
#define  LBITFIELD5(b1,b2,b3,b4,b5)											uint16 b1,b2,b3,b4,b5
#define  LBITFIELD6(b1,b2,b3,b4,b5,b6)										uint16 b1,b2,b3,b4,b5,b6
#define  LBITFIELD7(b1,b2,b3,b4,b5,b6,b7)									uint16 b1,b2,b3,b4,b5,b6,b7
#define  LBITFIELD8(b1,b2,b3,b4,b5,b6,b7,b8)								uint16 b1,b2,b3,b4,b5,b6,b7,b8
#define  LBITFIELD9(b1,b2,b3,b4,b5,b6,b7,b8,b9) 							uint16 b1,b2,b3,b4,b5,b6,b7,b8,b9
#define LBITFIELD10(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10)							uint16 b1,b2,b3,b4,b5,b6,b7,b8,b9,b10
#define LBITFIELD11(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11)						uint16 b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11
#define LBITFIELD12(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12)					uint16 b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12
#define LBITFIELD13(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13)				uint16 b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13
#define LBITFIELD14(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14)			uint16 b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14
#define LBITFIELD15(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15)		uint16 b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15
#define LBITFIELD16(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16)	uint16 b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16

#define  LBITFIELD32_2(b1,b2)													uint32_t b1,b2
#define  LBITFIELD32_3(b1,b2,b3)												uint32_t b1,b2,b3
#define  LBITFIELD32_4(b1,b2,b3,b4)												uint32_t b1,b2,b3,b4
#define  LBITFIELD32_5(b1,b2,b3,b4,b5)											uint32_t b1,b2,b3,b4,b5
#define  LBITFIELD32_6(b1,b2,b3,b4,b5,b6)										uint32_t b1,b2,b3,b4,b5,b6
#define  LBITFIELD32_7(b1,b2,b3,b4,b5,b6,b7)									uint32_t b1,b2,b3,b4,b5,b6,b7
#define  LBITFIELD32_8(b1,b2,b3,b4,b5,b6,b7,b8)									uint32_t b1,b2,b3,b4,b5,b6,b7,b8
#define  LBITFIELD32_9(b1,b2,b3,b4,b5,b6,b7,b8,b9) 								uint32_t b1,b2,b3,b4,b5,b6,b7,b8,b9
#define LBITFIELD32_10(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10)							uint32_t b1,b2,b3,b4,b5,b6,b7,b8,b9,b10
#define LBITFIELD32_11(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11)						uint32_t b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11
#define LBITFIELD32_12(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12)					uint32_t b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12
#define LBITFIELD32_13(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13)				uint32_t b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13
#define LBITFIELD32_14(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14)			uint32_t b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14
#define LBITFIELD32_15(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15)		uint32_t b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15
#define LBITFIELD32_16(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16)	uint32_t b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15,b16

#else
#error "Unknown host endianness"
#endif

#endif /* LENDIAN_BITFIELD_H */
