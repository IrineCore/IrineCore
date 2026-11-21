#include "StdAfx.h"
#include "DnPVPUserToolTipDlg.h"
#include "DnInterface.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPVPUserToolTipDlg::CDnPVPUserToolTipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
,m_pPVPLevel(NULL)
,m_pPVPIcon(NULL)
,m_pScore(NULL)
,m_pKillPoint(NULL)
,m_pAssistPoint(NULL)
,m_pSupportPoint(NULL)
,m_pTotalXP(NULL)
{
	
	
}

CDnPVPUserToolTipDlg::~CDnPVPUserToolTipDlg(void)
{

}

void CDnPVPUserToolTipDlg::InitialUpdate()
{
	m_pPVPLevel = GetControl<CEtUIStatic>("ID_RANK");
	m_pPVPIcon = GetControl<CEtUITextureControl>("ID_RANK_TEXTURE");
	m_pScore = GetControl<CEtUIStatic>("ID_SCORE");
	m_pKillPoint = GetControl<CEtUIStatic>("ID_KO");
	m_pAssistPoint = GetControl<CEtUIStatic>("ID_ASSIST");
	m_pSupportPoint = GetControl<CEtUIStatic>("ID_SUPPORT");
	m_pTotalXP = GetControl<CEtUIStatic>("ID_GETXP");

}

void CDnPVPUserToolTipDlg::SetData( byte cPVPLevel , int iTotalScore , int iKillPoint , int iAssistPoint , int iSuppotPoint , int iTotalXP)
{
	WCHAR wszTemp[20];

	if( GetInterface().GetPVPIconTex() )
	{
		int iIconW,iIconH;
		int iU,iV;


		iIconW = GetInterface().GeticonWidth();
		iIconH = GetInterface().GeticonHeight();

		if( GetInterface().ConvertPVPGradeToUV( cPVPLevel ,iU, iV ))
		{
			m_pPVPIcon->SetTexture(GetInterface().GetPVPIconTex(),iU, iV ,iIconW,iIconH );
			m_pPVPIcon->Show(true);
		}

	}

	SecureZeroMemory(wszTemp,sizeof(wszTemp));
	wsprintf(wszTemp,L"%d",cPVPLevel );
	m_pPVPLevel->SetText(wszTemp);

	SecureZeroMemory(wszTemp,sizeof(wszTemp));
	wsprintf(wszTemp,L"%d",iTotalScore );
	m_pScore->SetText(wszTemp);

	SecureZeroMemory(wszTemp,sizeof(wszTemp));
	wsprintf(wszTemp,L"%d",iKillPoint );
	m_pKillPoint->SetText(wszTemp);

	SecureZeroMemory(wszTemp,sizeof(wszTemp));
	wsprintf(wszTemp,L"%d",iAssistPoint );
	m_pAssistPoint->SetText(wszTemp);

	SecureZeroMemory(wszTemp,sizeof(wszTemp));
	wsprintf(wszTemp,L"%d",iSuppotPoint );
	m_pSupportPoint->SetText(wszTemp);

	SecureZeroMemory(wszTemp,sizeof(wszTemp));
	wsprintf(wszTemp,L"%d",iTotalXP );
	m_pTotalXP->SetText(wszTemp);
}

void CDnPVPUserToolTipDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpInform.ui" ).c_str(), bShow );	
}
