#pragma once

#include "EtUIDialog.h"
#include "DnChatTabDlg.h"
#include "DnPVPObserverDlg.h"
#include "DnPVPInfoDlg.h"
#include "DnPVPOptionDlg.h"

class CDnPVPLobbyChatTabDlg : public CDnChatTabDlg
{

protected:
	
	CEtUIListBoxEx *m_pObserverListBox;

public:

	struct sObserverSlot{
		int iJobID;
		BYTE    cLevel;
		BYTE    cPVPLevel;
		WCHAR   wszPlayerName[NAMELENMAX];
		UINT    uiSessionID;
		bool    isNeedUpdate;
		UINT	uiUserState;
#if defined( PRE_ADD_REVENGE )
		UINT uiRevengeID;
#endif	// #if defined( PRE_ADD_REVENGE )

		sObserverSlot()
			:iJobID(0)
			,cLevel(0)
			,cPVPLevel(0)
			,isNeedUpdate(true)
			,uiUserState(0)
			,uiSessionID(0)			
#if defined( PRE_ADD_REVENGE )
			, uiRevengeID(0)
#endif	// #if defined( PRE_ADD_REVENGE )
		{			
			SecureZeroMemory(wszPlayerName, sizeof(wszPlayerName));
		}
	};


protected:

	std::vector<sObserverSlot> m_sObserverSlot;
	bool m_SelectList;


	CEtUIRadioButton *m_pButtonCharInfo;
	CEtUIRadioButton *m_pButtonObserver;
	CEtUIRadioButton *m_pButtonConfig;

	CDnPVPObserverDlg *m_pPVPObserverDlg;
	CDnPVPInfoDlg *m_pPVPInfoDlg;
	CDnPVPOptionDlg *m_pPVPOptionDlg;

	bool m_bInitPvpInfo;      // Initialization does not occur when entering the PvP lobby = <no PvP village task> -> must handle initial setup separately depending on task presence.

#ifdef PRE_MOD_PVPOBSERVER
	bool m_bObserverMode;
#endif // PRE_MOD_PVPOBSERVER

	
public:
	CDnPVPLobbyChatTabDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPLobbyChatTabDlg(void);

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Render( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	CEtUIListBoxEx* GetObserverListBox(){return m_pObserverListBox;}
	
	void ShowObserverDlg(bool bTrue); // Show banner when outside and observer list when inside; function to toggle between them
	void InsertObserver(int team , UINT uiUserState , UINT    uiSessionID , int iJobID , BYTE  cLevel , BYTE  cPVPLevel , const WCHAR * szPlayerName );
	void RemoveObserver(int iSessionID ,bool bAll = false);
	void SetSelectedListBox(bool bTrue){m_SelectList = bTrue;}
	
	int GetSessionIDFromList(int Num){return m_sObserverSlot[Num].uiSessionID;};
	std::wstring GetNameFromList(int Num){return m_sObserverSlot[Num].wszPlayerName;}
	int GetObserverNumber(){return (int)m_sObserverSlot.size();}; // Find out how many observers there are.
	bool IsSelectedListBox(){return m_SelectList;};
	sObserverSlot GetObserverSlotFromSessionID(int nSessionID);
	void SetObserverSlotUserState( const int nSessionID, const UINT uiUserState );

	void RefreshMyPVPInfo();
	CDnPVPObserverDlg* GetPVPObserverDlg(){return m_pPVPObserverDlg;}
	CDnPVPOptionDlg* GetPVPOptionDlg(){return m_pPVPOptionDlg;}

#ifdef PRE_MOD_PVPOBSERVER
	bool IsObserverMode(){
		return m_bObserverMode;
	}
#endif // PRE_MOD_PVPOBSERVER

private:
#ifdef PRE_MOD_PVPOBSERVER
	void SetTextObserver();	
#endif // PRE_MOD_PVPOBSERVER


};
