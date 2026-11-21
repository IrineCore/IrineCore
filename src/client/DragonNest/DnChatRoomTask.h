#pragma once

#include "Task.h"
#include "MessageListener.h"

class CDnChatRoomTask : public CTask, public CTaskListener, public CEtUICallback, public CSingleton<CDnChatRoomTask>
{
	enum
	{
		CHATROOM_PASSWORD_DIALOG,
	};

public:
	CDnChatRoomTask();
	virtual ~CDnChatRoomTask();

protected:
	void OnRecvChatRoomMessage( int nSubCmd, char *pData, int nSize );
	void OnRecvRoomMessage( int nSubCmd, char *pData, int nSize );

	void OnRecvChatRoomCreateRoom( SCCreateChatRoom *pPacket );
	void OnRecvChatRoomView( SCChatRoomView *pPacket );
	void OnRecvChatRoomLeave( SCChatRoomLeaveUser *pPacket );
	void OnRecvChatRoomEnterRoom( SCChatRoomEnterRoom *pPacket );
	void OnRecvChatRoomEnterUser( SCChatRoomEnterUser *pPacket );
	void OnRecvChatRoomChangeRoomOption( SCChatRoomChangeRoomOption *pPacket );
	void OnRecvChatRoomKickUser( SCChatRoomKickUser *pPacket );
	void OnRecvRoomSyncChatRoomInfo( SCROOM_SYNC_CHATROOMINFO *pPacket );

	TChatRoomView m_ChatRoomView;	// Hold the room created by self (or the room currently in?)
	int m_nChatRoomAllow;
	std::wstring m_wszPassword;

	// Unlike guild view, room view is too large so it was not included in EnterUser.
	// So decided not to put ChatRoomView in the player actor at all.
	// As a result, when a character entered view while moving, this room view arrived before EnteredCallback finished processing and loading, causing it to be skipped.
	// So if the actor is not found, enqueue it and try to find it during Process.
	std::list<SCChatRoomView> m_listSCChatRoomView;

	// Like room view, SCChatRoomEnterUser also arrives separately from EnterUser,
	// When entering a channel, SCChatRoomEnterUser may arrive before other users finish loading.
	// Since this info only needs to be processed once and discarded,
	// When packet first arrives, search nearby; if character exists, process normally and finish; otherwise queue it, process next time, and discard.
	std::list<SCChatRoomEnterUser> m_listSCChatRoomEnterUser;

	TChatRoomView *FindChatRoomView( UINT nSessionID );
	void OnSetChatRoomView( CDnPlayerActor *pActor, TChatRoomView *pView );
	void OnNotifyRoomEnter( CDnPlayerActor *pActor, bool bEnter, int nRoomType = 0 );

	int m_nRequestRoomID;

	// Handled separately because Japan keeps attempting weird channel moves and chat room actions.
	bool m_bRequestWait;

public:
	void RequestCreateChatRoom( const WCHAR *pRoomName, const WCHAR *pPassword, int nRoomType, int nRoomAllow, const WCHAR *pPR1, const WCHAR *pPR2, const WCHAR *pPR3 );
	void RequestConfigChatRoom( const WCHAR *pRoomName, const WCHAR *pPassword, int nRoomType, int nRoomAllow, const WCHAR *pPR1, const WCHAR *pPR2, const WCHAR *pPR3 );

	bool IsEnableChatRoomEnter( UINT nSessionID );
	void RequestEnterChatRoom( UINT nSessionID );
	void RequestKickUser( LPCWSTR pwszName, bool bForever );

	// Info saved when creating the current room. The two below are not in RoomView, so save them separately.
	TChatRoomView &GetChatRoomView() { return m_ChatRoomView; }
	int GetChatRoomAllow() { return m_nChatRoomAllow; }
	LPCWSTR GetPassword() { return m_wszPassword.c_str(); }


	bool IsRequestWait() { return m_bRequestWait; }

	// Handle Entered/Leaved in sight range. Structured this way because the player does not own a room view.
	void OnCharUserEntered( DnActorHandle hActor );
	void OnCharUserLeaved( DnActorHandle hActor );
	void ClearChatRoomInfoList();

	// Exception handling: added because a forced call is needed in some cases.
	void ForceCloseChatRoom();

public:
	bool Initialize();
	void Finalize();

	virtual void OnDisconnectTcp( bool bValidDisconnect );
	virtual void OnDisconnectUdp();

	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );

	// CEtUICallback 
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
};

#define GetChatRoomTask()		CDnChatRoomTask::GetInstance()