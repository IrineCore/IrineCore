#include "StdAfx.h"
#include "DnMainQuestAnswerDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMainQuestAnswerDlg::CDnMainQuestAnswerDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pTextureImage( NULL )
{
}

CDnMainQuestAnswerDlg::~CDnMainQuestAnswerDlg()
{
	SAFE_RELEASE_SPTR( m_hImageTexture );
}

void CDnMainQuestAnswerDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "NpcAnswerDlg.ui" ).c_str(), bShow );
}

void CDnMainQuestAnswerDlg::InitialUpdate()
{
	m_pTextureImage = GetControl<CEtUITextureControl>( "ID_TEXTUREL_IMAGE" );
}

void CDnMainQuestAnswerDlg::Show( bool bShow )
{
	CEtUIDialog::Show( bShow );
}

void CDnMainQuestAnswerDlg::SetAnswerImage( const char *szFileName )
{
	if( !m_pTextureImage || !szFileName || szFileName[0] == '\0' )
		return;

	SAFE_RELEASE_SPTR( m_hImageTexture );
	m_hImageTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), RT_TEXTURE );
	if( m_hImageTexture )
		m_pTextureImage->SetTexture( m_hImageTexture, 0, 0, m_hImageTexture->Width(), m_hImageTexture->Height() );
}
