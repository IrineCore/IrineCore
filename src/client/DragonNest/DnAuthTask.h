#pragma once

#include "Task.h"
#include "MessageListener.h"

class CDnAuthTask : public CTask, public CTaskListener, public CSingleton<CDnAuthTask> , public CEtUICallback
{
public:
	CDnAuthTask();
	virtual ~CDnAuthTask();
	
	enum AuthMsg
	{
		AUTHUNLOCK,
		AUTH_MAINMSG = 6438,	// String 6438 showing the long message: Cannot use while locked...
	};

#ifdef PRE_ADD_VIP
	enum emAuthCheckType
	{
		Login,
		LoginCharDel, 
		Game, 
		Validate_VIP,
	};
#endif
	
	virtual void OnDisconnectTcp( bool bValidDisconnect ) {}
	virtual void OnDisconnectUdp( bool bValidDisconnect ) {}
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	bool Initialize() {return true;}
	void RecvOptionMessage( int nSubCmd, char *pData, int nSize );


	void OnRecvSecondAuthInfo(SCGameOptionNotifySecondAuthInfo *pPacket); 
	void OnRecvSecondAuthPassword(SCGameOptionUpdateSecondAuthPassword *pPacket);
	void OnRecvSecondAuthLock(SCGameOptionUpdateSecondAuthLock *pPacket);
	void OnRecvSecondAuthTradeRestraint();

	void OnRecvSecondAuthInit(SCGameOptionInitSecondAuth *pPacket);
	void OnRecvSecondAuthValidate(SCGameOptionValidateSecondAuth* pData);
	bool GetSecondAuthLock(){return m_bIsSetSecondAuthLock;}
	bool GetSecondAuthPW(){return m_bIsSetSecondAuthPW;}
	bool GetSecondAuthNotifyShow(){return m_bIsSecondAuthNotifyShowInfo;}
	void SetSecondAuthNotifyShow(bool bFlag, bool bShow2ndPassCreateButton);

	void SetSecondAuthLock(bool bSetAuthLock) {m_bIsSetSecondAuthLock = bSetAuthLock;}
	void SetSecondAuthPw(bool bSetAuthPw ) {m_bIsSetSecondAuthPW = bSetAuthPw;}
	void RequestCreateAuthPW( int nSeed, int nValue[] );
	void RequestChangeAuthPW( int nOldSeed, int nOldValue[], int nNewSeed, int nNewValue[] );
	void RequestAuthLock(bool bLock , int nSeed, int nValue[]);
	void RequestAuthUnLock(bool bLock , int nSeed, int nValue[]);
	void RequestAuthInit(int nSeed, int nValue[]);
	void RequestAuthNotifyDate();
	void OpenMsgBoxAuthUnLock();
	void RequestValidateAuth(int nSeed, int nValue[], int authCheckType);
	bool CheckAuthLock();
	__time64_t GetSecondAuthResetDate() { return m_tSecondAuthResetDate; }

protected:
	bool m_bIsSetSecondAuthLock; 
	bool m_bIsSetSecondAuthPW; 
	int m_FailCount;
	bool m_bIsSecondAuthNotifyShowInfo;
	__time64_t m_tSecondAuthResetDate;

#if defined(PRE_ADD_23829)
	bool m_bIgnore2ndPassNotify;		//Flag to check hide for 3 days
	bool m_bIsShow2ndPassCreateButton;	//Flag for Show/Hide of 2nd password create button
public:
	bool GetSecondPassCreateButton() { return m_bIsShow2ndPassCreateButton; }
	void SetIgnore2ndPassNotify(bool bCheck) { m_bIgnore2ndPassNotify = bCheck; }
	bool GetIgnore2ndPassNotify() { return m_bIgnore2ndPassNotify; }
#endif // PRE_ADD_23829
};
#define GetDnAuthTask()	CDnAuthTask::GetInstance()
