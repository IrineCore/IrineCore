#pragma once

#if defined( PRE_ADD_DWC )

#include "DnSimpleTooltipDlg.h"

class CDnDWCMarkTooltipDlg : public CDnSimpleTooltipDlg
{
public:
	CDnDWCMarkTooltipDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDWCMarkTooltipDlg();

public:
	virtual void Initialize( bool bShow );
	void SetDWCMarkInfo( const WCHAR *wszTeamName, int nRank, int nPoint );
};

class CDnDWCMarkInformer
{
public:
	CDnDWCMarkInformer();
	virtual ~CDnDWCMarkInformer();

protected:
	bool m_bActive;
	int m_nRank;
	BYTE m_cGrade;
	std::wstring m_strTeamName;

public:
	virtual void UpdateMarkInfo( const WCHAR *wszTeamName, int nRank, BYTE cGrade );
	virtual void ResetMarkInfo();

	bool IsActive() const { return m_bActive; }
	int GetRank() const { return m_nRank; }
	BYTE GetGrade() const { return m_cGrade; }
	const std::wstring &GetTeamName() const { return m_strTeamName; }
};

#endif // PRE_ADD_DWC
