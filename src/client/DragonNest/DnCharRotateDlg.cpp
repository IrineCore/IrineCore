#include "StdAfx.h"
#include "DnCharRotateDlg.h"
#include "DnLoginTask.h"
#include "DnBridgeTask.h"
#include "TaskManager.h"
#include "DnMessageBox.h"
#include "DnInterface.h"

#ifdef PRE_ADD_GAMEQUIT_REWARD
#include "DnCharSelectDlg.h"
#include "DnCharSelectListDlg.h"
#endif // PRE_ADD_GAMEQUIT_REWARD


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#ifdef PRE_MOD_SELECT_CHAR

CDnCharRotateDlg::CDnCharRotateDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pButtonRotateLeft( NULL )
, m_pButtonRotateRight( NULL )
, m_pButtonAvatarViewArea( NULL )
, m_fAvatarViewMouseX( 0.0f )
, m_fAvatarViewMouseY( 0.0f )
{
}

CDnCharRotateDlg::~CDnCharRotateDlg()
{
}

void CDnCharRotateDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharCreate_CharPreviewDlg.ui" ).c_str(), bShow );
}

void CDnCharRotateDlg::InitialUpdate()
{
	m_pButtonRotateLeft = GetControl<CEtUIButton>( "ID_BT_LEFT" );
	m_pButtonRotateRight = GetControl<CEtUIButton>( "ID_BT_RIGHT" );
	m_pButtonAvatarViewArea = GetControl<CEtUIButton>( "ID_BT_PREVIEW" );

	CEtUIStatic * pStatic = GetControl<CEtUIStatic>("ID_TEXT_RETURNTITLE");
#ifdef PRE_ADD_NEWCOMEBACK
	m_pStaticComeback = pStatic;
	m_pStaticComeback->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7899 ) ); // "* Returnee Character *"
	m_pStaticComeback->Show( false );

	m_crrTextColor = m_TextColor = EtColor( m_pStaticComeback->GetTextColor() );		
	m_sign = 1.0f;
#else
	if( pStatic )
		pStatic->Show( false );
#endif // PRE_ADD_NEWCOMEBACK

	
}

void CDnCharRotateDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
#ifdef PRE_ADD_NEWCOMEBACK
		m_sign = 1.0f;
		m_crrTextColor = m_TextColor;
#endif // PRE_ADD_NEWCOMEBACK

	}
	else
	{
#ifdef PRE_ADD_NEWCOMEBACK
		m_pStaticComeback->SetTextColor( m_TextColor ); // Restore original color.
#endif // PRE_ADD_NEWCOMEBACK

	}
}

void CDnCharRotateDlg::Process( float fDelta )
{
	CEtUIDialog::Process( fDelta );

	if( !IsShow() ) return;

	CDnLoginTask *pLoginTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pLoginTask ) return;

	static float fRotScale = 200.f;

	if( m_pButtonRotateLeft->IsPressed() )
		pLoginTask->CharacterAddRotateYaw( -fDelta * fRotScale );
	else if( m_pButtonRotateRight->IsPressed() )
		pLoginTask->CharacterAddRotateYaw( fDelta * fRotScale );
	else if( m_pButtonAvatarViewArea->IsPressed() )
	{
		float fX = m_pButtonAvatarViewArea->GetMouseCoord().fX;
		float fY = m_pButtonAvatarViewArea->GetMouseCoord().fY;
		if( m_fAvatarViewMouseX != 0.f && m_fAvatarViewMouseY != 0.f )
		{
			static float fMoveScale = 500.f;
			float fAddAngle = fMoveScale * sqrtf( (m_fAvatarViewMouseX-fX)*(m_fAvatarViewMouseX-fX)+(m_fAvatarViewMouseY-fY)*(m_fAvatarViewMouseY-fY) ) * (((m_fAvatarViewMouseX-fX)>0.f)? 1.f : -1.f);
			pLoginTask->CharacterAddRotateYaw( -fAddAngle );
		}
		m_fAvatarViewMouseX = fX;
		m_fAvatarViewMouseY = fY;
	}
	else
	{
		m_fAvatarViewMouseX = 0.f;
		m_fAvatarViewMouseY = 0.f;
		focus::SetFocus();
	}

#ifdef PRE_ADD_NEWCOMEBACK	
	
	if( m_pStaticComeback->IsShow() )
	{
		if( m_crrTextColor.a >= 1.0f )
			m_sign = -1.0f;
		else if( m_crrTextColor.a <= 0.0f )
			m_sign = 1.0f;

		m_crrTextColor.a += m_sign * fDelta * 0.5f;

		if( m_crrTextColor.a > 1.0f )
			m_crrTextColor.a = 1.0f;
		else if( m_crrTextColor.a < 0.0f )
			m_crrTextColor.a = 0.0f;

		m_pStaticComeback->SetTextColor( m_crrTextColor );
	}
#endif // PRE_ADD_NEWCOMEBACK


}

bool CDnCharRotateDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pTask ) 
		return false;
	
	if( pTask->GetState() == CDnLoginTask::CharSelect && uMsg == WM_LBUTTONDBLCLK )
	{
		if( !IsMouseInDlg() ) return false;
		POINT MousePoint;
		SUICoord uiCoordsBase[2];

		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );

		float fMouseX, fMouseY;
		PointToFloat( MousePoint, fMouseX, fMouseY );

		if( m_pButtonAvatarViewArea->IsInside( fMouseX, fMouseY ) ) 
		{
			if( pTask->GetSelectActorSlot() == -1 ) 
			{
				GetInterface().MessageBox( MESSAGEBOX_11, MB_OK, MESSAGEBOX_11, pTask );
				return false;
			}

			if( pTask->CheckSelectCharDeleteWait() )
				return false;

#ifdef PRE_ADD_COMEBACK
			CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
	
			if( pTask->GetComeback() )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7740 ), MB_OKCANCEL, 1, this ); // "Are you sure you want to receive it on this character? (Cannot be changed once selected. Please choose carefully.)"						
			}
			//else
			//	GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login , 0 );
	#ifdef PRE_ADD_GAMEQUIT_REWARD
			else
			{
				bool bNormalUser = true;
				CTask * _pTask = CTaskManager::GetInstance().GetTask( "BridgeTask" );
				if( pTask )
				{
					CDnBridgeTask *pBridgeTask = (CDnBridgeTask *)_pTask;

					// Can claim reconnect reward.
					CDnCharSelectDlg * pDlg = GetInterface().GetCharSelectDlg();
					if( pDlg && pBridgeTask->GetAccountRegTime() && pBridgeTask->GetNewbieReward() )				
					{
						CDnCharSelectListDlg * pDlgList = pDlg->GetCharSelectListDlg();					
						if( pDlgList )
						{				
							bNormalUser = false;
							GetInterface().ShowGameQuitRewardComboDlg( true );
							GetInterface().SelectedGameQuitRewardCharIndex( pDlgList->GetSelectCharIndex() );
						}					
					}	
				}


				// Normal account that is neither [New Account Reconnect Reward], [Returnee], nor [DWC Character].
				if( bNormalUser )
				{ 
#if defined(PRE_ADD_23829)
					SecondPassCheck(m_pCheckBox ? m_pCheckBox->IsChecked() : false);
#endif // PRE_ADD_23829

#ifdef PRE_MDD_USA_SECURITYNUMPAD
				// For US, secondary password entry is mandatory. 
				Show(false);
#else
				CDnLoginTask *pLoginTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask("LoginTask");
				int nValue[4] = { 0, };
				pLoginTask->SetAuthPassword(0, nValue);
				pLoginTask->ChangeState(CDnLoginTask::LoginStateEnum::ChannelList);
				//SelectChannel();

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
				pLoginTask->ResetPreviewCashCostume();
#endif

#ifdef PRE_ADD_SETCHECK
				SetCheckShow(m_pCheckBox->IsChecked());
#endif 

#endif // PRE_MDD_USA_SECURITYNUMPAD
				Show(false);
			}
		}
	#else	// PRE_ADD_GAMEQUIT_REWARD			
			else
				GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login , 0 );
	#endif	// PRE_ADD_GAMEQUIT_REWARD

#else	// PRE_ADD_COMEBACK
			GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login , 0 );
#endif	// PRE_ADD_COMEBACK
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnCharRotateDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{	
#ifdef PRE_ADD_COMEBACK
	case 1:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) {
				if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
				{
					GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login , 0 );
				}
			}
		}
		break;
#endif
	default:
		break;
	}
}

#ifdef PRE_ADD_NEWCOMEBACK
void CDnCharRotateDlg::SetComebackUser( bool bComeback )
{
	if( bComeback )
	{
		m_crrTextColor.a = 0.0f;
		m_pStaticComeback->SetTextColor( m_crrTextColor );
	}
	m_pStaticComeback->Show( bComeback );
}
#endif // PRE_ADD_NEWCOMEBACK



#endif // PRE_MOD_SELECT_CHAR