#include "StdAfx.h"
#include "DnSkillResetDlg.h"
#include "DnItem.h"
#include "DnSkillTask.h"
#include "boost/scoped_array.hpp"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSkillResetDlg::CDnSkillResetDlg( UI_DIALOG_TYPE dialogType /* = UI_TYPE_FOCUS */, CEtUIDialog *pParentDialog /* = NULL */, 
								    int nID /* = -1 */, CEtUICallback *pCallBack /* = NULL */, bool bAutoCursor )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallBack, bAutoCursor )
{
	m_iStartJobDegree = 0;
	m_iEndJobDegree = 0;
	m_iRewardSP = 0;
	m_biItemSerial = 0;
	m_pButtonCancel = NULL;
	m_pButtonOK = NULL;
	m_pResetExplain = NULL;
	m_pRewardSP = NULL;
	m_pSkillPage = NULL;
}

CDnSkillResetDlg::~CDnSkillResetDlg( void )
{

}


void CDnSkillResetDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillInit.ui" ).c_str(), bShow );
}


void CDnSkillResetDlg::InitialUpdate()
{
	m_pResetExplain = GetControl<CEtUIStatic>( "ID_STATIC0" );
	m_pRewardSP = GetControl<CEtUIStatic>( "ID_STATIC2" );

	m_pButtonOK = GetControl<CEtUIButton>("ID_BUTTON_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");

	m_pSkillPage = GetControl<CEtUIStatic>("ID_SKILL_PAGE");
}


void CDnSkillResetDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_SmartMove.MoveCursor();
	}
	else
	{
		m_SmartMove.ReturnCursor();
	}

	CEtUIDialog::Show( bShow );
}


void CDnSkillResetDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BUTTON_OK"))
		{
			// Send actual skill reset request to server.
			//GetPartyTask().ReqPartyJoinGetReversionItem(true);

			// Reset immediately for client code testing.
			//GetSkillTask().ResetSkillsByJob( m_iStartJobDegree, m_iEndJobDegree, m_iRewardSP );
			
			// Send final packet to server indicating skill reset cash item usage.
			if( GetSkillTask().IsRequestWait() ) return;
#ifdef PRE_MOD_SKILLRESETITEM
			GetSkillTask().SendUseSkillResetCashItemReq(m_biItemSerial, m_iStartJobDegree, m_iEndJobDegree, m_iItemInvenTypeCache, m_iItemInvenIndexCache);
#else
			GetSkillTask().SendUseSkillResetCashItemReq( m_biItemSerial, m_iStartJobDegree, m_iEndJobDegree );
#endif
		}

		//if (IsCmdControl("ID_CANCEL"))
		
		Show(false);
	}

	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}



#ifdef PRE_MOD_SKILLRESETITEM
void CDnSkillResetDlg::UpdateContent(const INT64 biItemSerial, const int iStartJobDegree, const int iEndJobDegree, const int iRewardSP, const int iInvenType, const int iInvenIndex)
#else
void CDnSkillResetDlg::UpdateContent( INT64 biItemSerial, int iStartJobDegree, int iEndJobDegree, int iRewardSP )
#endif
{
	// When resetting skills for only one class tier.
	// Are you sure you want to reset tier %d class skills? (114056)
	// Are you sure you want to reset tier %d ~ %d class skills?
	// Skills of tier %d ~ %d classes will be reset and refunded as SP.
	// Once reset, it cannot be undone.
	// Skills of tier %d class will be reset and refunded as SP. (114024)
	// When resetting skills across 2 or more job tiers by range.

	if( iStartJobDegree == iEndJobDegree )
	{
		const wchar_t* pFormatMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114024 );
		wchar_t awcBuf[ 1024 ] = { 0 };
		swprintf_s( awcBuf, _countof(awcBuf), pFormatMsg, iStartJobDegree, iStartJobDegree );
		m_pResetExplain->SetText( awcBuf );

		// %d SP (114055)
		pFormatMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114055 );
		swprintf_s( awcBuf, _countof(awcBuf), pFormatMsg, iRewardSP );
		m_pRewardSP->SetText( awcBuf );

		//boost::scoped_array<wchar_t> pBuffer( new wchar_t[ wcslen(pFormatMsg)+1 ] );

		//swprintf_s( pBuffer.get(), wcslen(pBuffer.get()), pFormatMsg, iStartJobDegree, iStartJobDegree );
		//m_pResetExplain->SetText( pBuffer.get() );

		//// %d SP (114055)
		//pFormatMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114055 );
		//swprintf_s( pBuffer.get(), wcslen(pBuffer.get()), pFormatMsg, iRewardSP );
		//m_pRewardSP->SetText( pBuffer.get() );
	}
	else
	{
		const wchar_t* pFormatMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114056 );
		wchar_t awcBuf[ 1024 ] = { 0 };
		swprintf_s( awcBuf, _countof(awcBuf), pFormatMsg, iStartJobDegree, iEndJobDegree, iStartJobDegree, iEndJobDegree );
		m_pResetExplain->SetText( awcBuf );

		// %d SP (114055)
		pFormatMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114055 );
		swprintf_s( awcBuf, _countof(awcBuf), pFormatMsg, iRewardSP );
		m_pRewardSP->SetText( awcBuf );

		//boost::scoped_array<wchar_t> pBuffer( new wchar_t[ wcslen(pFormatMsg)+1 ] );

		//swprintf_s( pBuffer.get(), wcslen(pBuffer.get()), pFormatMsg, iStartJobDegree, iEndJobDegree, iStartJobDegree, iEndJobDegree );
		//m_pResetExplain->SetText( pBuffer.get() );

		//// %d SP (114055)
		//pFormatMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114055 );
		//swprintf_s( pBuffer.get(), wcslen(pBuffer.get()), pFormatMsg, iRewardSP );
		//m_pRewardSP->SetText( pBuffer.get() );
	}

	if( GetSkillTask().GetCurrentSkillPage() == DualSkill::Type::Primary )
		m_pSkillPage->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1157 ));
	else
		m_pSkillPage->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1158 ));

	m_biItemSerial = biItemSerial;
	m_iStartJobDegree = iStartJobDegree;
	m_iEndJobDegree = iEndJobDegree;
	m_iRewardSP = iRewardSP;

#ifdef PRE_MOD_SKILLRESETITEM
	m_iItemInvenTypeCache = iInvenType;
	m_iItemInvenIndexCache = iInvenIndex;
#endif
}