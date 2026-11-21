#pragma once
#include "DnCustomDlg.h"
#include "DnRenderAvatar.h"
#include "DnParts.h"
#include "MAPartsBody.h"


#ifdef PRE_ADD_GACHA_JAPAN
class CDnGachaPreview : public CDnCustomDlg
						
{
private:
	// Character set parts read from the table.
	// Strictly for previewing costume appearance, not item stats or
	// Do not use any other attributes; only for selecting and displaying appearance.
	// (Japanese gacha only drops cash items.)
	struct S_COSTUME_INFO
	{
		int aiCashParts[ CDnParts::CashPartsTypeEnum_Amount ];
		tstring strSetName;
		int iGradeNameStringID;

		S_COSTUME_INFO( void ) : iGradeNameStringID( 0 )
		{
			SecureZeroMemory( aiCashParts, sizeof(aiCashParts) );
		};
	};

	// Store costume sets grouped by job.
	struct S_COSTUME_SET_INFO
	{
		int iClassID;
		int iNowSelectedCostumeSet;
		vector<S_COSTUME_INFO> vlCostumes;

		S_COSTUME_SET_INFO( void ) : iClassID( 0 ), iNowSelectedCostumeSet( 0 )
		{

		};
	};

	// Current gachapon shop ID received from server
	int m_iGachaShopID;

	CEtUIButton* m_pBtnRotateLeft;
	CEtUIButton* m_pBtnRotateRight;
	CEtUIButton* m_pBtnRotateFront;
	CEtUIButton* m_pBtnRotateBack;
	CEtUIButton* m_pBtnAvatarViewArea;

	CEtUITextBox* m_pTextBoxSetName;

	CDnRenderAvatarNew m_RenderAvatar;

	// Directly control the character with the mouse.
	float m_fAvatarViewMouseX;
	float m_fAvatarViewMouseY;

	// Per-class appearance info displayable per gacha NPC, read from the table.
	// Initialized with the value sent from the server when this dialog opens.
	map<int, S_COSTUME_SET_INFO> m_mapCostumeSetInfoByJob;
	int m_iNowSelectedClassID;

	// Variable set when selecting a part to draw in the gachapon dialog.
	// Start the continuous parts-changing effect based on this variable.
	int m_iSelectedPart;
	int m_iRouletteCostumePartIndex;
	float m_fPartChangeTime;
	float m_fElapsedTime;

	// State where the costume currently worn by my character is shown in preview.
	bool m_bSelectedMyJobClassCostume;

protected:
	void _ChangeCostumeParts( const S_COSTUME_INFO& CostumeInfo );

public:
	CDnGachaPreview( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGachaPreview( void );

	// Must set gachapon shop ID upon receiving it from the server.
	void SetGachaponShopID( int iShopID ) { m_iGachaShopID = iShopID; };

	// UI-related functions
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate( void );
	virtual void Show( bool bShow );
	virtual void Process( float fDelta );
	virtual void Render( float fDelta );
	// Handle event
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	// Avatar-related functions
	void InitializeCostumeSetInfo( int iGachaTableID );

	void AttachCostume( int iClassID, bool bAllowAttachLocalActorCostume = true );
	void AttachPart( int iItemID );
	void UpdateJobSelection( int iRootJobID );
	void NextCostume( void );
	void PrevCostume( void );
	void SelectedPart( int iSelectedPart );

	// Data used by GachaDialog.
	int GetNumGachPartItem( int iClassID );
	int GetGachaSelectedPartItemID( int iClassID, int iIndex );

	// Utility function also used in GachaDialog.
	static int GetItemNeedJob( DNTableFileFormat*  pItemTable, int iItemTableID );
};
#endif // PRE_ADD_GACHA_JAPAN