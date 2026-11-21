#pragma once
#include "EtUIDialog.h"

#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
class CDnDungeonLimitDlg : public CEtUIDialog
{
public:
	enum eIconType
	{
		TYPE_DAMAGE = 0,
		TYPE_HEAL = 1,
		TYPE_NONE1 = 2,
		TYPE_NONE2 = 3,
		TYPE_MAX = 4,
	};

protected:
	CEtUIStatic *m_pStaticIcon[eIconType::TYPE_MAX];

public:
	CDnDungeonLimitDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnDungeonLimitDlg();

	virtual void Initialize(bool bShow);
	virtual void InitialUpdate();
	void SetDungeonLimitInfo(int nMapIndex);

};
#endif
