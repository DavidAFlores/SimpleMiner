#include "Game/Voxel.hpp"

std::vector<BlockDef> BlockDef::s_blockDefs;

//BlockDef::BlockDef()
//{
//}

BlockDef::BlockDef(bool visible, bool solid, bool opaque, IntVec2 UVsBottom, IntVec2 UvsSide, IntVec2 UVsTop)
	:m_visible(visible)
	,m_solid(solid)
	,m_opaque(opaque)
	,m_UVsBottom(UVsBottom)
	,m_UVsSide(UvsSide)
	,m_UVsTop(UVsTop)
{
}

void BlockDef::InitializeBlockDefs()
{
	//air
	s_blockDefs.push_back(BlockDef(false, false, false, IntVec2(0, 0), IntVec2(0, 0), IntVec2(0, 0)));

	//water
	s_blockDefs.push_back(BlockDef(true, false, false, IntVec2(32, 44), IntVec2(32, 44), IntVec2(32, 44)));

	//dirt
	s_blockDefs.push_back(BlockDef(true, true, true, IntVec2(32, 34), IntVec2(32, 34), IntVec2(32, 34)));

	//grass
	s_blockDefs.push_back(BlockDef(true, true, true, IntVec2(32, 34), IntVec2(33, 33), IntVec2(32, 33)));

	//stone
	s_blockDefs.push_back(BlockDef(true, true, true, IntVec2(33, 32), IntVec2(33, 32), IntVec2(33, 32)));

	//coal
	s_blockDefs.push_back(BlockDef(true, true, true, IntVec2(63, 34), IntVec2(63, 34), IntVec2(63, 34)));

	//iron
	s_blockDefs.push_back(BlockDef(true, true, true, IntVec2(63, 35), IntVec2(63, 35), IntVec2(63, 35)));

	//gold
	s_blockDefs.push_back(BlockDef(true, true, true, IntVec2(63, 36), IntVec2(63, 36), IntVec2(63, 36)));

	//diamond
	s_blockDefs.push_back(BlockDef(true, true, true, IntVec2(63, 37), IntVec2(63, 37), IntVec2(63, 37)));

	//cobblestone
	s_blockDefs.push_back(BlockDef(true, true, true, IntVec2(34, 32), IntVec2(34, 32), IntVec2(34, 32)));
}

Voxel::Voxel(unsigned char type)
	:m_type(type)
{
}
