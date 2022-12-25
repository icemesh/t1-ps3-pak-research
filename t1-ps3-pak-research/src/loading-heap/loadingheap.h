/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/
#pragma once

#include <cinttypes>

namespace LoadingHeap
{
	//every page in the pak has this size limit
	static const uint32_t s_PageSize = 0x80000; //512*1024
	
	//it probably changed between version..
	static const uint32_t kBigPs3MaxPages = 0xAC; //172
};
