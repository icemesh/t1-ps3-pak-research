/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/
#include "pmcollection.h"
#include "../utils/utils.h"

#include <stdio.h>

void PmCollection::DumpInfo(uint8_t* pMem)
{
	PmCollection2*  pCollection = reinterpret_cast<PmCollection2*>(pMem);
	if (pCollection)
	{
		uint32_t numPmCollections = swapU32(&pCollection->m_numPmCollections);
		PmCollectionEntry** pArray = &pCollection->m_apEntries;
		for (uint32_t iCollection = 0; iCollection < numPmCollections; iCollection++)
		{
			PmCollectionEntry* pEntry = pArray[iCollection];
			printf("ProtoName: %s\n", pEntry->m_pPrototype->m_szProtoName->m_name);
		}
		
	}
}
