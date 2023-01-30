/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/
#include "package.h"
#include "../utils/utils.h"
#include "package-def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/stringid.h"
#include "../pak-entries/debug-info.h"
#include "../pak-entries/spawner-group.h"
#include "../pak-entries/pmcollection.h"

uintptr_t g_packageOffset;

Package::Package(const char* pakName)
{
	m_pLoadedFile = nullptr;
	m_currentPageIdx = 0;
	m_status = PackageStatus::kPackageStatusEmpty;
	m_textureBaseOffset = 0;
	FILE* fh;
	fopen_s(&fh, pakName, "rb");
	if (fh)
	{
		fseek(fh, 0x0, SEEK_END);
		size_t fsize = ftell(fh);
		m_fileSize = fsize;
		fseek(fh, 0x0, SEEK_SET);
		void* pFile = malloc(fsize);
		if (pFile)
		{
			fread(pFile, fsize, 0x1, fh);
			fclose(fh);
			m_pLoadedFile = pFile;
			g_packageOffset = reinterpret_cast<uintptr_t>(pFile);
			snprintf(m_realFileName, 128, "%s", pakName);
		}
		else
		{
			MsgErr("Failed to allocate file");
			fclose(fh);
		}
	}
	else
	{
		MsgErr("Could not open %s! - Does it even exist?\n", pakName);
	}
}

Package::~Package()
{
	if (m_pLoadedFile != nullptr)
	{
		free(m_pLoadedFile);
	}
}

int Package::PackageLogin()
{
	//m_pageSize <= 512 * 1024;
	PakHeader* pHdr = reinterpret_cast<PakHeader*>(m_pLoadedFile);
	if (pHdr)
	{
		m_status = PackageStatus::kPackageStatusLoadingPakHeader;
		while (1)
		{
			switch (m_status)
			{
				case PackageStatus::kPackageStatusLoadingPakHeader:
				{
					uint32_t magic = swapU32(&pHdr->m_magic);
					if (magic == 0xA79)
					{
						if (swapU32(&pHdr->m_pageCt) < LoadingHeap::kBigPs3MaxPages)
						{
							m_hdr.m_magic = magic;
							m_hdr.m_pageCt = swapU32(&pHdr->m_pageCt); //m_loadingHeapNumPages
							m_hdr.m_hdrSize = swapU32(&pHdr->m_hdrSize);
							m_hdr.m_pakLoginTableIdx = swapU32(&pHdr->m_pakLoginTableIdx);
							m_hdr.m_pakLoginTableOffset = swapU32(&pHdr->m_pakLoginTableOffset);
							m_hdr.m_numPointerFixUpPages = swapU32(&pHdr->m_numPointerFixUpPages);
							m_hdr.m_pointerFixUpTableOffset = swapU32(&pHdr->m_pointerFixUpTableOffset);
							PakPageEntry* pCurrentPage = reinterpret_cast<PakPageEntry*>(reinterpret_cast<uint8_t*>(m_pLoadedFile) + 0x2C);
							for (uint32_t i = 0; i < m_hdr.m_pageCt; i++)
							{
								m_pageHdrs[i] = swapU32(&pCurrentPage->m_resPageOffset);
#ifdef _DEBUG
								printf("m_pageHdrs[%i]: 0x%08X\n", i, m_pageHdrs[i]);
#endif
								pCurrentPage++;
							}
							//m_textureBaseOffset = m_hdr.m_hdrSize + (m_hdr.m_pageCt << 19); //m_hdr.m_pageCt*LoadingHeap::s_PageSize
							m_textureBaseOffset = m_fileSize - pHdr->m_dataSize;
							m_status = PackageStatus::kPackageStatusLoadingPages;
						}
						else
						{
							MsgErr("Level [%s] contains too many pages!", m_realFileName);
							return 0;
						}
					}
					else
					{
						MsgErr("Level [%s] is an older version, found 0x%08x instead of 0x%08x!", m_realFileName, magic, 0xA79);
						return 0;
					}
					break;
				}

				case PackageStatus::kPackageStatusLoadingPages:
				{
					printf("Loading RAM pages for %s\n", m_realFileName);
					ResPage* pCurrentPageHdr = reinterpret_cast<ResPage*>(reinterpret_cast<uint8_t*>(m_pLoadedFile) + m_pageHdrs[m_currentPageIdx]);
					uint32_t numEntries = swapU16(&pCurrentPageHdr->m_numPageHeaderEntries);
					if (numEntries != 0)
					{
						//start login items
						uint32_t counter = 0;
						ResPageEntry* pPageEntry = reinterpret_cast<ResPageEntry*>(reinterpret_cast<uint8_t*>(pCurrentPageHdr) + 0x10);
						do {
							//ld        r0, 0(r10) 
							//add       r0, r8, r0    
							uintptr_t resPageEntryName = reinterpret_cast<uintptr_t>(swapU64(pPageEntry->m_name) + reinterpret_cast<uint8_t*>(pCurrentPageHdr));
							//stw       r0, 0(r10)
							pPageEntry->m_name = resPageEntryName;
							//lwz       r11, 8(r10)
							//add       r11, r11, r8
							ResItem* pResItem = reinterpret_cast<ResItem*>(swapU32(&pPageEntry->m_resItemOffset) + reinterpret_cast<uint8_t*>(pCurrentPageHdr));
							//ld        r0, 0(r11) 
							//stw       r0, 0(r11)   
							const char* resItemName = reinterpret_cast<const char*>(swapU64(pResItem->m_itemNameOffset) + reinterpret_cast<uint8_t*>(pCurrentPageHdr) );
							pResItem->m_itemNameOffset = reinterpret_cast<uintptr_t>(resItemName);
							//ld        r9, 8(r11)
							//stw       r9, 8(r11) 
							const char* resItemType = reinterpret_cast<const char*>(swapU64(pResItem->m_itemTypeOffset)+ reinterpret_cast<uint8_t*>(pCurrentPageHdr));
							pResItem->m_itemTypeOffset = reinterpret_cast<uintptr_t>(resItemType);
							printf("Logged in item \"%s\" of type \"%s\"\n", resItemName, resItemType);
							counter++;
							pPageEntry++;
						} while (counter < numEntries);
					}
					m_currentPageIdx++;
					if (m_currentPageIdx == m_hdr.m_pageCt)
					{
						FixPointers();
						m_status = PackageStatus::kPackageStatusLoadingVram;
					}
					break;
				}

				case PackageStatus::kPackageStatusLoadingVram:
				{
					m_status = PackageStatus::kPackageStatusDoingLogin;
					break;
				}

				case PackageStatus::kPackageStatusDoingLogin:
				{
					printf("Login for %s\n", m_realFileName);
					uint32_t pakLoginTableOffset = m_hdr.m_pakLoginTableOffset;
					ResItem* pPakLoginTable = reinterpret_cast<ResItem*>(reinterpret_cast<uint8_t*>(m_pLoadedFile) + m_pageHdrs[m_hdr.m_pakLoginTableIdx] + pakLoginTableOffset);
					uint8_t* pData = (reinterpret_cast<uint8_t*>(pPakLoginTable) + 0x20);
					PakLoginTableEntry* pEntry = nullptr;
					uint32_t max = swapU32(pData);
#ifdef _DEBUG
					printf("Max: %d\n", max);
#endif // _DEBUG

					uint32_t counter = 0;
					do {
						if (max > counter)
						{
							pEntry = reinterpret_cast<PakLoginTableEntry*>(pData + (counter << 0x3));
							uint32_t pageIdx = swapU32(&pEntry->m_wantedPage);
							uint32_t itemOffset = swapU32(&pEntry->m_resItemOffset);
#ifdef _DEBUG
							printf("Page requested %d\nItemOffset 0x%08X\n", pageIdx, itemOffset);
#endif
							ResPage* pPage = reinterpret_cast<ResPage*>(reinterpret_cast<uint8_t*>(m_pLoadedFile) + m_pageHdrs[pageIdx]);
							ResItem* pItem = reinterpret_cast<ResItem*>(reinterpret_cast<uint8_t*>(pPage) + itemOffset);
							//printf("%s - %s\n", (const char*)pItem->m_itemNameOffset, (const char*)pItem->m_itemTypeOffset);
							Login(pItem, pPage, this);
							counter++;
						}
					} while (counter != max);
					return 1;
					break;
				}
			} // end switch
		} // end main while
	}
	else
	{
		return 0;
	}
}

void Package::FixPointers()
{
	uint32_t numLoginPages = m_hdr.m_numPointerFixUpPages;
	if (numLoginPages == 0x8)
	{
		PointerFixUpPage* pLoginPage = reinterpret_cast<PointerFixUpPage*>(reinterpret_cast<uint8_t*>(m_pLoadedFile)+ m_hdr.m_pointerFixUpTableOffset);
		uint32_t numTotalEntries = 0;
		uint32_t offset = swapU32(&pLoginPage->m_dataOffset);
		for (int i = 0; i < 8; i++)
		{
			numTotalEntries += swapU32(&pLoginPage->m_numLoginPageEntries);
			pLoginPage++;
		}
		PointerFixUpPageEntry* pEntry = reinterpret_cast<PointerFixUpPageEntry*>(reinterpret_cast<uint8_t*>(m_pLoadedFile) + offset);
		for (uint32_t j = 0; j < numTotalEntries; j++)
		{
			//lhz       r9, 0(r7)
			uint16_t page1Idx = swapU16(&pEntry->m_page1Idx);
			//lhz       r11, 2(r7)
			uint16_t page2Idx = swapU16(&pEntry->m_page2Idx);
			//lwz       r10, 4(r7)
			uint32_t unkData = swapU32(&pEntry->m_offset);
			//lwzx      r8, r5, r9
			uint8_t* pPage1 = reinterpret_cast<uint8_t*>(m_pLoadedFile) + m_pageHdrs[page1Idx];
			//lwzx      r9, r5, r11
			uint8_t* pPage2 = reinterpret_cast<uint8_t*>(m_pLoadedFile) + m_pageHdrs[page2Idx];
			//add       r10, r10, r8
			pPage1 += unkData;
			//lwz       r0, 0(r10)
			/*
#ifdef _DEBUG
			printf("tmp: 0x%08X\n", swapU32(pPage1));
#endif
*/
			//add       r0, r0, r9
			pPage2 += swapU32(pPage1);
			//stw       r0, 0(r10)
			*reinterpret_cast<uint32_t*>(pPage1) = reinterpret_cast<uintptr_t>(pPage2);
			//addi      r7, r7, 8
			pEntry++;
		}
		/*
		uint32_t counter = 0;
		do{
			uint32_t numloginPageEntries = swapU32(&pLoginPage->m_numLoginPageEntries);
			if (numloginPageEntries != 0)
			{
				uint32_t offset = swapU32(&pLoginPage->m_dataOffset);
				PointerFixUpPageEntry* pEntry = reinterpret_cast<PointerFixUpPageEntry*>(reinterpret_cast<uint8_t*>(m_pLoadedFile) + offset);
				if (numloginPageEntries > 0)
				{
					uint32_t innerCounter = 0;
					do {
						//lhz       r9, 0(r7)
						uint16_t page1Idx = swapU16(&pEntry->m_page1Idx);
						//lhz       r11, 2(r7)
						uint16_t page2Idx = swapU16(&pEntry->m_page2Idx);
						//lwz       r10, 4(r7)
						uint32_t unkData = swapU32(&pEntry->m_offset);
						//lwzx      r8, r5, r9
						uint8_t* pPage1 = reinterpret_cast<uint8_t*>(m_pLoadedFile) + m_pageHdrs[page1Idx];
						//lwzx      r9, r5, r11
						uint8_t* pPage2 = reinterpret_cast<uint8_t*>(m_pLoadedFile) + m_pageHdrs[page2Idx];
						//add       r10, r10, r8
						pPage1 += unkData;
						//lwz       r0, 0(r10)
#ifdef _DEBUG
						printf("tmp: 0x%08X\n", swapU32(pPage1));
#endif

						//add       r0, r0, r9
						pPage2 += swapU32(pPage1);
						//stw       r0, 0(r10)
						* reinterpret_cast<uint32_t*>(pPage1) = reinterpret_cast<uintptr_t>(pPage2);
						//addi      r7, r7, 8
						pEntry++;
						//addi      r0, r6, 1
						innerCounter++;
					} while (innerCounter < numloginPageEntries);
				}
			}
			counter++;
			pLoginPage++;
		} while (counter < numLoginPages);
		*/
	}
	else
	{
		puts("Wrong format!");
	}

}

bool Login(ResItem * pResItem, ResPage * pResPage, Package * pPackage)
{
	StringId typeId =  SID(reinterpret_cast<const char*>(pResItem->m_itemTypeOffset));
	switch (typeId)
	{

		case SID("TAG_INT"):
		{
			printf("\x1B[0;32mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			const char* itemName = (const char*)pResItem->m_itemNameOffset;
			uint32_t data = swapU32(reinterpret_cast<uint8_t*>(pResItem) + 0x20);
			if (strcmp("vis_joint_index", itemName) != 0)
			{
				if (strcmp("coverActionPackCount", itemName) != 0)
				{
					if (strcmp("parenting_joint_name", itemName) != 0)
					{
						puts("unkCase");
					}
					else
					{
						puts("parenting_joint_name");
					}
				}
				else
				{
					printf("coverActionPackCount: %d\n", data);
				}
			}
			else
			{
				printf("vis_joint_index: %d\n", data);
			}
			break;
		}

		case SID("TAG_FLOAT"):
		{
			printf("\x1B[0;32mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			if (strcmp("lod_distance", (const char*)pResItem->m_itemNameOffset) == 0)
			{
				uint32_t lod_distance = swapU32(reinterpret_cast<uint8_t*>(pResItem) + 0x20);
				printf("lod_distance: %f\n", *(float*)&lod_distance);
			}
			break;
		}

		case SID("LEVEL_OFFSET_1"):
		{
			printf("\x1B[0;32mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			break;
		}

		case SID("DEBUG_INFO_1"):
		{
			printf("\x1B[0;32mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			DebugInfo1* pDebugInfo1 = reinterpret_cast<DebugInfo1*>(reinterpret_cast<uint8_t*>(pResItem) + 0x20);
			printf("DEBUG_INFO_1: %s\n", pDebugInfo1->m_pName);
			break;
		}

		case SID("LEVEL_INFO_1"):
		{
			printf("\x1B[0;32mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			break;
		}
	
		case SID("FEATURE_DB_2"):
		{
			printf("\x1B[0;32mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			break;
		}

		case SID("SPAWNER_GROUP"):
		{
			printf("\x1B[0;32mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			EntitySpawnerGroup::DumpInfo(reinterpret_cast<uint8_t*>(pResItem) + 0x20);
			break;
		}

		case SID("PM_COLLECTION_2"):
		{
			printf("\x1B[0;32mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			PmCollection::DumpInfo(reinterpret_cast<uint8_t*>(pResItem) + 0x20);
			break;
		}

		default:
		{
			printf("\x1B[0;31mFound: %s -> 0x%08X\x1B[m\n", reinterpret_cast<const char*>(pResItem->m_itemTypeOffset), typeId);
			break;
		}
	}
	return true;
}
