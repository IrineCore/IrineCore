#pragma once


#include "../../DnLauncherDlg.h"


class CDnLauncherDlgUSA : public CDnLauncherDlg
{
public:
	CDnLauncherDlgUSA( CWnd* pParent = NULL );
	virtual ~CDnLauncherDlgUSA() {}

public:
	afx_msg void OnDestroy();
	afx_msg void OnClickButtonCancel();
	afx_msg void OnClickButtonGameStart();
	afx_msg void OnClickButtonClose();

	DECLARE_MESSAGE_MAP()

protected:
	virtual void MakeFont();			// Create font
	virtual BOOL InitWebpage();			// Initialize guide web page
};
