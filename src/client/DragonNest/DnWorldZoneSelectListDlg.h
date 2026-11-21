#pragma once

#include "DnWorld.h"

class CDnWorldZoneSelectListDlg : public CEtUIDialog
{
public:
	CDnWorldZoneSelectListDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnWorldZoneSelectListDlg(void);

protected:
	CEtUIStatic * m_pStaticName;
	CEtUIStatic * m_pStaticQuestImage;
	CEtUIStatic * m_pStaticQuestText;

	bool m_bIsEnterStage;

protected:
	void SetMapName( const std::wstring & szMapName, const bool bEnable = true );
	void SetQuestInfo( const int nMapIndex );
	int GetEnableQuestCount( std::vector<TQuest*> & vecQuestList, std::vector<int> & vecDungeonIndex );
	int GetEnableQuestCount( const int nMapIndex, std::vector<TQuest*> & vecQuestList );
#ifdef PRE_FIX_QUEST_COUNT_WORLDZONE
	bool HasEnableQuestInDungeons(const TQuest& quest, const std::vector<int> vecDungeonIndexes) const;
#endif
	void GetTargetMapDungeonIndex( const int nTargetMapIndex, std::vector<int> & vecDungeonIndex );
	bool IsEnterStage( const int nMapIndex );
	int GetStageEnterLevel( const int nMapIndex );

public:
	void SetGateInfo( CDnWorld::GateStruct * pGateStruct );
	bool IsEnterStage() { return m_bIsEnterStage; }

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
	int GetEnableQuestCountWithMapIndex( int nMapIndex );
#endif 

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
};
