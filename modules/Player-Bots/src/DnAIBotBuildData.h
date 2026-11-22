#pragma once

#include <vector>

class CDNUserSession;
class CDNGameRoom;

namespace AIBot
{
	enum class RoleType
	{
		Tank = 0,
		Healer,
		IceStacker,
		Support,
		Dps,
	};

	struct BotJobSpec
	{
		BYTE cSecondJob;
		BYTE cFirstJob;
		BYTE cBaseClass;
		RoleType Role;
		char cMainWeaponType;
		char cSubWeaponType;
		const WCHAR* pDefaultName;
	};

	struct HumanBuildProfile
	{
		int   nLevel;
		int   nEquippedNormalCount;
		int   nEquippedAccessoryCount;
		float fAvgRank;
		float fAvgItemLevel;
		float fAvgEnchant;
		bool  bHasPotentials;
		int   nCashCostumeCount;
		bool  bHasCashCostumes;
		int   nEnhGlyphCount;
		int   nSkillGlyphCount;
		bool  bHasSpecialGlyph;
		int   nCashGlyphCount;
		int   nDragonGlyphCount;
		int   nMaxGlyphRank;
		int   nTalismanCount;
		int   nMaxTalismanRank;
		float fSkillRankRatio;

		HumanBuildProfile()
			: nLevel( 1 )
			, nEquippedNormalCount( 0 )
			, nEquippedAccessoryCount( 0 )
			, fAvgRank( 1.0f )
			, fAvgItemLevel( 1.0f )
			, fAvgEnchant( 0.0f )
			, bHasPotentials( false )
			, nCashCostumeCount( 0 )
			, bHasCashCostumes( false )
			, nEnhGlyphCount( 0 )
			, nSkillGlyphCount( 0 )
			, bHasSpecialGlyph( false )
			, nCashGlyphCount( 0 )
			, nDragonGlyphCount( 0 )
			, nMaxGlyphRank( 2 )
			, nTalismanCount( 0 )
			, nMaxTalismanRank( 2 )
			, fSkillRankRatio( 0.85f )
		{}
	};

	RoleType GetJobRole( BYTE cJobID );
	const BotJobSpec* FindJobSpec( BYTE cJobID );

	void AnalyzeHumanBuildProfile( CDNUserSession* pHuman, HumanBuildProfile& outProfile );
	void SelectSquadSpecs( BYTE cHumanJobID, int nBotCount, int nTargetLevel, std::vector<const BotJobSpec*>& outSpecs );
	bool PopulateBotSession( CDNUserSession* pBotSession, const BotJobSpec* pSpec, int nBotSlotIndex, const HumanBuildProfile& humanProfile );
	bool PopulateBotSession( CDNUserSession* pBotSession, const BotJobSpec* pSpec, int nBotSlotIndex, int nTargetLevel = 60 );
}
