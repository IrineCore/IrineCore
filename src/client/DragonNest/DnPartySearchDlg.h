#pragma once

#include "EtUIDialog.h"
#include "DnPartyData.h"

class CDnPartySearchDlg : public CEtUIDialog
{
public:
	enum eSearchType
	{
		eSEARCH_DIFFICULTY,
		eSEARCH_QUEST,
		eSEARCH_DAILYMISSION,
		eSEARCH_MAX
	};

	// note by kalliste : Because an event type enum is defined and used in DNMissionSystem on the Server side, 
	//					  Unless moved to commondef, c/s must maintain this separately.
	//					  Remove and follow unified c/s type once defined.
	enum eEventValueTypeEnum
	{
		None,
		PlayerLevel,		// Player level
		PartyCount,			// Number of party members
		MapID,				// Map ID
	};

	enum ePartySearchError
	{
		eERRPS_FAIL_NO_STAGE,
	};

	CDnPartySearchDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPartySearchDlg(void);

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
#ifdef PRE_PARTY_DB
	const std::map<int, int>& GetPartyTargetMapLevelLimitList() const;
	const std::vector<SPartyTargetMapInfo>& GetPartyTargetMapList() const;
#else
	void SetCurrentPartyListPage(int page) { m_nCurPartyListPage = page; }
#endif

protected:
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ );

private:
	void SetDifficultyComboBox();
	void SetQuestComboBox();
	void SetMissionComboBox();
	bool OnSetMapSelect(int nMapId, bool bOn);

	bool DoAddListBox(const SPartyTargetMapInfo& info, int nMapStringId);
	void DoAddQuestComboBox(TQuest* pQuest);
	void DoAddMissionComboBox(const std::wstring& missionName, int mapId);

	bool IsMapInSearchArea(int mapIdx, int mapNameId) const;
	bool IsMapStringInMapAreaNameList(const std::wstring& mapName) const;
#ifdef PRE_MOD_PARTY_SEARCH_MINLEV
	bool IsPlayerEnterMap(int mapId) const;
#endif

	void RefreshListBox();
	void RefreshQuestDescription(int nMapId);
	void RefreshMissionDescription(int nMapId);
	void RefreshSearchCondition();

	void HandlePartySearchError(ePartySearchError code);

	CEtUIStatic*			m_pCurConditionStatic;
	CEtUIComboBox*			m_pSearchComboBox[eSEARCH_MAX];
	CEtUITextBox*			m_pDescriptions[eSEARCH_MAX];
	CEtUIListBoxEx*			m_pListBoxEx;
	CEtUIButton*			m_pSearch;
	CEtUIButton*			m_pClose;
#ifdef PRE_PARTY_DB
	CEtUICheckBox* m_pCurVillageDungeonCheckBox;
#endif
	int						m_nCurrentMapIdx;
	std::map<int, int>		m_nMapLimitLevelList;	// mapid / limitLevel
	std::vector<SPartyTargetMapInfo> m_MapAreaNameList;

	int						m_nCurSelectDifficulty;
	int						m_nCurSelectQuestMapStringId;
	int						m_nCurSelectMisssionMapStringId;
#ifdef PRE_PARTY_DB
#else
	int						m_nCurPartyListPage;
#endif
	//std::vector<int>		m_nMapSelected;
};
