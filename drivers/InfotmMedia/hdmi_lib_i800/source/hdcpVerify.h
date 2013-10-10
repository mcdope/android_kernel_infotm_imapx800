/*
 * hdcpVerify.h
 *
 *  Created on: Jul 20, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HDCPVERIFY_H_
#define HDCPVERIFY_H_

#include "types.h"

typedef struct
{
	IM_UINT8 mLength[8];
	IM_UINT8 mBlock[64];
	IM_INT32 mIndex;
	IM_INT32 mComputed;
	IM_INT32 mCorrupted;
	IM_UINT32 mDigest[5];
} sha_t;

static const IM_UINT32 KSV_LEN = 5;
static const IM_UINT32 KSV_MSK = 0x7F;
static const IM_UINT32 VRL_LENGTH = 0x05;
static const IM_UINT32 VRL_HEADER = 5;
static const IM_UINT32 VRL_NUMBER = 3;
static const IM_UINT32 HEADER = 10;
static const IM_UINT32 SHAMAX = 20;
static const IM_UINT32 DSAMAX = 20;
static const IM_UINT32 INT_KSV_SHA1 = 1;

void sha_Reset(sha_t *sha);

IM_INT32 sha_Result(sha_t *sha);

void sha_Input(sha_t *sha, const IM_UINT8 * data, size_t size);

void sha_ProcessBlock(sha_t *sha);

void sha_PadMessage(sha_t *sha);

IM_INT32 hdcpVerify_DSA(const IM_UINT8 * M, size_t n, const IM_UINT8 * r, const IM_UINT8 * s);

IM_INT32 hdcpVerify_ArrayADD(IM_UINT8 * r, const IM_UINT8 * a, const IM_UINT8 * b, size_t n);

IM_INT32 hdcpVerify_ArrayCMP(const IM_UINT8 * a, const IM_UINT8 * b, size_t n);

void hdcpVerify_ArrayCPY(IM_UINT8 * dst, const IM_UINT8 * src, size_t n);

IM_INT32 hdcpVerify_ArrayDIV(IM_UINT8 * r, const IM_UINT8 * D, const IM_UINT8 * d, size_t n);

IM_INT32 hdcpVerify_ArrayMAC(IM_UINT8 * r, const IM_UINT8 * M, const IM_UINT8 m, size_t n);

IM_INT32 hdcpVerify_ArrayMUL(IM_UINT8 * r, const IM_UINT8 * M, const IM_UINT8 * m, size_t n);

void hdcpVerify_ArraySET(IM_UINT8 * dst, const IM_UINT8 src, size_t n);

IM_INT32 hdcpVerify_ArraySUB(IM_UINT8 * r, const IM_UINT8 * a, const IM_UINT8 * b, size_t n);

void hdcpVerify_ArraySWP(IM_UINT8 * r, size_t n);

IM_INT32 hdcpVerify_ArrayTST(const IM_UINT8 * a, const IM_UINT8 b, size_t n);

IM_INT32 hdcpVerify_ComputeEXP(IM_UINT8 * c, const IM_UINT8 * M, const IM_UINT8 * e, const IM_UINT8 * p,
		size_t n, size_t nE);

IM_INT32 hdcpVerify_ComputeINV(IM_UINT8 * out, const IM_UINT8 * z, const IM_UINT8 * a, size_t n);

IM_INT32 hdcpVerify_ComputeMOD(IM_UINT8 * dst, const IM_UINT8 * src, const IM_UINT8 * p, size_t n);

IM_INT32 hdcpVerify_ComputeMUL(IM_UINT8 * p, const IM_UINT8 * a, const IM_UINT8 * b, const IM_UINT8 * m,
		size_t n);

IM_INT32 hdcpVerify_KSV(const IM_UINT8 * data, size_t size);

IM_INT32 hdcpVerify_SRM(const IM_UINT8 * data, size_t size);

#endif /* HDCPVERIFY_H_ */
