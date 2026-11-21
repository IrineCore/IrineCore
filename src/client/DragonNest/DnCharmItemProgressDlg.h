#pragma once
#include "DnCustomDlg.h"

class CDnItem;
class CDnItemSlotButton;

class CDnCharmItemProgressDlg : public CDnCustomDlg
{
public:
	CDnCharmItemProgressDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharmItemProgressDlg(void);

protected:
	CEtUIButton *m_pCancelButton;
	CEtUIProgressBar *m_pProgressBarTime;

	float m_fMaxTimer;
	float m_fTimer;
	char m_cFlag;

	char m_cInvenType;
	short m_sInvenIndex;
	INT64 m_biInvenSerial;
	BYTE m_cKeyInvenIndex;
	int m_nKeyItemID;
	INT64 m_biKeyItemSerial;

	int m_nCurItemIndex;
	std::map<int, int> m_CharmItemList;		//List for displaying on screen
	std::map<int, int> m_OrigCharmItemList;	//List created from MakeItem list
	CDnItem* m_pTempItem;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );

	void ShowEx( bool bShow, char cInvenType, short sInvenIndex = -1, INT64 biInvenSerial = 0, BYTE cKeyInvenIndex = -1, int nKeyItemID = 0, INT64 biKeyItemSerial = 0, float fTimer = 0.f, float fDelayTime = 0.0f );

#if defined(PRE_FIX_43986)
protected:
	float m_ItemDelayTime;	//Item display duration.
	float m_CurItemDelay;	

	int m_nSoundIndex;
	EtSoundChannelHandle m_hSound;

	CDnItemSlotButton* m_pItemSlot;
	CEtUIStatic* m_pStaticText;

	void MakeItemList();
	void ChangeItemInfo();
#endif // PRE_FIX_43986

};