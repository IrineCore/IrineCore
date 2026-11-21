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
	, m_pCustomMask( NULL )
	, m_nComboCount( 0 )
	, m_fDelayTime( 0.0f )
	, m_bSlideIn( false )
	, m_bShake( false )
	, m_bSlideOut( false )
	, m_nShakeCount( 0 )
	, m_fBasePosX( 0.0f )
	, m_fSlideProgress( 0.0f )
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
	m_pCustomMask = GetControl<CEtUIControl>( "ID_CUSTOM_MASK" );
	SUICoord dlgCoord;
	GetDlgCoord( dlgCoord );
	m_fBasePosX = dlgCoord.fX;
}

void CDnSkillCountDlg::SetComboCount( int nCount, float fDelay )
{
	m_nComboCount = nCount;
	m_fDelayTime = fDelay;
	if( nCount > 0 )
	{
		m_bSlideIn = true;
		m_bShake = false;
		m_bSlideOut = false;
		m_nShakeCount = 6;
		m_fSlideProgress = 0.0f;
		if( m_pCustomMask )
			m_pCustomMask->Show( true );
		Show( true );
	}
	else
	{
		m_bSlideIn = false;
		m_bShake = false;
		m_bSlideOut = false;
		Show( false );
	}
}

void CDnSkillCountDlg::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	if( !IsShow() )
		return;

	SUICoord dlgCoord;
	GetDlgCoord( dlgCoord );

	if( m_bSlideIn )
	{
		m_fSlideProgress += fElapsedTime * 8.0f;
		if( m_fSlideProgress >= 1.0f )
		{
			m_fSlideProgress = 1.0f;
			m_bSlideIn = false;
			m_bShake = true;
		}
		dlgCoord.fX = m_fBasePosX + ( 1.0f - m_fSlideProgress ) * 0.05f;
		SetDlgCoord( dlgCoord );
		return;
	}

	if( m_bShake )
	{
		float fOffset = ( ( m_nShakeCount & 1 ) == 0 ) ? -0.004f : 0.004f;
		dlgCoord.fX = m_fBasePosX + fOffset;
		SetDlgCoord( dlgCoord );
		--m_nShakeCount;
		if( m_nShakeCount <= 0 )
		{
			dlgCoord.fX = m_fBasePosX;
			SetDlgCoord( dlgCoord );
			m_bShake = false;
			m_bSlideOut = true;
		}
		return;
	}

	if( m_fDelayTime > 0.0f )
		m_fDelayTime -= fElapsedTime;

	if( m_bSlideOut && m_fDelayTime <= 0.0f )
	{
		m_fSlideProgress -= fElapsedTime * 6.0f;
		if( m_fSlideProgress <= 0.0f )
		{
			m_bSlideOut = false;
			m_nComboCount = 0;
			dlgCoord.fX = m_fBasePosX;
			SetDlgCoord( dlgCoord );
			Show( false );
		}
		else
		{
			dlgCoord.fX = m_fBasePosX + ( 1.0f - m_fSlideProgress ) * 0.05f;
			SetDlgCoord( dlgCoord );
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
