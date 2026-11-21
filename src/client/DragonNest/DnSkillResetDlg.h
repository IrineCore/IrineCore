#pragma once
#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"

class CDnItem;
class CDnSkillResetDlg : public CEtUIDialog
{
public:
	CDnSkillResetDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallBack = NULL, bool bAutoCursor = false );
	virtual ~CDnSkillResetDlg(void);

protected:
	CEtUIStatic* m_pResetExplain;
	CEtUIStatic* m_pRewardSP;
	CEtUIStatic* m_pSkillPage;

	CEtUIButton* m_pButtonOK;
	CEtUIButton* m_pButtonCancel;

	// Smart move
	CDnSmartMoveCursor m_SmartMove;

	int m_iStartJobDegree;
	int m_iEndJobDegree;
	int m_iRewardSP;
	INT64 m_biItemSerial;		// Serial index of the skill reset cash item currently used
#ifdef PRE_MOD_SKILLRESETITEM
	int m_iItemInvenTypeCache;
	int m_iItemInvenIndexCache;
#endif

public:
	virtual void InitialUpdate();
	virtual void Initialize(bool bShow);
	virtual void Show( bool bShow );

	virtual void ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg);

#ifdef PRE_MOD_SKILLRESETITEM
	void UpdateContent(const INT64 biItemSerial, const int iStartJobDegree, const int iEndJobDegree, const int iRewardSP, const int iInvenType, const int iInvenIndex);
#else
	void UpdateContent( INT64 biItemSerial, int iStartJobDegree, int iEndJobDegree, int iRewardSP );
#endif
};
