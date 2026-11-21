#pragma once

#include "DnStageClearImp.h"
class CDnDungeonClearImp : public CDnStageClearImp
{
public:
	CDnDungeonClearImp( CDnGameTask *pTask );
	virtual ~CDnDungeonClearImp();

	struct SDungeonClearInfo
	{
		TDungeonClearInfo sBaseInfo; // Note : Basic info received from server
		char cRewardItemType[4];
//		int nRewardItemID[4];
		TItem RewardItem[4];

		int nClassID;
		int nLevel;
		std::wstring strName;

		int nComboBonus;
		int nKillBonus;
		std::wstring strTime;
		int nTimeBonus;
		int nPartyBonus;

#if defined( PRE_ADD_CP_RANK )
		bool bBestPlayer;
		bool bNewRecord;
#endif	// #if defined( PRE_ADD_CP_RANK )

		SDungeonClearInfo()
			: nClassID(0)
			, nLevel(0)
			, nComboBonus(0)
			, nKillBonus(0)
			, nTimeBonus(0)
			, nPartyBonus(0)
#if defined( PRE_ADD_CP_RANK )
			, bBestPlayer( false )
			, bNewRecord( false )
#endif	// #if defined( PRE_ADD_CP_RANK )
		{
			SecureZeroMemory( &sBaseInfo, sizeof(TDungeonClearInfo) );
			memset( cRewardItemType, 0, sizeof(cRewardItemType) );
//			memset( &nRewardItemID, 0, sizeof(nRewardItemID) );
			memset( RewardItem, 0, sizeof(RewardItem) );
		}
	};

	struct SDungeonClearValue
	{
		int nDungeonLevel;			// Note : Dungeon level
		int nClearTime;
		int nMaxCombo;				// Note: Max combo count x
		int nKillMonster;			// Note : Normal kill count x
		int nPartyBonus;			// Note: Party score
		int nTimeLimit;
		int nTimeRatio;
		int nPartyCount;
		int nCurrentExp;
		int nCurrentLevel;
		bool bIgnoreRewardItem;

		SDungeonClearValue() : nDungeonLevel( 0 ), nClearTime( 0 ), nMaxCombo( 0 ), nKillMonster( 0 ), 
								nPartyBonus( 0 ), nTimeLimit( 0 ), nTimeRatio( 0 ), nPartyCount( 0 ), 
								nCurrentExp( 0 ), nCurrentLevel( 0 ), bIgnoreRewardItem( false )
		{
		}
	};

protected:
	SDungeonClearValue m_DungeonClearValue;
	std::vector<SDungeonClearInfo> m_vecDungeonClearInfo;

protected:
	void CalcDungeonClearInfo();

public:
	virtual void OnCalcClearProcess( void *pData );
	virtual void OnCalcClearRewardItemProcess( void *pData );
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	virtual void OnCalcClearRewardBoxProcess( void *pData );
#endif

	std::vector<SDungeonClearInfo>& GetDungeonClearInfoList() { return m_vecDungeonClearInfo; }
	SDungeonClearInfo* GetDungeonClearInfo( int nSessionID );
	SDungeonClearValue& GetDungeonClearValue() { return m_DungeonClearValue; }
};