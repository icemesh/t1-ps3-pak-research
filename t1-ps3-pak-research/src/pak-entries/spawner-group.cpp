/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/

#include "spawner-group.h"
#include "../utils/utils.h"
#include "../utils/sidbase/sidbase.h"
#include <stdio.h>

extern uintptr_t g_packageOffset;

void EntitySpawnerGroup::DumpInfo(uint8_t* pMem)
{
	SpawnerGroup* pSpawnerGroup = reinterpret_cast<SpawnerGroup*>(pMem);
	int32_t numSpawners = (int32_t)swapU32(&pSpawnerGroup->m_numSpawners);
	if (numSpawners != 0)
	{
		InitSidbase();
		int32_t iSpawner;
		for (iSpawner = 0; iSpawner < numSpawners; iSpawner++)
		{
			EntitySpawner* pSpawner = pSpawnerGroup->m_apEntitySpawners[iSpawner];
			printf("\nEntitySpawner \"%s\":\n", pSpawner->m_spawnerName);
			printf("  Process Type: '%s\n", StringIdToStringInternal(swapU32(&pSpawner->m_processType)));
			printf("  Art Group: \"%s\"\n", pSpawner->m_artGroup);
			printf("  Level: \"%s\"\n", "(null)");
			const char* parentSpawner = "(none)";
			if (pSpawner->m_pParentSpawner)
			{
				//lwz       r5, 0x20(r9)
				parentSpawner = pSpawner->m_pParentSpawner->m_spawnerName;
			}
			printf("  Parent Spawner: \"%s\"\n", parentSpawner);
			uint32_t posX = swapU32(&pSpawner->m_posX);
			uint32_t posY = swapU32(&pSpawner->m_posY);
			uint32_t posZ = swapU32(&pSpawner->m_posZ);
			uint32_t rotX = swapU32(&pSpawner->m_rotX);
			uint32_t rotY = swapU32(&pSpawner->m_rotY);
			uint32_t rotZ = swapU32(&pSpawner->m_rotZ);
			uint32_t rotW = swapU32(&pSpawner->m_rotW);
			printf("  Local Pos/Quat: (%4.2f, %4.2f, %4.2f) / (%4.2f, %4.2f, %4.2f, %4.2f) //value @ %.08X\n",
				*(float*)&posX,
				*(float*)&posY,
				*(float*)&posZ,
				*(float*)&rotX,
				*(float*)&rotY,
				*(float*)&rotZ,
				*(float*)&rotW,
				(reinterpret_cast<uintptr_t>(pSpawner) - g_packageOffset));
			puts("  Flags: ");
			uint32_t flags = swapU32(&pSpawner->m_spawnerFlags);
			if ((flags & 0x2) != 0)
			{
				puts("noAutoBirth ");
			}
			puts("");
			puts("  Schema Properties:\n");
			//lwz       r9, 0x44(r24)
			//cmpwi     cr7, r9, 0
			uint8_t* pSchema = pSpawner->m_pSchemaProperty;
			if (pSchema)
			{
				//clrldi    r26, r9, 32
				//lwz       r0, 4(r26)
				int32_t propertyCount = (int32_t)swapU32(pSchema + 0x4);
				//lwz       r25, 0(r26)
				Property* pPropertyTable = (Property*)*(uint32_t*)pSchema;
				int32_t iProperty;
				for (iProperty = 0; iProperty < propertyCount; iProperty++)
				{
					//lwz       r3, 0(r11)
					uint32_t propertyId = swapU32(&pPropertyTable->m_id);
					//"    '%s: "
					printf("    '%s: ", StringIdToStringInternal(propertyId));
					//lwz       r28, 4(r11)
					//uint32_t type = swapU32(pSchema+0x4);
					PropertyValue* pVal = pPropertyTable->m_pVal;
					switch (swapU16(&pVal->m_typeId))
					{
					case 0x1:
					{
						int32_t value = 0;
						if (pVal->m_pValue)
						{
							value = swapU32(&*pVal->m_pValue);
						}
						printf("%d //value @ %.08X\n", value, (reinterpret_cast<uintptr_t>(pVal->m_pValue) - g_packageOffset));
						break;
					}
					case 0x4:
					{
						uint32_t value = 0;
						if (pVal->m_pValue)
						{
							value = swapU32(&*pVal->m_pValue);
						}
						printf("%f //value @ %.08X\n", *(float*)&value, (reinterpret_cast<uintptr_t>(pVal->m_pValue) - g_packageOffset));
						break;
					}

					case 5:
					{
						const char* value = "(null)";
						if (pVal->m_pValue)
						{
							value = reinterpret_cast<const char*>(*pVal->m_pValue);
						}
						printf("%s //value @ %.08X\n", value, (reinterpret_cast<uintptr_t>(pVal->m_pValue) - g_packageOffset));
						break;
					}

					case 7:
					case 8:
					case 9:
					{
						uint32_t x = 0;
						uint32_t y = 0;
						uint32_t z = 0;
						if (pVal->m_pValue)
						{
							uint32_t* value = reinterpret_cast<uint32_t*>(pVal->m_pValue);
							x = swapU32(&value[0]);
							y = swapU32(&value[1]);
							z = swapU32(&value[2]);
						}
						printf("(%4.2f, %4.2f, %4.2f) //value @ %.08X\n", *(float*)&x, *(float*)&y, *(float*)&z, (reinterpret_cast<uintptr_t>(pVal->m_pValue) - g_packageOffset));
						break;
					}

					case 13:
					{
						uint32_t value = 0;
						if (pVal->m_pValue)
						{
							value = swapU32(&*pVal->m_pValue);
						}
						printf("'%s  //value @ %.08X\n", StringIdToStringInternal(value), (reinterpret_cast<uintptr_t>(pVal->m_pValue) - g_packageOffset));
						break;
					}

					default:
					{
						printf("(%d) unsupported print type\n", swapU16(&pVal->m_typeId));
						break;
					}
					}
					pPropertyTable++;
				}

			}
		}
		ShutdownSidbase();
	}
}
