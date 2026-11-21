#pragma once
#include "DnCustomDlg.h"

class CDnPVPLadderTabDlg : public CDnCustomDlg, public CEtUICallback
{
public:					 
	CDnPVPLadderTabDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPLadderTabDlg(void);
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );

public:
	enum LadderPage
	{
		_1vs1,
		_2vs2,
		_3vs3,
		_4vs4,
		Max,
	}; // Common ladder type; since LadderSystem uses 1, 2, 3, 4, declare one extra type to match the array.

	int GetSelectedChannel(){return m_nSelectedLadderChannel;}
	void EnableTabButton( int nIndex,bool bEnable = true, bool bAll = false );

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	void SelectPvPChannelProperly();
	void UpdatePvPChannelTab();
	void UpdateAndSelectTabProperly();
	void SetSelectedGradeChannelTab( PvPCommon::RoomType::eRoomType eSelectedRoomType );
	void SetChannelTabSelectTimeStampForInputDelay();
	void SetPVPLadderState( int iState ) { m_iLadderState = iState; };
	PvPCommon::RoomType::eRoomType GetSelectedGradeChannel( void ) { return m_eSelectedGradeChannel; };
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	void SelectLadderTab(int nType);

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	void EnableVillageAccessMode(bool bTrue);
	bool IsVillageAccessMode( void ) { return m_bVillageAccessMode; };
#endif

protected:
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	CEtUIRadioButton* m_pRButtonBeginnerRoomList;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
	CEtUIRadioButton *m_pRButtonRoomList;
	CEtUIRadioButton *m_pRButtonLadder[LadderPage::Max];

protected:
	int m_nSelectedLadderChannel;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	PvPCommon::RoomType::eRoomType m_eSelectedGradeChannel;
	PvPCommon::RoomType::eRoomType m_eLastSelectdGradeChannel;
	bool m_bIsRegular;
	DWORD m_dwLastSelectChannelTime;
	bool m_bChannelTabDelay;
	int m_iLadderState;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	// Since this class is created every time entering a village, once set to true it does not need to be reset to false.
	bool m_bVillageAccessMode;
#endif // #ifdef PRE_ADD_PVP_VILLAGE_ACCESS

};