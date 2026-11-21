#pragma once
#include "EtUIDialog.h"

class CDnNpcReputationAlbumDlg;

class CDnNpcReputationBromideDlg : public CEtUIDialog,
								   public CEtUICallback
{
private:
	CDnNpcReputationAlbumDlg* m_pAlbumDlg;		// Clicking the large image returns to the album.
	
	CEtUITextureControl* m_pBromideTextureCtrl;
	CEtUIStatic* m_pStaticEventReceiver;
	EtTextureHandle m_hSettedTexture;

public:
	CDnNpcReputationBromideDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_TOP_MSG, CEtUIDialog *pParentDialog = NULL, 
								int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnNpcReputationBromideDlg( void );

	void Initialize( bool bShow );
	void InitialUpdate( void );
	void Show( bool bShow );
	void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ );

	void SetBromideTexture( EtTextureHandle hTexture );

	void SetAlbumDlg( CDnNpcReputationAlbumDlg* pAlbumDlg );
};
