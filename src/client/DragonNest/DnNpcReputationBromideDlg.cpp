#include "StdAfx.h"
#include "DnNpcReputationBromideDlg.h"
#include "DnNpcReputationAlbumDlg.h"

CDnNpcReputationBromideDlg::CDnNpcReputationBromideDlg( UI_DIALOG_TYPE dialogType /* = UI_TYPE_TOP_MSG */, CEtUIDialog *pParentDialog /* = NULL */, 
													   int nID /* = -1 */, CEtUICallback *pCallback /* = NULL */ ) : CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true ),
													   m_pBromideTextureCtrl( NULL ),
													   m_pStaticEventReceiver( NULL ),
													   m_pAlbumDlg(NULL)
{
	
}

CDnNpcReputationBromideDlg::~CDnNpcReputationBromideDlg(void)
{
}


void CDnNpcReputationBromideDlg::Initialize( bool bShow )
{
#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "Tab_ReputePictureDlg.ui" ).c_str(), bShow );
#else
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ReputePictureDlg.ui" ).c_str(), bShow );
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT
}

void CDnNpcReputationBromideDlg::InitialUpdate( void )
{
	m_pBromideTextureCtrl = GetControl<CEtUITextureControl>( "ID_TEXTUREL_PICTURE" );
	m_pStaticEventReceiver = GetControl<CEtUIStatic>( "ID_CLICK" );
	m_pStaticEventReceiver->SetButton( true );

#ifdef PRE_MOD_CORRECT_UISOUND_PLAY
	m_DlgInfo.bSound = false;
#endif 
}

void CDnNpcReputationBromideDlg::Show( bool bShow )
{
	CEtUIDialog::Show( bShow );

	if( m_bShow == bShow )
		return;
}

void CDnNpcReputationBromideDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	if( false == IsShow() )
		return;

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_CLICK" )  )
		{
			// Show the album dialog.
			m_pAlbumDlg->ShowFromBromide();
			m_pAlbumDlg->Show( true );
			this->Show( false );

			// Release the displayed bromide texture
			SAFE_RELEASE_SPTR( m_hSettedTexture );
		}
	}
}

void CDnNpcReputationBromideDlg::SetBromideTexture( EtTextureHandle hTexture )
{
	// Size is unknown yet, so set arbitrarily.
	if( m_pBromideTextureCtrl && hTexture )
	{
		m_pBromideTextureCtrl->SetTexture( hTexture, 0, 0, hTexture->Width(), hTexture->Height() );
		m_hSettedTexture = hTexture;
		m_pBromideTextureCtrl->Show( true );
	}
	else
		m_pBromideTextureCtrl->Show( false );
}

void CDnNpcReputationBromideDlg::SetAlbumDlg( CDnNpcReputationAlbumDlg* pAlbumDlg )
{
	m_pAlbumDlg = pAlbumDlg;
}