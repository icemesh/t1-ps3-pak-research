/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/
#pragma once

#include <cinttypes>

namespace SMath
{
	struct Vec4
	{
		float m_x;
		float m_y;
		float m_z;
		float m_w;
	};

	struct Mat44
	{
		Vec4 m_row[4];
	};
}

namespace PmCollection
{
	void DumpInfo(uint8_t* pMem);

	struct PmCollectionEntry;
	struct PmPrototype;
	struct PmInstance;
	struct PmRenderable;

	struct SurfaceDesc;

	struct PmCollection2
	{
		uint32_t			m_numPmCollections;
		PmCollectionEntry*	m_apEntries;
	};

	struct PmCollectionEntry
	{
		PmPrototype*	m_pPrototype;///< <c>0x00</c>: 
		PmInstance**	m_pInstanceTable;///< <c>0x04</c>: 
		int32_t			m_numInstances;///< <c>0x08</c>: 
		int32_t			m_numPrototypeMods;///< <c>0x0C</c>: unused ?
	};

	// 0x80 ?
	struct PmPrototype
	{
		struct DebugInfo
		{
			const char* m_name;
			uint32_t m_pad;
		};
		uint8_t			m_numLodGroups;///< <c>0x00</c>:
		uint8_t			m_modelFlags;
		uint16_t		m_numBones;///< <c>0x02</c>:
		uint16_t		m_numTotalRenderables;///< <c>0x04</c>:
		uint16_t		m_numTotalVertexSets;///< <c>0x06</c>:
		uint16_t		m_fadeStart;///< <c>0x08</c>:
		uint16_t		m_fadeEnd;///< <c>0x0A</c>:
		uint16_t		m_shaderLodStart;
		uint16_t		m_shaderLodEnd;
		PmRenderable*	m_pRenderableTable;///< <c>0x10</c>:
		uint32_t		m_pVertexSetTable;///< <c>0x14</c>:
		uint32_t		m_pVertexInfuenceTable;///< <c>0x18</c>:
		DebugInfo*		m_szProtoName;///< <c>0x1C</c>: eg art/t1/levels/test/anewman-test/playtest-melee-training-basic.ma
		uint32_t		field_20;
		uint32_t		field_24;
		uint32_t		field_28;
		uint32_t		field_2C;
		uint16_t		m_fadeInStart;///< <c>0x30</c>:
		uint16_t		m_fadeInEnd;///< <c>0x32</c>:
		uint32_t		field_34;
		uint32_t		field_38;
		uint32_t		field_3C;
		uint32_t		m_pLod;///< <c>0x</c>:
		uint32_t		field_44;
		uint32_t		field_48;//this is probably 2 uint16_t fields
		uint32_t		field_4C;
	};

	//ok
	struct PmInstance
	{
		uint16_t		m_instanceFlags;//0x0 ok
		uint16_t		m_instanceIdx;//0x2 ok
		uint16_t		m_numExtraInfluences;//0x4 unsure
		uint16_t		field_6;
		uint16_t		m_numBlendShapeInflences;//0x8 ok
		uint16_t		m_visibilityIndex;//0xA ok
		uint32_t		m_pRenderableFlags;//0xC m_pRenderableFlags //ok
		uint32_t		m_unkFloatPtr;
		uint32_t		m_couldBeInterestingPtr;
		uint32_t		m_pVertexSetPatchTable;//0x18
		SMath::Mat44*	m_transform;
		uint32_t		m_pWrinkleData;
		uint32_t		m_pBlendShapeFactors;
		uint32_t		m_pPmInstanceParameters;
		uint32_t		field_2C;
		SMath::Vec4		m_boundingSphere;
	};

	//ok
	struct PmRenderable 
	{
		uint32_t	m_pSurfaceDesc;
		uint16_t	m_firstVertexSet;
		uint16_t	m_numVertexSets;
		uint32_t	m_pVertexSetBlendShapes;
		uint8_t		m_runtimeFlags;
		uint8_t		field_D;
		uint8_t		m_defaultFlags;
		uint8_t		field_F;
	};

	
}