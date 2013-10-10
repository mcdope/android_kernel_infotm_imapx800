/*
 * shortVideoDesc.c
 *
 *  Created on: Jul 22, 2010
 *      Author: klabadi & dlopo
 */

#include "shortVideoDesc.h"
#include "bitOperation.h"
#include "hdmi_log.h"

void shortVideoDesc_Reset(shortVideoDesc_t *svd)
{
	svd->mNative = FALSE;
	svd->mCode = 0;
}

IM_INT32 shortVideoDesc_Parse(shortVideoDesc_t *svd, IM_UINT8 data)
{
	shortVideoDesc_Reset(svd);
	svd->mNative = (bitOperation_BitField(data, 7, 1) == 1) ? TRUE : FALSE;
	svd->mCode = bitOperation_BitField(data, 0, 7);
	return TRUE;
}

IM_UINT32 shortVideoDesc_GetCode(shortVideoDesc_t *svd)
{
	return svd->mCode;
}

IM_INT32 shortVideoDesc_GetNative(shortVideoDesc_t *svd)
{
	return svd->mNative;
}
