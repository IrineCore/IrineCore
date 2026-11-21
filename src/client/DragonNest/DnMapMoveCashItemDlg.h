#pragma once
#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"

class CDnItem;
class CDnMapMoveCashItemDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnMapMoveCashItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMapMoveCashItemDlg(void);

protected:

	// List of movable maps. Must keep adding when new destination towns are added.
	enum
	{
		PRARIETOWN,
		MANARIDGE,
		CADERACK_GATEWAY,
		SAINTHAEVEN,
		LOTUSMARSH,
		COLOSSEUM,
		FARM,
		NUM_MOVABLE_MAP,
	};

	enum
	{
		NUM_SLOT = 5,
	};

	enum
	{
		MESSAGEBOX_OPENBOX,
	};

	struct S_MAP_INFO
	{
		CEtUIRadioButton* pRadioButton;
		CEtUIStatic* pMapImage;
		int iMapID;		// Map ID in map table
		int iToolTipUIString;
		int iMapExplanationStringID;
		int iLevelLimit;

		S_MAP_INFO( void ) : pRadioButton( NULL ), 
							 pMapImage( NULL ),
							 iMapID( 0 ),
							 iToolTipUIString( 0 ),
							 iMapExplanationStringID( 0 ),
							 iLevelLimit( 0 )
		{

		};
	};

	CDnItem* m_pUsedItem;

	S_MAP_INFO m_apMovableMapInfo[ NUM_MOVABLE_MAP ];
	CEtUIStatic* m_pStaticMapName;
	CEtUIStatic* m_pStaticSummary;
	CEtUIButton* m_pOKBtn;
	CEtUIButton* m_pCancelBtn;
	int m_iSelectedMapRadioBtn;

	CDnSmartMoveCursor m_SmartMove;

public:
	void SetUsedItem( CDnItem* pItem ) { m_pUsedItem = pItem; };
	void SetVillageList( const WarpVillage::WarpVillageInfo* pVillageList, int iListCount );
	void SelectMap( int iIndex );
	void ResetOKBtn( void );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
};
