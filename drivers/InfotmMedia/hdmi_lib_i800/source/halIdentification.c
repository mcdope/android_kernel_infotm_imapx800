/*
 * @file:halIdentification.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halIdentification.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 DESIGN_ID = 0x00;
static const IM_UINT8 REVISION_ID = 0x01;
static const IM_UINT8 PRODUCT_ID0 = 0x02;
static const IM_UINT8 PRODUCT_ID1 = 0x03;

IM_UINT8 halIdentification_Design(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte((baseAddr + DESIGN_ID));
}

IM_UINT8 halIdentification_Revision(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte((baseAddr + REVISION_ID));
}

IM_UINT8 halIdentification_ProductLine(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte((baseAddr + PRODUCT_ID0));
}

IM_UINT8 halIdentification_ProductType(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte((baseAddr + PRODUCT_ID1));
}

