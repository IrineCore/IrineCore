#include "StdAfx.h"
#include "DnCharStatusStatic.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnCharStatusStatic::CDnCharStatusStatic( CEtUIDialog *pParent )
	: CEtUIStatic( pParent )
{
	m_bInitialized = false;
	ClearValue();
	ClearBuffValue();
}

CDnCharStatusStatic::~CDnCharStatusStatic(void)
{
}

void CDnCharStatusStatic::UpdateValueCoord()
{
	SUIElement *pElement = GetElement(0);
	if( pElement )
	{
		SUICoord textCoord, textCoord2;

		m_pParent->CalcTextRect( m_strValueString1.c_str() , pElement, textCoord );
		m_ValueCoord1.SetCoord( m_Property.UICoord.fX, m_Property.UICoord.fY, textCoord.fWidth, m_Property.UICoord.fHeight );

		if( m_bShowTooltip == true )
		{
			m_pParent->CalcTextRect( m_strValueString2.c_str() , pElement, textCoord2 );
			m_ValueCoord2.SetCoord( m_Property.UICoord.fX + textCoord.fWidth, m_Property.UICoord.fY, textCoord2.fWidth, m_Property.UICoord.fHeight );
		}
	}
}

void CDnCharStatusStatic::SetValue_Float( emVALUE_TYPE valueType, bool bShowTooltip, float fValue1, float fValue2, float fValue3 )
{
	if( m_fValue1 == fValue1 && 
		m_fValue2 == fValue2 && 
		m_fValue3 == fValue3 && 
		m_bShowTooltip == bShowTooltip && 
		m_bInitialized == true )
	{
		return;
	}

	ClearValue();

	m_fValue1 = fValue1;
	m_fValue2 = fValue2;
	m_fValue3 = fValue3;

	switch( valueType )
	{
	case TYPE_03: // Elemental attack, elemental defense
		{
			m_strValueString1 += FormatW( L"%.2f", m_fValue1 );
			m_strValueString2 += FormatW( L"%.2f" , m_fValue2 );

			if( m_fBuffValue1 != 0.f || m_fBuffValue2 !=0.f )
			{
				float fBaseValue = m_fValue3 - m_fBuffValue1;

				m_strValueString2 += FormatW( L" %s(%c%.2f)" , ( fBaseValue >= 0.f ) ? L"#g" : L"#r" , ( fBaseValue >= 0.f ) ? '+' : '-', fabs( fBaseValue ) );
				m_strValueString2 += FormatW( L" %s(%c%.2f)" , ( m_fBuffValue1 >= 0.f ) ? L"#s" : L"#r" , ( m_fBuffValue1 >= 0.f ) ? '+' : '-', fabs( m_fBuffValue1 ) );
			}
			else
			{
				m_strValueString2 += FormatW( L" %s(%c%.2f)" , ( m_fValue3 >= 0.f ) ? L"#g" : L"#r" , ( m_fValue3 >= 0.f ) ? '+' : '-', fabs(m_fValue3) );
			}
		}
		break;

	default:
		{
		}
		break;
	}

	m_bShowTooltip = bShowTooltip;

	std::wstring strToolTipString;

	strToolTipString += m_strValueString2;
	strToolTipString += L"#d\n";
	strToolTipString += m_strDetailString;
	m_strFinalString = m_strValueString2;

	if( m_bShowTooltip ) 
		SetTooltipText( strToolTipString.c_str() );

	UpdateValueCoord();

	m_bInitialized = true;
}

void CDnCharStatusStatic::SetValue( emVALUE_TYPE valueType, bool bShowTooltip, int nValue1, int nValue2, int nValue3, int nValue4 , int nValue5 ,int nValue6)
{
	if( m_nValue1 == nValue1 &&
		m_nValue2 == nValue2 &&
		m_nValue3 == nValue3 &&
		m_nValue4 == nValue4 &&
		m_nValue5 == nValue5 &&
		m_nValue6 == nValue6 &&
		m_bShowTooltip == bShowTooltip && 
		m_bInitialized == true )
	{
		return;
	}
	
	ClearValue();

	m_nValue1 = nValue1;
	m_nValue2 = nValue2;
	m_nValue3 = nValue3;
	m_nValue4 = nValue4;
	m_nValue5 = nValue5;
	m_nValue6 = nValue6;

	switch( valueType )
	{
	case TYPE_01: // STR, AGI, INT, VIT
		{
			m_strValueString1 += FormatW( L"%d", m_nValue1 );
			m_strValueString2 += FormatW( L"%d" , m_nValue2 );

			if( m_fBuffValue1 != 0.f || m_fBuffValue2 !=0.f )
			{
				int nBaseValue = m_nValue3 - (int)m_fBuffValue1;

				m_strValueString2 += FormatW( L" %s(%c%d)" , ( nBaseValue >= 0 ) ? L"#g" : L"#r" ,( nBaseValue >= 0 ) ? '+' : '-', abs(nBaseValue) );
				m_strValueString2 += FormatW( L" %s(%c%d)" , ( (int)m_fBuffValue1 >= 0 ) ? L"#s" : L"#r" ,( (int)m_fBuffValue1 >= 0 ) ? '+' : '-', abs((int)m_fBuffValue1) );
			}
			else
			{
				m_strValueString2 += FormatW( L" %s(%c%d)" , ( m_nValue3 >= 0 ) ? L"#g" : L"#r" ,( m_nValue3 >= 0 ) ? '+' : '-', abs(m_nValue3) );
			}
		}
		break;

	case TYPE_02: // HP, MP
		{
			m_strValueString1 += FormatW( L"%d/%d", m_nValue1, m_nValue2 );

#ifdef PRE_ADD_CHAR_STATUS_SECOND_RENEW
			m_strValueString2 += FormatW( L"%d" , m_nValue3 );

			if( m_fBuffValue1 != 0.f || m_fBuffValue2 !=0.f )
			{
				int nBaseValue = m_nValue4 - (int)m_fBuffValue1;
				m_strValueString2 += FormatW( L" %s(%c%d)" , ( nBaseValue >= 0 ) ? L"#g" : L"#r" ,( nBaseValue >= 0 ) ? '+' : '-', abs(nBaseValue) );
				m_strValueString2 += FormatW( L" %s(%c%d)" , ( (int)m_fBuffValue1 >= 0 ) ? L"#s" : L"#r" ,( (int)m_fBuffValue1 >= 0 ) ? '+' : '-', abs((int)m_fBuffValue1) );
			}
			else
			{
				m_strValueString2 += FormatW( L" %s(%c%d)" , ( m_nValue4 >= 0 ) ? L"#g" : L"#r" ,( m_nValue4 >= 0 ) ? '+' : '-', abs(m_nValue4) );
			}
#endif

		}
		break;

	case TYPE_04: // PhysAtk, MagAtk
		{
			m_strValueString1 += FormatW( L"%d%s%d", m_nValue1, TILDE, m_nValue2 );
			m_strValueString2 += FormatW( L"(%c%d%s%d)" , '+' , m_nValue3, TILDE, m_nValue4 );
			

			if( m_fBuffValue1 != 0.f || m_fBuffValue2 !=0.f )
			{
				int nBaseValue1 = m_nValue5 - (int)m_fBuffValue1;
				int nBaseValue2 = m_nValue6 - (int)m_fBuffValue2;

				// (+a~+b)
				m_strValueString2 += FormatW( L" #d(%s%c%d#d%s%s%c%d#d)" , 
					( nBaseValue1 >= 0.f ) ? L"#g" : L"#r" , ( nBaseValue1 >= 0 ) ? '+' : '-', abs(nBaseValue1), TILDE, 
					( nBaseValue2 >= 0.f ) ? L"#g" : L"#r" , ( nBaseValue2 >= 0 ) ? '+' : '-' , abs(nBaseValue2) );

				// (+a~+b)
				m_strValueString2 += FormatW( L" #d(%s%c%d#d%s%s%c%d#d)" , 
					( (int)m_fBuffValue1 >= 0.f ) ? L"#s" : L"#r" , ( (int)m_fBuffValue1 >= 0 ) ? '+' : '-', abs((int)m_fBuffValue1) , TILDE, 
					( (int)m_fBuffValue2 >= 0.f ) ? L"#s" : L"#r" , ( (int)m_fBuffValue2 >= 0 ) ? '+' : '-', abs((int)m_fBuffValue2) );
			}
			else
			{
				// (+a~+b)
				m_strValueString2 += FormatW( L" #d(%s%c%d#d%s%s%c%d#d)" , 
					( m_nValue5 >= 0.f ) ? L"#g" : L"#r" , ( m_nValue5 >= 0 ) ? '+' : '-', abs(m_nValue5), TILDE, 
					( m_nValue6 >= 0.f ) ? L"#g" : L"#r" , ( m_nValue6 >= 0 ) ? '+' : '-' , abs(m_nValue6) );
			}

		}
		break;
	case TPYE_05: // PhysDef, MagDef, CritResist, StunResist, MoveSpeed
		{
			m_strValueString1 += FormatW(  L"%d", m_nValue1 );
			m_strValueString2 += FormatW( L"%d" , m_nValue2 );

			if( m_fBuffValue1 != 0.f || m_fBuffValue2 !=0.f )
			{
				int nBaseValue1 = m_nValue3 - (int)m_fBuffValue1;
				int nBaseValue2 = m_nValue4 - (int)m_fBuffValue2;
			
#if defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW)
				m_strValueString2 += FormatW( L" %s(%c%d)#d" , ( nBaseValue1 >= 0.f ) ? L"#g" : L"#r" , ( nBaseValue1 >= 0 ) ? '+' : '-', abs(nBaseValue1) );
				m_strValueString2 += FormatW( L" %s(%c%d)#d" , ( (int)m_fBuffValue1 >= 0.f ) ? L"#s" : L"#r" , ( (int)m_fBuffValue1 >= 0 ) ? '+' : '-', abs((int)m_fBuffValue1) );
#else
				m_strValueString2 += FormatW( L" %s(%c%d)#d #y[%d%%]" , ( nBaseValue1 >= 0.f ) ? L"#g" : L"#r" , ( nBaseValue1 >= 0 ) ? '+' : '-', abs(nBaseValue1) , abs(nBaseValue2) );
				m_strValueString2 += FormatW( L" %s(%c%d)#d #y[%d%%]" , ( (int)m_fBuffValue1 >= 0.f ) ? L"#s" : L"#r" , ( (int)m_fBuffValue1 >= 0 ) ? '+' : '-', abs((int)m_fBuffValue1) , abs((int)m_fBuffValue2) );
#endif

			}
			else
			{
#if defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW)
				m_strValueString2 += FormatW( L" %s(%c%d)#d" , ( m_nValue3 >= 0.f ) ? L"#g" : L"#r" , ( m_nValue3 >= 0 ) ? '+' : '-', abs(m_nValue3)  );
#else
				m_strValueString2 += FormatW( L" %s(%c%d)#d #y[%d%%]" , ( m_nValue3 >= 0.f ) ? L"#g" : L"#r" , ( m_nValue3 >= 0 ) ? '+' : '-', abs(m_nValue3) , abs(m_nValue4) );
#endif
			}

#if defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW)
			if( abs(m_nValue5) > 0 )
				m_strValueString2 += FormatW( L"%s(%c%d%%)" , ( m_nValue5 >= 0.f ) ? L"#s" : L"#r" , ( m_nValue5 >= 0 ) ? '+' : '-', abs(m_nValue5) );

			m_strValueString2 += FormatW( L" #y[%d%%]" , abs(m_nValue4) );
#endif

		}
		break;

	default:
		{

		}
		break;
	}

	m_bShowTooltip = bShowTooltip;


	std::wstring strToolTipString;

	strToolTipString += m_strValueString2;
	strToolTipString += L"#d\n";
	strToolTipString += m_strDetailString;
	m_strFinalString = m_strValueString2;

	if( m_bShowTooltip ) 
		SetTooltipText( strToolTipString.c_str() );

	UpdateValueCoord();

	m_bInitialized = true;
}


void CDnCharStatusStatic::Render( float fElapsedTime )
{
	SUIElement *pElement = GetElement(0);
	
	if( !pElement || !IsShow() ) 
		return;

	m_pParent->DrawDlgText( m_strValueString1.c_str() , pElement, pElement->FontColor.dwCurrentColor, m_ValueCoord1, -1, m_Property.StaticProperty.dwFontFormat );

	if( m_bShowTooltip == false )
		m_pParent->DrawDlgText( m_strFinalString.c_str() , pElement, EtInterface::textcolor::FONT_ORANGE, m_ValueCoord2, -1, m_Property.StaticProperty.dwFontFormat );
}

void CDnCharStatusStatic::OnChangeResolution()
{
	CEtUIStatic::OnChangeResolution();
	UpdateValueCoord();
}

void CDnCharStatusStatic::ClearDetailString()
{
	m_strDetailString.clear();
}

void CDnCharStatusStatic::AddDetailString( int nUIString , float fValue, bool bPercentage )
{
	m_strDetailString.clear();

	wstring wszUIString = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIString);

	if( !StrStrW(wszUIString.c_str() , L"%s") )
	{
		m_strDetailString = wszUIString;
		return;
	}

	wstring wszParameter = FormatW( L"%s%.2f%s#d" , fValue >= 0 ? L"#y+" : L"#r-" , abs( fValue ) , bPercentage ? L"%" : L"" );
	wstring wszResult = FormatW( wszUIString.c_str() , wszParameter.c_str() );
	m_strDetailString = wszResult;
}

void CDnCharStatusStatic::AddDetailString( int nUIString , int nValue, bool bPercentage )
{
	m_strDetailString.clear();

	wstring wszUIString = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIString);

	if( !StrStrW(wszUIString.c_str() , L"%s") )
	{
		m_strDetailString = wszUIString;
		return;
	}

	wstring wszParameter = FormatW( L"%s%d%s#d" , nValue >= 0 ? L"#y+" : L"#r-" , abs( nValue ) , bPercentage ? L"%" : L"" );
	wstring wszResult = FormatW( wszUIString.c_str() , wszParameter.c_str() );
	m_strDetailString = wszResult;
}

void CDnCharStatusStatic::AddBuffValue( float fBuffValue1 , float fBuffValue2 )
{
	m_fBuffValue1 = fBuffValue1;
	m_fBuffValue2 = fBuffValue2;
}

void CDnCharStatusStatic::AddBuffValue( int nBuffValue1 , int nBuffValue2 )
{
	m_fBuffValue1 = (float)nBuffValue1;
	m_fBuffValue2 = (float)nBuffValue2;
}

void CDnCharStatusStatic::ClearValue()
{
	m_strValueString1.clear();
	m_strValueString2.clear();

	m_nValue1 = m_nValue2 = m_nValue3 = m_nValue4 = m_nValue5 = m_nValue6 = 0;
	m_fValue1 = m_fValue2 = m_fValue3 = 0.f;
	m_bShowTooltip = false;
}

void CDnCharStatusStatic::ClearBuffValue()
{
	m_fBuffValue1 = m_fBuffValue2 = 0.f;
}
