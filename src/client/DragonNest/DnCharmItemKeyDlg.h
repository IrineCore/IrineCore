#pragma once
#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"

class CDnItem;

class CDnCharmItemKeyDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnCharmItemKeyDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharmItemKeyDlg(void);

#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
	struct SCharmKeyInfo
	{
		BYTE  cKeyIndexIndex;
		int   nKeyItemID;
		INT64 biKeyItemSerial;
	};
#endif 

protected:

	enum
	{
		NUM_SLOT = 5,
	};

	enum
	{
		MESSAGEBOX_OPENBOX,
	};

	std::vector<CDnItem*> m_vecItem;
	std::vector<CDnItemSlotButton*> m_vecSlotButton;
	std::vector<CEtUIStatic*> m_vecSelect;

	char m_cInvenType;
	short m_sInvenIndex;
	INT64 m_biInvenSerial;
	int m_nTypeParam;
	BYTE m_cKeyInvenIndex;
	int m_nKeyItemID;
	INT64 m_biKeyItemSerial;

	std::vector<CDnItem *> m_vecKeyItem;

	CEtUIButton *m_pButtonPagePrev;
	CEtUIButton *m_pButtonPageNext;
	CEtUIStatic *m_pStaticPage;
	int m_nCurPage;
	int m_nMaxPage;
	CEtUIButton *m_pButtonOK;

	CDnSmartMoveCursor m_SmartMove;

	int m_nNeedEmptySlotCount;

#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
	bool CheckInvenNeedExtraSlot( CDnItem* pCharmItem );
	SCharmKeyInfo m_CharmKeyInfo;
#endif 

	bool CheckInvenEmptySlot();

	void PrevPage();
	void NextPage();
	void UpdatePage();
	void UpdateKeySlot();

	void UpdateKeyList();

	void RequestUseCharmItem();

public:

#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
	SCharmKeyInfo GetCharmKeyInfo()		{ return m_CharmKeyInfo; }
#endif
	
#ifdef PRE_ADD_AUTOUNPACK
	// When AutoUnPackDlg is open, set bDirectly to True so the confirmation message box does not appear.
	void SetCharmItem( CDnItem *pCharmItem, bool bDirectly=false );
#else
	void SetCharmItem( CDnItem *pCharmItem );
#endif
	void OnRecvRequestCharmItem( char cInvenType, short sInvenIndex, INT64 biInvenSerial );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
};