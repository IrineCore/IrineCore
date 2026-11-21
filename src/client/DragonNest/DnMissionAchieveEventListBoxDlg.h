#pragma once

#include "DnCustomDlg.h"

class CDnMissionAchieveEventListBoxDlg : public CDnCustomDlg
{
public:
	CDnMissionAchieveEventListBoxDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMissionAchieveEventListBoxDlg();

protected:
	CEtUIListBoxEx *m_pListBoxCompleteTitle;
	std::map<int, int> m_mapMissionEventGroups;

	void LoadMissionEventTable();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void ClearList();
	bool IsEventMissionGroup( int nGroupID ) const;
};
