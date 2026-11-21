#pragma once

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnCustomDlg.h"
#include "DnTalisman.h"

class CDnPlayerActor;

class CDnCharTalismanInfoDlg : public CDnCustomDlg
{
public:
	enum {
		FLOAT_TYPE,
		DOUBLE_FLOAT_TYPE,
		INT_TYPE,
		DOUBLE_INT_TYPE
	};

	CDnCharTalismanInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharTalismanInfoDlg();

	virtual void SetText();
	virtual void SetText( CDnPlayerActor *pActor );
	void AddStateText( int nStringID, int eType, float fFirstData = 0.0f, float fSecondData = 0.0f );
	void SetItemStateText( CDnState *pState );

protected:
	CEtUITextBox *m_pTextBox;

	std::vector< std::pair<int, int> >                      m_vIntTypeList;
	std::vector< std::pair<int, std::pair<int, int> > >     m_vDoubleIntTypeList;
	std::vector< std::pair<int, float> >                    m_vFloatTypeList;
	std::vector< std::pair<int, std::pair<float, float> > > m_vDoubleFloatTypeList;

	void SetNormalInfo();
	void SetDropInfo();
	void SetSkillInfo();

	void SetNormalInfo( CDnPlayerActor *pActor );
	void SetDropInfo( CDnPlayerActor *pActor );
	void SetSkillInfo( CDnPlayerActor *pActor );

	void FlushAccumulatedStats();
	void AddTalismanExplain( CDnTalisman *pTalisman, float fEfficiency );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

#endif // PRE_ADD_TALISMAN_SYSTEM
