#include "StdAfx.h"
#include "LoginClientSession.h"
#include "ClientSessionManager.h"
#include "DnLoginTask.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CLoginClientSession::CLoginClientSession() : CTaskListener(false)
{
}

CLoginClientSession::~CLoginClientSession()
{
}

void CLoginClientSession::RequestLogin( const TCHAR *szID, const TCHAR *szPass )
{
	WCHAR szLocalIP[ 32 ];
	CClientSessionManager::GetInstance().GetLocalIP( szLocalIP );

	SendLogin( szID, szPass, szLocalIP );
	FUNC_LOG();
}

void CLoginClientSession::OnConnectTcp()
{
	OutputDebug( "OnConnect\n" );
	FUNC_LOG();
}

void CLoginClientSession::OnDisconnectTcp( bool bValidDisconnect )
{
	if( !bValidDisconnect ) {
		OutputDebug( "OnDisconnect\n" );
		FUNC_LOG();
	}
}

void CLoginClientSession::OnDispatchMessage(int iMainCmd, int iSubCmd, char * pData, int iLen)
{
	switch(iMainCmd) {
		case SC_LOGIN:	OnRecvLoginMessage( iSubCmd, pData, iLen ); break;
		case SC_SYSTEM: OnRecvSystemMessage( iSubCmd, pData, iLen ); break;
#ifdef PRE_ADD_DOORS
		case SC_DOORS: OnRecvDoorsMessage(iSubCmd, pData, iLen); break;
#endif
	}
}

void CLoginClientSession::OnRecvLoginMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eLogin::SC_CHECKVERSION: OnRecvLoginCheckVersion( (SCCheckVersion *)pData ); break;
		case eLogin::SC_CHECKLOGIN: OnRecvLoginCheckLogin( (SCCheckLogin*)pData ); break;
		case eLogin::SC_CHARLIST: OnRecvLoginCharList( (SCCharList*)pData ); break;
		case eLogin::SC_WAITUSER: OnRecvLoginWaitUser( (SCWaitUser*)pData ); break;
		case eLogin::SC_CHECKBLOCK: OnRecvLoginBlock( (SCCHeckBlock*)pData ); break;
		case eLogin::SC_CREATECHAR: OnRecvLoginCharCreate( (SCCreateChar *)pData ); break;
		case eLogin::SC_CREATEDWCCHAR: OnRecvLoginCharCreate( (SCCreateChar *)pData ); break;
		case eLogin::SC_DELETECHAR: OnRecvLoginCharDelete( (SCDeleteChar *)pData ); break;
		case eLogin::SC_SERVERLIST: OnRecvLoginServerList( (SCServerList *)pData ); break;
		case eLogin::SC_CHANNELLIST: OnRecvLoginChannelList( (SCChannelList *)pData ); break;
		case eLogin::SC_BACKBUTTON_LOGIN: OnRecvLoginBackButton(); break;
		case eLogin::SC_REVIVECHAR: OnRecvLoginReviveChar( (SCReviveChar*)pData ); break;

	}
}

void CLoginClientSession::OnRecvSystemMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eSystem::SC_GAMEINFO: OnRecvSystemGameInfo( (SCGameInfo *)pData ); break;
		case eSystem::SC_VILLAGEINFO: OnRecvSystemVillageInfo( (SCVillageInfo*)pData ); break;
	}
}
#ifdef PRE_ADD_DOORS
void CLoginClientSession::OnRecvDoorsMessage(int nSubCmd, char *pData, int nSize)
{
	switch(nSubCmd)
	{
	case eDoors::SC_GET_AUTHKEY: OnRecvDoorsAuthKey((SCDoorsGetAuthKey*)pData); break;
	case eDoors::SC_CANCEL_AUTH: OnRecvDoorsCancelAuth((SCDoorsCancelAuth*)pData); break;
	case eDoors::SC_GET_MOBILEAUTHFLAG: OnRecvDoorsAuthFlag((SCDoorsGetAuthFlag*)pData); break;
	default: _ASSERT(0);
	}
}
#endif