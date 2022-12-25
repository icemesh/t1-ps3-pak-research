/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/
#pragma once

#include <cinttypes>

namespace EntitySpawnerGroup
{
	struct PropertyValue
	{
		uint16_t	m_typeId;
		uint16_t	m_unk;
		void**		m_pValue;
	};

	struct Property
	{
		uint32_t		m_id;
		PropertyValue*	m_pVal;
	};

	struct EntitySpawner
	{
		uint32_t		m_posX;				///< <c>0x00</c>: float
		uint32_t		m_posY;				///< <c>0x04</c>: float
		uint32_t		m_posZ;				///< <c>0x08</c>: float
		uint32_t		m_unk;				///< <c>0x0C</c>: unk
		uint32_t		m_rotX;				///< <c>0x10</c>: float
		uint32_t		m_rotY;				///< <c>0x14</c>: float
		uint32_t		m_rotZ;				///< <c>0x18</c>: float
		uint32_t		m_rotW;				///< <c>0x1C</c>: float
		const char*		m_spawnerName;		///< <c>0x20</c>: spawner name
		uint8_t			m_unk2[0xC];		///< <c>0x24</c>: 
		uint32_t		m_processType;		///< <c>0x30</c>: StringId of the process type
		const char*		m_artGroup;			///< <c>0x34</c>: art group name
		uint32_t		m_spawnerFlags;		///< <c>0x38</c>: spawner flags
		uint8_t*		m_pLevel;			///< <c>0x3C</c>: used to store the pointer to the level class
		EntitySpawner*	m_pParentSpawner;	///< <c>0x40</c>: ptr to the parent spawner
		uint8_t*		m_pSchemaProperty;	///< <c>0x44</c>: ptr to the schema property
		uint32_t		m_hasLoginData;		///< <c>0x48</c>: used to store the level name
	};

	struct SpawnerGroup
	{
		uint32_t	m_unk;					///< <c>0x00</c>: alwasy 0
		EntitySpawner**	m_apEntitySpawners;	///< <c>0x04</c>: EntitySpawner table
		uint32_t	m_numSpawners;			///< <c>0x08</c>: number of spawners

	};

	void DumpInfo(uint8_t* pMem);
}
