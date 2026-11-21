#pragma once
#include "EtUIDialog.h"
#include "EtUIDialogGroup.h"
#include "DNPacket.h"

class CDnHelpDlg;
class CDnGameOptDlg;
class CDnSystemOptDlg;
class CDnChannelDlg;
class CDnGRBDlg;
class CDnEventTodayDlg;
class CDnGameKeySetDlg;

#ifdef PRE_ADD_VIP
class CDnVIPDlg;
#endif

class CDnSystemDlg : public CEtUIDialog, public CEtUICallback
{
public:
	enum
	{
		HELP_DIALOG,
		CHANNEL_DIALOG,
		GAME_OPTION_DIALOG,
		SYSTEM_OPTION_DIALOG,
		GAME_GRAGE_DIALOG,
#ifdef PRE_ADD_VIP
		VIP_DIALOG,
#endif // PRE_ADD_VIP
		GAME_KEY_DIALOG,
		MESSAGEBOX_STAGE,
		MESSAGEBOX_CHAR,
		MESSAGEBOX_EXIT,
	};

public:
	CDnSystemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSystemDlg(void);

protected:
	CEtUIButton *m_pButtonHelp;
	CEtUIButton *m_pButtonChannel;
	CEtUIButton *m_pButtonHelpKeyboard;
	CEtUIButton *m_pButtonGame;
	CEtUIButton *m_pButtonSystem;
	CEtUIButton *m_pButtonStage;
	CEtUIButton *m_pButtonChar;
	CEtUIButton *m_pButtonExit;
#ifdef PRE_ADD_VIP
	CEtUIButton *m_pButtonVIP;
#endif


	CDnHelpDlg *m_pHelpDlg;
	CDnGameOptDlg *m_pGameOptDlg;
	CDnSystemOptDlg *m_pSystemOptDlg;
	CDnChannelDlg *m_pChannelListDlg;
	CDnGRBDlg *m_pGameGradeDlg;
	CDnEventTodayDlg * m_pEventTodayDlg;
#ifdef PRE_ADD_VIP
	CDnVIPDlg* m_pVIPDlg;
#endif
	CDnGameKeySetDlg * m_pGameKeySetDlg;

	CEtUIDialogGroup m_SystemDlgGroup;

	bool m_bOpenChannel;

#ifdef PRE_ADD_GAMEQUIT_REWARD
	int m_nGameQuitCharSelectStrIdx;
#endif // PRE_ADD_GAMEQUIT_REWARD

protected:
	tstring MakeDurabilityWarningStr( int nUIString );

#ifdef PRE_ADD_PVP_COMBOEXERCISE
	bool IsPvPComboExerciseMode(); // Change exit message for combo practice mode.	
#endif // PRE_ADD_PVP_COMBOEXERCISE

public:

	void AddChannelList( sChannelInfo *pChannelInfo );
	void SetDefaultList();
	void OpenChannelDlg();
	void ToggleHelpDlg();
	void OnCloseCompletely();
	bool IsOpenChannelDlg() const;
#ifdef PRE_ADD_GAMEQUIT_REWARD
	void OnExitButton( bool bForce=true );	
#else
	void OnExitButton();
#endif // PRE_ADD_GAMEQUIT_REWARD
	void OnCharButton();
	void ShowHelpDlg(int nIndex = 0);
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
	void ShowChoiceHelpDlg(eShortCutHelpDlgType eDlgType);
#endif

#ifdef PRE_ADD_GAMEQUIT_REWARD
	int IsGameQuitCharSelectStrIdx(){
		return m_nGameQuitCharSelectStrIdx;
	}
#endif // PRE_ADD_GAMEQUIT_REWARD

#ifdef PRE_FIX_APPELATION_TOOLTIP
	bool IsShowDlg(DWORD dwDlgType) const;
#endif

public:
	virtual void InitialUpdate();
	virtual void Initialize(bool bShow);
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void Process( float fElapsedTime );
};
