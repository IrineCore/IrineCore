
#pragma once

#include "Task.h"
#include "MessageListener.h"
#include "DNProtocol.h"
#include "DNPacket.h"
#include "MIInventoryItem.h"

class CDnGestureTask : public CTask, public CTaskListener, public CSingleton<CDnGestureTask>
{
public:
	CDnGestureTask();
	virtual ~CDnGestureTask();

	enum
	{
		NUM_KEYWORD = 9
	};

#if defined(PRE_ADD_GUILD_GESTURE)
	enum eGestureType
	{
		GestureType_Normal = 0,
		GestureType_GuildWarWin = 1,
		GestureType_GuildReward = 2,
	};
#endif // PRE_ADD_GUILD_GESTURE

	struct SGestureInfo
	{
		bool bUsable;	// If free or paid, receive list from server and set to true.
		int nID;
		tstring szName;
		int nIconID;
		string szActionName;
		int nMsgID;
		tstring szKeyword[NUM_KEYWORD];
		tstring szDescription;
		int nUnlockLevel;	// -1 is cash. -2 is guild championship skill
		string szReactionName1;
		string szReactionName2;

#if defined(PRE_ADD_GUILD_GESTURE)
		eGestureType _Type;	//Gesture type
#endif // PRE_ADD_GUILD_GESTURE

	};

	bool Initialize();
	void Finalize();

	void OnDisconnectTcp(bool bValidDisconnect) {}
	void OnDisconnectUdp(bool bValidDisconnect) {}

	void OnDispatchMessage(int nMainCmd, int nSubCmd, char *pData, int nSize);

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	DWORD GetVisibleGestureCount();
	SGestureInfo *GetVisibleGestureInfoFromIndex( DWORD nIndex );
	SGestureInfo *GetGestureInfoFromID( int nID );

	// Use localplayer
	void UseGesture( int nGestureID, bool bUseByChat = false );

	// Used as a chat command
	void UseGestureByChat( LPCWSTR wszMessage );

	// Other player response
	void OnRecvCmdAction( LPCWSTR wszActorName, int nGestureID );

	// Non-cash gestures are now handled locally without exchanging lists between server and client.
	void RefreshGestureList( int nCurrentLevel, bool bOnLevelUp = false );

protected:

	// No spamming
	float m_fPaperingRemainTime;
	std::list<float> m_listRecentChatTime;
	bool CheckPapering();

	void OnRecvGestureMessage(int nSubCmd, char *pData, int nSize);
	void OnRecvGuildMessage(int nSubCmd, char *pData, int nSize);

	virtual void OnRecvGestureList( SCGestureList *pPacket );
	virtual void OnRecvGestureAdd( SCGestureAdd *pPacket );
	virtual void OnRecvUseGesture( SCUseGesture *pPacket );

	virtual void OnRecvGuildWarPreWin( SCGuildWarPreWin *pPacket );
	virtual void OnRecvGuildWarWinSkill( SCGuildWarWinSkill *pPacket );
	virtual void OnRecvGuildWarUserWinSkill( SCGuildWarUserWinSkill *pPacket );
	int m_nRequestGuildWarSkillID;

	SGestureInfo *GetGestureInfoFromActionName( const char *szActionName );
	SGestureInfo *GetGestureInfoFromGestureName( LPCWSTR wszName );
	SGestureInfo *GetGestureInfoFromKeyword( LPCWSTR wszKeyword );

	std::vector<SGestureInfo *> m_pVecGestureInfo;

	// Looking at ItemTask, quickslot Add requests cannot be made while receiving the list.
	// ItemTask works without issues, so follow the same approach.
	bool m_bRequestQuickSlot;

	void OnRecvGuildRewardGestureList( SCGestureList *pPacket );
};

#define GetGestureTask()	CDnGestureTask::GetInstance()