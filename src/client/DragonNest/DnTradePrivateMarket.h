#pragma once
#include "DNPacket.h"

class CDnItem;
class CDnPrivateMarketDlg;

class CDnTradePrivateMarket	: public CEtUICallback
{
	struct STradeUserInfo
	{
		bool m_bSelfToTarget;	// true if self initiated the trade, false if self received the trade request.
		DWORD m_dwSessionID;
		std::wstring m_strName;
		float m_fAcceptTime;

		STradeUserInfo()
			: m_dwSessionID(0)
			, m_fAcceptTime(10.0f)
		{
			m_bSelfToTarget = false;
		}
	};

public:
	CDnTradePrivateMarket(void);
	virtual ~CDnTradePrivateMarket(void);

protected:
	DWORD m_dwCurTradeUserID;		// ID of the trading partner user
	DWORD m_dwReadyTradeUserID;		// Target ID of the currently open window (whether accept or request)
	std::wstring m_wszTradeUserName;

	// Other party's trade item list
	std::vector<CDnItem*> m_vecTradePlayerItem;

	CDnPrivateMarketDlg *m_pPrivateMarketDialog;

	typedef std::list<STradeUserInfo>		LIST_TRADEUSERINFO;
	typedef LIST_TRADEUSERINFO::iterator	LIST_TRADEUSERINFO_ITER;

	LIST_TRADEUSERINFO m_listTradeUserInfo;

	int m_nChatMode;				// Chat mode when the trade window opens. Saved to restore later.

public:
	void Process( float fElapsedTime );

	float GetShortestAcceptTime();
	void OpenAcceptRequestDialog();

protected:
	void AddTradePlayerItem( int nIndex, TItemInfo &Item );
	void RemoveTradePlayerItem( int nIndex );
	void ClearTradePlayerItem();

	void AddTradeUserInfo( bool bSelfToTarget, DWORD nSessionID );
	bool RemoveTradeUserInfo( bool bSelfToTarget, DWORD nSessionID );

	bool GetTradeUserName( DWORD nSessionID, std::wstring &wszName );

public:
	// Request cancellation can also be called externally.
#ifdef PRE_FIX_CANCELTRADE
	void ClearTradeUserInfoList(bool bClearOnly = false);
#else
	void ClearTradeUserInfoList();
#endif

	// Target ID of the user with a currently open request/accept window
	void SetTradePlayerSessionID( DWORD dwSessionID )	{ m_dwReadyTradeUserID = dwSessionID; }
	DWORD GetTradePlayerSessionID()						{ return m_dwReadyTradeUserID; }

#ifdef PRE_FIX_49403
	// ID of the trading partner user.
	DWORD GetCurTradeUserID(){
		return m_dwCurTradeUserID;
	}

	void SendExchangeRequestCancel( DWORD nSessionID ); // Cancel trade request.

#endif

	void SetPrivateMarketDialog( CDnPrivateMarketDlg *pDialog ) { m_pPrivateMarketDialog = pDialog; }
	CDnPrivateMarketDlg *GetPrivateMarketDlg() { return m_pPrivateMarketDialog; }

	// Whether user already requested. - Previously only users who sent/received a trade request could not re-request trade,
	// Design changed, so only the two functions below are used now.
	bool IsRequestUser( DWORD nSessionID );

	// Am I requesting someone,
	bool IsRequesting();

	// Whether someone is requesting to me.
	bool IsAccepting();


	// Mark items in the trade history with a new indicator.
	// Currently unused because items arriving via RefreshInven are processed in batch.
	//void SetNewGainTradeList();

public:
	void RequestPrivateMarket( UINT nSessionID );
	void RequestPrivateMarketAccept( bool bAccept, UINT nSessionID );
	void RequestPrivateMarketAddItem( int nSlotIndex, int nInvenIndex, int nCount, INT64 biSerial );
	void RequestPrivateMarketDeleteItem( int nIndex );
	void RequestPrivateMarketCoin( INT64 nCoin );
	void RequestPrivateMarketConfirm( char cType );
	void RequestPrivateMarketCancel();
	void RequestPrivateMarketComplete();

public:
	void OnRecvPrivateMarket( SCExchangeRequest *pPacket );
	void OnRecvPrivateMarketStart( SCExchangeStart *pPacket );
	void OnRecvPrivateMarketAddItem( SCExchangeAddItem *pPacket );
	void OnRecvPrivateMarketDeleteItem( SCExchangeDeleteItem *pPacket );
	void OnRecvPrivateMarketCoin( SCExchangeAddCoin *pPacket );
	void OnRecvPrivateMarketConfirm( SCExchangeConfirm *pPacket );
	void OnRecvPrivateMarketComplete( SCExchangeComplete *pPacket );
	void OnRecvPrivateMarketCancel();
	void OnRecvPrivateMarketReject( SCExchangeReject *pPacket );

public:
	// CEtUICallback
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

};