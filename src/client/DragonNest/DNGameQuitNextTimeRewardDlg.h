#ifdef PRE_ADD_GAMEQUIT_REWARD

#include "DnCustomDlg.h"

class CDnGameQuitNextTimeRewardDlg : public CDnCustomDlg
{
private:

	// Controls //
	//CEtUITextBox * m_pStatic;
	CEtUIStatic * m_pStatic;
	std::vector< CDnItemSlotButton * > m_vecItemSlot;
	std::vector< CDnItem * > m_vecItem;

	int m_nRewardType; // GameQuitReward::RewardType::eType

	bool m_bLogin; // Window used in both login (character select) and in-game, but handled differently in each.

public:


	CDnGameQuitNextTimeRewardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnGameQuitNextTimeRewardDlg();

	void ReleaseDlg();

	// Override - CEtUIDialog //
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();	
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg = 0 );		
	virtual void Show( bool bShow );

private:

	// Set reward items according to reward target.
	void SetNextTimeRewardItem( bool bCharSelect=false, int nLevel=0, int nClass=-1 );

public:

	// Reward target type (new user, returning user)
	void SetNextTimeRewardType( int type, bool bCharSelect=false, int nLevel=0, int nClass=-1 );



};
#endif // PRE_ADD_GAMEQUIT_REWARD