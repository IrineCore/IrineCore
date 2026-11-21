#include "StdAfx.h"
#include "DnSkillCountDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnSkillCount
//-----------------------------------------------------------------------------
CDnSkillCount::CDnSkillCount( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_nCount( 0 )
	, m_fRemainTime( 0.0f )
{
}

CDnSkillCount::~CDnSkillCount()
{
}

void CDnSkillCount::InitialUpdate()
{
}

void CDnSkillCount::SetSkillCount( int nCount, float fDuration )
{
	m_nCount = nCount;
	m_fRemainTime = fDuration;
	Show( nCount > 0 );
}

void CDnSkillCount::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );
	if( IsShow() && m_fRemainTime > 0.0f )
	{
		m_fRemainTime -= fElapsedTime;
		if( m_fRemainTime <= 0.0f )
		{
			m_nCount = 0;
			Show( false );
		}
	}
}

//-----------------------------------------------------------------------------
// CDnSkillCountDlg
//-----------------------------------------------------------------------------
CDnSkillCountDlg::CDnSkillCountDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_nComboCount( 0 )
	, m_fDelayTime( 0.0f )
{
}

CDnSkillCountDlg::~CDnSkillCountDlg()
{
}

void CDnSkillCountDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ComboMaskDlg.ui" ).c_str(), bShow );
}

void CDnSkillCountDlg::InitialUpdate()
{
}

void CDnSkillCountDlg::SetComboCount( int nCount, float fDelay )
{
	m_nComboCount = nCount;
	m_fDelayTime = fDelay;
	Show( nCount > 0 );
}

void CDnSkillCountDlg::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	if( IsShow() && m_fDelayTime > 0.0f )
	{
		m_fDelayTime -= fElapsedTime;
		if( m_fDelayTime <= 0.0f )
		{
			m_nComboCount = 0;
			Show( false );
		}
	}
}

//-----------------------------------------------------------------------------
// CDnSkillCountMng
//-----------------------------------------------------------------------------
CDnSkillCountMng::CDnSkillCountMng()
	: m_pSkillCount( NULL )
	, m_pSkillCountDlg( NULL )
{
}

CDnSkillCountMng::~CDnSkillCountMng()
{
	Finalize();
}

void CDnSkillCountMng::Initialize()
{
	if( !m_pSkillCount )
		m_pSkillCount = new CDnSkillCount( UI_TYPE_BOTTOM );

	if( !m_pSkillCountDlg )
	{
		m_pSkillCountDlg = new CDnSkillCountDlg( UI_TYPE_BOTTOM );
		m_pSkillCountDlg->Initialize( false );
	}
}

void CDnSkillCountMng::Finalize()
{
	SAFE_DELETE( m_pSkillCount );
	SAFE_DELETE( m_pSkillCountDlg );
}

void CDnSkillCountMng::SetSkillCount( int nCount, float fDelay )
{
	if( m_pSkillCount )
		m_pSkillCount->SetSkillCount( nCount, fDelay );
	if( m_pSkillCountDlg )
		m_pSkillCountDlg->SetComboCount( nCount, fDelay );
}

void CDnSkillCountMng::Reset()
{
	if( m_pSkillCount )
		m_pSkillCount->Show( false );
	if( m_pSkillCountDlg )
		m_pSkillCountDlg->Show( false );
}
