#pragma once
#include "DnCustomDlg.h"

const int NPC_REPUTATION_ALBUM_COUNT_PER_PAGE = 8;

class CDnNpcReputationDlg;
class CDnNpcReputationBromideDlg;

// Affinity tab dialog.
// Holds the affinity list dialog and the affinity album dialog.
class CDnNpcReputationAlbumDlg : public CDnCustomDlg,
								 public CEtUICallback
{
private:
	enum
	{
		NPC_SORT_DEFAULT,
		NPC_SORT_FAVOR,
		NPC_SORT_MALICE,
		NPC_SORT_NAME,
	};

	struct S_ALBUM_ELEMENT
	{
		//EtTextureHandle hTexture; // Small texture.
		CEtUITextureControl* pPictureCtrl;
		CEtUIStatic* pStaticEventReceiver;
		CEtUIStatic* pName;

		struct S_ALBUM_ELEMENT( void ) : pPictureCtrl( NULL ), pStaticEventReceiver( NULL ), pName( NULL ) {};
		void Reset( void )
		{
			//SAFE_RELEASE_SPTR( hTexture );
			pPictureCtrl->Show( false );
			pName->ClearText();
		}
	};

	struct S_AVAILALBUM_INFO
	{
		int iReputeTableID;
		int iAlbumListOffset;		// Stores the starting index in the album list for this NPC's albums.

		vector<int>		vlPortraitIndex; // Index in the bromide portrait texture for the NPC. Up to 3 are currently provided per NPC.
		vector<string>	vlBromideTextureFileName;
#ifdef PRE_ADD_REPUTATION_EXPOSURE
		vector<int>		vIBromideQuestID;		// Quest ID unlocking bromide (Album open: 1st check)
		vector<int>		vIBromideQuestEnumID;	// Quest Enum ID unlocking bromide (Album open: 1st check)
		vector<int>		vIBromideRepute;		// Favorability value unlocking bromide (Album open: 2nd check)
		vector<bool>	vbIsFoundAlbum;			// Check affinity and store whether the album can be viewed.
#endif
		S_AVAILALBUM_INFO( void ) : iReputeTableID( -1 ), iAlbumListOffset( -1 ) {};
	};

	S_ALBUM_ELEMENT m_aElements[ NPC_REPUTATION_ALBUM_COUNT_PER_PAGE ];

	CEtUIComboBox* m_pComboSort;
	CEtUIStatic* m_pStaticPage;
	CEtUIButton* m_pBtnPrevPage;
	CEtUIButton* m_pBtnNextPage;

#ifdef PRE_ADD_REPUTATION_EXPOSURE 
	CEtUIStatic* m_pStaticAchievePercent;
	EtTextureHandle m_hGrayScalePortraitTexture;
	
	vector<bool> m_vbIsFoundAlbum;		// Check affinity and store whether the album can be viewed.
	vector<int>	 m_vIBromideQuestID;	// Quest ID unlocking bromide acquisition (Album open: 1st check)
	vector<int>	 m_vIBromideQuestEnumID;// Quest Enum ID unlocking bromide acquisition (Album open: 1st check)
	vector<int>  m_vIBromideRepute;		// Favorability required to obtain bromide (Album open: 2nd check)
#endif

	int m_iNowPage;
	int m_iMaxPage;
	int m_iNowSelectedSortMethod;
	
	//vector<int> m_vlAvailNpcReputeTableIDs;
	// Bromide index of the NPC. Up to 3 are currently provided per NPC. The sum of this vector's counts is the number of bromides to be displayed.
	//vector<int> m_vlBromideIndices;
	vector<S_AVAILALBUM_INFO>	m_vlAvailAlbumInfos;
	vector<int>					m_vlReputeTableID;		// Table IDs of the currently acquired bromides.
	vector<int>					m_vlPortrait;			// Index of the portrait texture of the currently acquired bromide 
	vector<string>				m_vlBromideFileName;	// Texture name of the currently acquired bromide.
	EtTextureHandle				m_hPortraitTexture;

	// Bromide dialog that enlarges the small image when clicked.
	CDnNpcReputationBromideDlg* m_pBromideDlg;
	bool m_bInitialized;
	bool m_bShowFromBromide; // Returning from bromide to album. Keep the page that was being viewed right before.

#ifdef PRE_ADD_REPUTATION_EXPOSURE
	bool IsOpenBromide(int iBromideIndex);
	void ShowBromideHelpTooltip(float fX, float fY);
#endif

protected:
	void _UpdateAvailAlbumList( int iSortMethod ); 

public:
	CDnNpcReputationAlbumDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, 
							int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnNpcReputationAlbumDlg( void );

	void Initialize( bool bShow );
	void InitialUpdate(	void );
	void Show( bool bShow );
	void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ );

	void Clear( void );
	void UpdatePage( int iPage );
	void RefreshToCurrentPage( void ) { UpdatePage( m_iNowPage ); };

	void SetBromideDlg( CDnNpcReputationBromideDlg* pBromoideDlg ) { m_pBromideDlg = pBromoideDlg; };
	void ShowFromBromide( void ) { m_bShowFromBromide = true; };

	// Added mouse wheel handling [2010/11/16 semozz]
	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void RefreshPage(bool bInc);
}; 