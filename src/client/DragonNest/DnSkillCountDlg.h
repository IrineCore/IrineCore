#pragma once

#include "DnCustomDlg.h"

class CDnSkillCount : public CEtUIDialog
{
public:
	CDnSkillCount( UI_DIALOG_TYPE dialogType = UI_TYPE_BOTTOM, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSkillCount();

protected:
	int m_nCount;
	float m_fRemainTime;

public:
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );

	void SetSkillCount( int nCount, float fDuration );
	int GetSkillCount() const { return m_nCount; }
};

class CDnSkillCountDlg : public CDnCustomDlg
{
public:
	CDnSkillCountDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_BOTTOM, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSkillCountDlg();

protected:
	int m_nComboCount;
	float m_fDelayTime;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );

	void SetComboCount( int nCount, float fDelay );
};

class CDnSkillCountMng
{
public:
	CDnSkillCountMng();
	virtual ~CDnSkillCountMng();

protected:
	CDnSkillCount *m_pSkillCount;
	CDnSkillCountDlg *m_pSkillCountDlg;

public:
	virtual void Initialize();
	virtual void Finalize();
	void SetSkillCount( int nCount, float fDelay = 2.0f );
	void Reset();
	CDnSkillCountDlg *GetSkillCountDlg() const { return m_pSkillCountDlg; }
};
