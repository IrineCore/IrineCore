#ifdef PRE_ADD_GAMEQUIT_REWARD

#include "DnCustomDlg.h"

class CDnGameQuitRewardDlg : public CDnCustomDlg
{
private:

	// Controls //
	CEtUIStatic * m_pStatic;
	std::vector< CDnItemSlotButton * > m_vecItemSlot;

	std::vector< CDnItem * > m_vecItem[2]; // 0 (New user), 1 (Returning user)

	int m_nRewardType; // GameQuitReward::RewardType::eType

	bool m_bRecvReward; // true if a new user received the instant-claim gift.

public:


	CDnGameQuitRewardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnGameQuitRewardDlg();

	void ReleaseDlg();

	// Override - CEtUIDialog //
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();	
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg = 0 );		
	virtual void Show( bool bShow );

private:

	// Set reward items according to reward target.
	void SetRewardItem();

	// Request to receive gift.
	void GiftReceiving();

	// Display reward on next login.
	void ShowNextTimeReward();

public:

	// Reward target type (new user, returning user)
	void SetRewardType( int type );

	void RecvReward();
	bool GetRecvReward(){
		return m_bRecvReward;
	}
	bool ShowNewbieNextTimeReward();

};
#endif // PRE_ADD_GAMEQUIT_REWARD