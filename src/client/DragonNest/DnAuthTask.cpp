	
#include "stdafx.h"
#include "DnInterface.h"
#include "GameOptionSendPacket.h"
#include "DnItemTask.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnAuthTask.h"
#include "DnGaugeDlg.h"
#ifdef PRE_ADD_VIP
#include "SystemSendPacket.h"
#endif


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 



CDnAuthTask::CDnAuthTask()
: CTaskListener( true )
, m_bIsSetSecondAuthLock( false )
, m_bIsSetSecondAuthPW( false )
, m_bIsSecondAuthNotifyShowInfo( false )
, m_FailCount( 0 )
, m_tSecondAuthResetDate( -1 )
#if defined(PRE_ADD_23829)
, m_bIsShow2ndPassCreateButton(false)
, m_bIgnore2ndPassNotify(false)
#endif // PRE_ADD_23829
{
}

CDnAuthTask::~CDnAuthTask()
{
}

//Must stay alive across login and village server; acts almost like a bridge and does not die until game exits. 
//With login session info
//Character select
//When in village
void CDnAuthTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
}

void CDnAuthTask::OnDispatchMessage(int nMainCmd, int nSubCmd, char *pData, int nSize)
{
	switch (nMainCmd)
	{
		case SC_GAMEOPTION: RecvOptionMessage(nSubCmd , pData , nSize ); break; 
	}
}

void CDnAuthTask::RecvOptionMessage( int nSubCmd, char *pData, int nSize )
{
	switch(nSubCmd)
	{
		case eGameOption::SC_NOTIFY_SECONDAUTHINFO:			OnRecvSecondAuthInfo( (SCGameOptionNotifySecondAuthInfo*)pData); break; 
		case eGameOption::SC_UPDATE_SECONDAUTH_PASSWORD:	OnRecvSecondAuthPassword( (SCGameOptionUpdateSecondAuthPassword*)pData);break; 
		case eGameOption::SC_UPDATE_SECONDAUTH_LOCK:		OnRecvSecondAuthLock( (SCGameOptionUpdateSecondAuthLock*)pData );	break; 
		case eGameOption::SC_NOTIFY_SECONDAUTH_RESTRAINT:	OnRecvSecondAuthTradeRestraint(); break; 
		case eGameOption::SC_INIT_SECONDAUTH:				OnRecvSecondAuthInit((SCGameOptionInitSecondAuth*)pData); break; 
		case eGameOption::SC_GAMEOPTION_VALIDATE_SECONDAUTH: OnRecvSecondAuthValidate((SCGameOptionValidateSecondAuth*)pData); break;
	}
}

//Retrieve lock info and password info. 
void CDnAuthTask::OnRecvSecondAuthInfo(SCGameOptionNotifySecondAuthInfo *pPacket)
{
	//bool bIsSetSecondAuthPW; // Whether 2nd auth password is set
	//bool bIsSetSecondAuthLock; // Whether 2nd auth account lock is set
	SetSecondAuthLock(pPacket->bIsSetSecondAuthLock);
	SetSecondAuthPw(pPacket->bIsSetSecondAuthPW);
	//Reset count
	m_FailCount = 0;
	m_tSecondAuthResetDate = pPacket->tSecondAuthResetDate;

#if defined(PRE_ADD_23829)

	// Whether 2nd password create button is enabled [2010/12/27 semozz]
	// Hide if bCompletlySecondAuthPassFlag is true.
	bool show2ndPassCreateButton = false;
#if defined(PRE_ADD_IGNORESECONDAUTH_EKEYECARD)
	show2ndPassCreateButton = !pPacket->bCompletlySecondAuthPassFlag;

	//Always disable the 2nd password create button if a 2nd password exists
	if (pPacket->bIsSetSecondAuthPW)
		show2ndPassCreateButton = false;
	else
	{
		//2nd password is not set, and
		//If the 2nd password create button is disabled and the 2nd password verify window is also disabled,
		if (!show2ndPassCreateButton && pPacket->bSecondAuthPassFlag)
			show2ndPassCreateButton = true;
	}
#endif

	SetSecondAuthNotifyShow(!pPacket->bSecondAuthPassFlag, show2ndPassCreateButton);
#endif // PRE_ADD_23829
}
//Count when password change succeeds or fails
void CDnAuthTask::OnRecvSecondAuthPassword(SCGameOptionUpdateSecondAuthPassword *pPacket)
{
	//int		iRet;	
	//char cFailCount; Fail count
	/*
ERROR_SECONDAUTH_CHECK_MAXFAILED = 101125, // Error due to exceeding consecutive max failure count.
ERROR_SECONDAUTH_CHECK_OLDPWINVALID = 101126, // Existing secondary auth password does not match.
ERROR_SECONDAUTH_CHECK_FAILED = 101127, // Secondary auth password does not match.
	*/
	WCHAR wzStrTmp[1024]={0,};

	switch( pPacket->iRet)	
	{
	case ERROR_NONE: 
		{
			CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask("LoginTask");
			if(!m_bIsSetSecondAuthPW) 
			{
				if( pTask ) GetInterface().MessageBox( 6302 );					//Create password 
				else GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6302), true );
			}
			else 
			{
				if( pTask ) GetInterface().MessageBox( 6436 );		//_T("2nd password has been changed.")
				else GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"",GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6436) , true );
			}
			break; 
		} 
	case ERROR_SECONDAUTH_CHECK_OLDPWINVALID:
	case ERROR_SECONDAUTH_CHECK_FAILED:
		{
			swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6303) , pPacket->cFailCount , SecondAuth::Common::LimitCount );
			GetInterface().MessageBox( wzStrTmp ) ;	
			break; 
		}
			
	}
	m_FailCount  =  pPacket->cFailCount;
}

//When unlocking .. failure count
void CDnAuthTask::OnRecvSecondAuthLock(SCGameOptionUpdateSecondAuthLock *pPacket)
{
	WCHAR wzStrTmp[1024]={0,};
	switch( pPacket->iRet)	
	{
	case ERROR_NONE: 
		{
			CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask("LoginTask");

			if( !m_bIsSetSecondAuthLock )
			{
				if( pTask )	GetInterface().MessageBox( 6437 ) ;	//HAN_MSG
				else GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6437)  , true );	
			}
			else 
			{
				if( pTask )	GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6308 ) );		//	 L"The lock has been released."	
				else GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6308 ) , true );
			}
			break; 
		} 
	case ERROR_SECONDAUTH_CHECK_OLDPWINVALID:
	case ERROR_SECONDAUTH_CHECK_FAILED:
		{
			swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6303) , pPacket->cFailCount , SecondAuth::Common::LimitCount );
			GetInterface().MessageBox( wzStrTmp ) ;	
			break; 
		}
	}

	m_FailCount  =  pPacket->cFailCount;
}

void CDnAuthTask::OnRecvSecondAuthTradeRestraint()
{
	GetItemTask().SetRequestWait(false);
	GetInterface().MessageBox( AUTH_MAINMSG, MB_OK, -1, NULL, true, false, false, true );
}

void CDnAuthTask::OnRecvSecondAuthInit(SCGameOptionInitSecondAuth *pPacket)
{
	WCHAR wzStrTmp[1024]={0,};

	switch( pPacket->iRet)	
	{
	case ERROR_NONE: 
		{
			CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask("LoginTask");
			if(!m_bIsSetSecondAuthPW) 
			{
				if( pTask ) GetInterface().MessageBox( 6446  );					//Password has been cleared. 
				else GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6446), true );
			}
			else 
			{
				if( pTask ) GetInterface().MessageBox( 6436 );		//_T("2nd password has been changed.")
				else GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"",GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6436) , true );
			}
			break; 
		} 
	case ERROR_SECONDAUTH_CHECK_OLDPWINVALID:
	case ERROR_SECONDAUTH_CHECK_FAILED:
		{
			swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6303) , pPacket->cFailCount , SecondAuth::Common::LimitCount );
			GetInterface().MessageBox( wzStrTmp ) ;	
			break; 
		}
	}
}

void CDnAuthTask::SetSecondAuthNotifyShow(bool bFlag, bool bShow2ndPassCreateButton)
{
	m_bIsSecondAuthNotifyShowInfo = bFlag;
#if defined(PRE_ADD_23829)
	m_bIsShow2ndPassCreateButton = bShow2ndPassCreateButton;
	GetInterface().Show2ndPassCreateButton(bShow2ndPassCreateButton);
#endif
}

void CDnAuthTask::RequestCreateAuthPW( int nSeed, int nValue[] )
{
	int nTemp[4] = { 0, };
	SendSecondAuthPassword( 0, nTemp, nSeed, nValue );
}

void CDnAuthTask::RequestChangeAuthPW( int nOldSeed, int nOldValue[], int nNewSeed, int nNewValue[] )
{
	if( m_FailCount >= SecondAuth::Common::LimitCount)
	{
		//6350
		WCHAR wzStr[1024] = {0,};
		swprintf_s( wzStr , _countof(wzStr) ,   GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6426)  , SecondAuth::Common::LimitCount , SecondAuth::Common::RestraintTimeMin  ) ; //HAN_MSG
		GetInterface().MessageBox( wzStr ); //HAN_MSG
		return;
	}

	SendSecondAuthPassword( nOldSeed, nOldValue, nNewSeed, nNewValue );
}

void CDnAuthTask::RequestAuthLock(bool bLock , int nSeed, int nValue[])
{
	if( m_FailCount >= SecondAuth::Common::LimitCount)
	{
		WCHAR wzStr[1024] = {0,};
		//6350
		swprintf_s( wzStr , _countof(wzStr) , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6426) , SecondAuth::Common::LimitCount  , SecondAuth::Common::RestraintTimeMin ) ; //HAN_MSG
		GetInterface().MessageBox( wzStr ); //HAN_MSG
		return;
	}

	SendSecondAuthLock(bLock , nSeed, nValue);
}

void CDnAuthTask::RequestAuthUnLock(bool bLock , int nSeed, int nValue[])
{
	SendSecondAuthLock(bLock , nSeed, nValue);
}

void CDnAuthTask::RequestAuthInit(int nSeed, int nValue[])
{
	SendSecondAuthInit(nSeed, nValue);
}




void CDnAuthTask::RequestAuthNotifyDate()
{
	SendSecondAuthNotifyDate();
	//Sending to server means hiding it for 3 days..
	m_bIsSecondAuthNotifyShowInfo = false; 
}

bool CDnAuthTask::CheckAuthLock()
{
/*
	if( m_bIsSetSecondAuthLock )
	{
//		GetInterface().MessageBox( _T("Cannot use while locked. Please unlock at the top of the character screen.") ); //HAN_MSG	
		return TRUE;
	}
*/	
	return FALSE;
}

void CDnAuthTask::OpenMsgBoxAuthUnLock()
{
	GetInterface().MessageBox( 6467 , MB_YESNO , AUTHUNLOCK , this ); //HAN_MSG	
}

void CDnAuthTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg)
{
	switch( nID ) 
	{
	case AUTHUNLOCK:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( strcmp( pControl->GetControlName(), "ID_YES" ) == 0 ) 
				{
					int nValue[4] = { 0, };
					RequestAuthLock(true , 0, nValue);	
				}
			}
		}
	}
}

void CDnAuthTask::RequestValidateAuth(int nSeed, int nValue[], int authCheckType)
{
	SendSecondAuthValidate(nSeed, nValue, authCheckType);
}

void CDnAuthTask::OnRecvSecondAuthValidate(SCGameOptionValidateSecondAuth* pData)
{
	WCHAR wzStrTmp[1024]={0,};
	switch(pData->nRet)
	{
	case ERROR_NONE: 
		{
#ifdef PRE_ADD_VIP
			if (pData->nAuthCheckType == Validate_VIP)
				SendVIPAutomaticPay(false);
#endif
			break;
		}

	case ERROR_SECONDAUTH_CHECK_FAILED:
		{
			swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 6303) , pData->cFailCount , SecondAuth::Common::LimitCount);
			GetInterface().MessageBox(wzStrTmp);
			break;
		}
	}

	m_FailCount  =  pData->cFailCount;
}
