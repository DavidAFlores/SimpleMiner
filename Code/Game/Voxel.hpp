#pragma once

#include "Engine/Math/IntVec2.hpp"

#include <vector>
#include <string>

struct BlockDef
{
	//BlockDef();
	BlockDef(bool visible = false, bool solid = false, bool opaque = false, IntVec2 UVsBottom = IntVec2(0, 0), IntVec2 UvsSide = IntVec2(0, 0), IntVec2 UVsTop = IntVec2(0, 0));

	bool m_visible;
	bool m_solid;
	bool m_opaque;
	IntVec2 m_UVsBottom;
	IntVec2 m_UVsSide;
	IntVec2 m_UVsTop;

	static std::vector<BlockDef> s_blockDefs;

	static void InitializeBlockDefs();

	//static BlockDef* GetDefinitionAtName(std::string name);
};


class Voxel
{
public:
	Voxel() = default;
	Voxel(unsigned char type);

public:
	unsigned char m_type;
};