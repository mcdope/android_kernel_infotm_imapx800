/***************************************************************************** 
 * ** 
 * ** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** v1.0.1	karst@2012/08/16: first commit.
 * **
 * *****************************************************************************/ 

#ifndef __IM_DBGTOOLAPI_H__
#define __IM_DBGTOOLAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef DBGTOOLAPI_EXPORTS
		#define DBT_API		__declspec(dllexport)	/* For dll lib */
#else
		#define DBT_API		__declspec(dllimport)	/* For application */
#endif
#else
		#define DBT_API
#endif	
/*############################################################################*/
//

DBT_API IM_UINT32 dbt_version(OUT IM_TCHAR *ver_string);

DBT_API IM_RET dbt_reg_write(IN IM_UINT32 addr, IN IM_UINT32 val);
DBT_API IM_RET dbt_reg_read(IN IM_UINT32 addr, IN IM_INT32 num, OUT IM_UINT32 *vals);

DBT_API IM_RET dbt_mem_write(IN IM_UINT32 addr, IN IM_INT32 num, IN IM_UINT32 *vals);
DBT_API IM_RET dbt_mem_read(IN IM_UINT32 addr, IN IM_INT32 num, OUT IM_UINT32 *vals);

/*############################################################################*/
#ifdef __cplusplus
}
#endif

#endif	// __IM_DBGTOOLAPI_H__

