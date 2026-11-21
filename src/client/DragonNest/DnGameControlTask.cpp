#include "stdafx.h"
#include "DnGameControlTask.h"
#include "InputWrapper.h"
#include "DnGameOptionTask.h"
#include "DnInterface.h"
#include "GameOption.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGameControlTask::CDnGameControlTask()
{
	SecureZeroMemory( m_ControlTypeStrings, sizeof(m_ControlTypeStrings) );

	ResetWrappingData();

	Initialize_InputWrapper();
}

CDnGameControlTask::~CDnGameControlTask()
{
	for( UINT itr = 0; itr < m_vKeyData.size(); ++itr )
		SAFE_DELETE( m_vKeyData[itr] );

	m_vKeyData.clear();
}

bool CDnGameControlTask::Initialize()
{
	InitializeControlTypeString();

	DNTableFileFormat* pSox = GetControlTable();

	if( NULL == pSox )
		return false;

	int nItemCount = pSox->GetItemCount();
	for( int itr = 0; itr < nItemCount; ++itr )
	{
		SKeyData * pData = new SKeyData;

		pData->nItemID = pSox->GetItemID( itr );
		pData->nControlType = pSox->GetFieldFromLablePtr( pData->nItemID, "_ControlType" )->GetInteger();
		pData->nCategoryType = pSox->GetFieldFromLablePtr( pData->nItemID, "_CategoryType" )->GetInteger();
		pData->nLowCategorySection = pSox->GetFieldFromLablePtr( pData->nItemID, "_LowCategorySection" )->GetInteger();
		pData->nLowCategoryType = pSox->GetFieldFromLablePtr( pData->nItemID, "_LowCategoryType" )->GetInteger();
		pData->nLowCategoryStringID = pSox->GetFieldFromLablePtr( pData->nItemID, "_LowCategory" )->GetInteger();
		pData->nDefaultKey_first = pSox->GetFieldFromLablePtr( pData->nItemID, "_DefultKey01" )->GetInteger();
		pData->nDefaultKey_second = pSox->GetFieldFromLablePtr( pData->nItemID, "_DefultKey02" )->GetInteger();
		pData->bConversion = pSox->GetFieldFromLablePtr( pData->nItemID, "_ConversionCheck" )->GetInteger() == 1 ? true : false;
		pData->bShow = pSox->GetFieldFromLablePtr( pData->nItemID, "_MonitorOutput" )->GetInteger() == 1 ? true : false;

		m_mControlType.insert( std::make_pair( pData->nControlType, m_ControlTypeStrings[pData->nControlType] ) );		
		m_vKeyData.push_back( pData );
	}

	InitializeFixedControl();

	return true;
}

DNTableFileFormat * CDnGameControlTask::GetControlTable()
{
	DNTableFileFormat* pSox = NULL;

#if defined(PRE_ADD_COUNTRY_BY_COUNTRY_CONTROL)
	MultiLanguage::SupportLanguage::eSupportLanguage eLanuage = CGlobalInfo::GetInstance().m_eLanguage;
	if( MultiLanguage::SupportLanguage::Fra == eLanuage )
		pSox = GetDNTable( CDnTableDB::TCONTROLKEY_FRA );;
	else if( MultiLanguage::SupportLanguage::Ger == eLanuage )
		pSox = GetDNTable( CDnTableDB::TCONTROLKEY_GER );
	else if( MultiLanguage::SupportLanguage::Esp == eLanuage )
		pSox = GetDNTable( CDnTableDB::TCONTROLKEY_ESP );
	else
		pSox = GetDNTable( CDnTableDB::TCONTROLKEY );
#else
	pSox = GetDNTable( CDnTableDB::TCONTROLKEY );
#endif	//	#if defined(PRE_ADD_COUNTRY_BY_COUNTRY_CONTROL)

	return pSox;
}

void CDnGameControlTask::GetDataByType( const int eControlIndex, const int eCategoryIndex, std::vector<SKeyData *> & vKeyData )
{
	vKeyData.clear();

	SKeyData * pData = NULL;
	for( UINT itr = 0; itr < m_vKeyData.size(); ++itr )
	{
		pData = m_vKeyData[itr];

		if( NULL != pData && eControlIndex == pData->nControlType && eCategoryIndex == pData->nCategoryType )
			vKeyData.push_back( pData );
	}
}

void CDnGameControlTask::GetControlTypeData( std::map<int, int> & mControlType )
{
	std::map<int, int>::const_iterator cBeginItor = m_mControlType.begin();
	std::map<int, int>::const_iterator cEndItor = m_mControlType.end();

	std::copy( cBeginItor, cEndItor, std::inserter( mControlType, mControlType.begin() ) );
}

void CDnGameControlTask::InitializeControlTypeString()
{
	m_ControlTypeStrings[ eControlType_Keyboard ]	= 2010014;	// UISTRING : Keyboard & Mouse
	m_ControlTypeStrings[ eControlType_XBox ]		= 102002;	// UISTRING : XBOX Controller
	m_ControlTypeStrings[ eControlType_Hangame ]	= 3116;		// UISTRING : Hangame Pad
	m_ControlTypeStrings[ eControlType_GAMMAC ]		= 117;		// UISTRING : GAMMAC
	m_ControlTypeStrings[ eControlType_CustomPad ]	= 102004;	// UISTRING : Custom Controller A
}

void CDnGameControlTask::InitializeFixedControl()
{
	for( DWORD itr = 0; itr < m_vKeyData.size(); ++itr )
	{
		SKeyData * pData = m_vKeyData[itr];

		if( NULL == pData )
			continue;

		if( eControlType_XBox == pData->nControlType )
		{
			if( eLowCategorySection_Motion == pData->nLowCategorySection )
			{
				g_DefaultWrappingXBOX360Data[pData->nLowCategoryType] = pData->nDefaultKey_first;
				g_DefaultWrappingXBOX360AssistData[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
			else if( eLowCategorySection_Interface == pData->nLowCategorySection )
			{
				g_DefaultUIWrappingXBOX360Data[pData->nLowCategoryType] = pData->nDefaultKey_first;
				g_DefaultUIWrappingXBOX360AssistData[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
		}
		else if( eControlType_Hangame == pData->nControlType )
		{
			if( eLowCategorySection_Motion == pData->nLowCategorySection )
			{
				g_DefaultWrappingHanData[pData->nLowCategoryType] = pData->nDefaultKey_first;
				g_DefaultWrappingHanAssistData[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
			else if( eLowCategorySection_Interface == pData->nLowCategorySection )
			{
				g_DefaultUIWrappingHanData[pData->nLowCategoryType] = pData->nDefaultKey_first;
				g_DefaultUIWrappingHanAssistData[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
		}
		else if( eControlType_GAMMAC == pData->nControlType )
		{
			if( eLowCategorySection_Motion == pData->nLowCategorySection )
			{
				g_DefaultWrappingGAMMACData[pData->nLowCategoryType] = pData->nDefaultKey_first;
				g_DefaultWrappingGAMMACAssistData[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
			else if( eLowCategorySection_Interface == pData->nLowCategorySection )
			{
				g_DefaultUIWrappingGAMMACData[pData->nLowCategoryType] = pData->nDefaultKey_first;
				g_DefaultUIWrappingGAMMACAssistData[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
		}
	}

	ChangePadType( CGameOption::GetInstance().GetInputDevice() );
}

void CDnGameControlTask::ResetWrappingData()
{
	memset( m_WrappingData, 0, sizeof(BYTE) * WrappingKeyIndex_Amount );
	memset( m_WrappingDataAssist, 0, sizeof(BYTE) * WrappingKeyIndex_Amount );
	memset( m_UI_WrappingData, 0, sizeof(BYTE) * UIWrappingKeyIndex_Amount );
	memset( m_UI_WrappingDataAssist, 0, sizeof(BYTE) * UIWrappingKeyIndex_Amount );

	memset( m_WrappingData_Pad, CInputJoyPad::NULL_VALUE, sizeof(BYTE) * WrappingKeyIndex_Amount );
	memset( m_WrappingDataAssist_Pad, CInputJoyPad::NULL_VALUE, sizeof(BYTE) * WrappingKeyIndex_Amount );
	memset( m_UI_WrappingData_Pad, CInputJoyPad::NULL_VALUE, sizeof(BYTE) * UIWrappingKeyIndex_Amount );
	memset( m_UI_WrappingDataAssist_Pad, CInputJoyPad::NULL_VALUE, sizeof(BYTE) * UIWrappingKeyIndex_Amount );
}

void CDnGameControlTask::SetWrappingDataByControlType( const int eControlIndex )
{
	ResetWrappingData();

	switch( eControlIndex )
	{
	case eControlType_Keyboard:
		{
			memcpy_s( m_WrappingData, _countof(m_WrappingData), g_WrappingKeyData, sizeof(BYTE) * WrappingKeyIndex_Amount );
			memcpy_s( m_UI_WrappingData, _countof(m_UI_WrappingData), g_UIWrappingKeyData, sizeof(BYTE) * UIWrappingKeyIndex_Amount );
		}
		break;

	case eControlType_CustomPad:
		{
			memcpy_s( m_WrappingData_Pad, _countof(m_WrappingData_Pad), g_WrappingCustomData, sizeof(BYTE) * WrappingKeyIndex_Amount );
			memcpy_s( m_WrappingDataAssist_Pad, _countof(m_WrappingDataAssist_Pad), g_WrappingCustomAssistData, sizeof(BYTE) * WrappingKeyIndex_Amount );
			memcpy_s( m_UI_WrappingData_Pad, _countof(m_UI_WrappingData_Pad), g_UIWrappingCustomData, sizeof(BYTE) * UIWrappingKeyIndex_Amount );
			memcpy_s( m_UI_WrappingDataAssist_Pad, _countof(m_UI_WrappingDataAssist_Pad), g_UIWrappingCustomAssistData, sizeof(BYTE) * UIWrappingKeyIndex_Amount );
		}
		break;
	}
}

std::pair<int, int> CDnGameControlTask::GetWrappingData( const int nControlType, const int nCategorySection, const int nCategoryType )
{
	int nFirst, nSecond = 0;

	if( eLowCategorySection_Motion == nCategorySection )
	{
		if( eControlType_Keyboard == nControlType )
		{
			nFirst = m_WrappingData[ nCategoryType ];
			nSecond = m_WrappingDataAssist[ nCategoryType ];
		}
		else if( eControlType_CustomPad == nControlType )
		{
			nFirst = m_WrappingData_Pad[ nCategoryType ];
			nSecond = m_WrappingDataAssist_Pad[ nCategoryType ];
		}
	}
	else if( eLowCategorySection_Interface == nCategorySection )
	{
		if( eControlType_Keyboard == nControlType )
		{
			nFirst = m_UI_WrappingData[ nCategoryType ];
			nSecond = m_UI_WrappingDataAssist[ nCategoryType ];
		}
		else if( eControlType_CustomPad == nControlType )
		{
			nFirst = m_UI_WrappingData_Pad[ nCategoryType ];
			nSecond = m_UI_WrappingDataAssist_Pad[ nCategoryType ];
		}
	}

	return std::pair<int, int>( nFirst, nSecond );
}

bool CDnGameControlTask::RequestChangeKey( const SChangeValue & sChangeValue, std::wstring & wszControlNameString )
{
	for( int itr = 0; itr < WrappingKeyIndex_Amount; ++itr )
	{
		if( m_WrappingData[itr] == sChangeValue.cFirstKey )
			wszControlNameString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vKeyData[itr]->nLowCategoryStringID );
	}

	for( int itr = 0; itr < UIWrappingKeyIndex_Amount; ++itr )
	{
		if( m_UI_WrappingData[itr] == sChangeValue.cFirstKey )
			wszControlNameString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vKeyData[WrappingKeyIndex_Amount + itr]->nLowCategoryStringID );
	}

	m_ChangeValue = sChangeValue;

	if( wszControlNameString.empty() )
		ChangeKey();
	else
		return false;

	return true;
}

void CDnGameControlTask::ChangeKey()
{
	for( int itr = 0; itr < WrappingKeyIndex_Amount; ++itr )
	{
		if( m_WrappingData[itr] == m_ChangeValue.cFirstKey )
			m_WrappingData[itr] = 0;
	}

	for( int itr = 0; itr < UIWrappingKeyIndex_Amount; ++itr )
	{
		if( m_UI_WrappingData[itr] == m_ChangeValue.cFirstKey )
			m_UI_WrappingData[itr] = 0;
	}

	if( eLowCategorySection_Motion == m_ChangeValue.nLowCategorySection )
		m_WrappingData[ m_ChangeValue.nLowCategoryType ] = m_ChangeValue.cFirstKey;
	else if( eLowCategorySection_Interface == m_ChangeValue.nLowCategorySection )
		m_UI_WrappingData[ m_ChangeValue.nLowCategoryType ] = m_ChangeValue.cFirstKey;
}

bool CDnGameControlTask::RequestChangePad( const SChangeValue & sChangeValue, std::wstring & wszControlNameString )
{
	bool bFirstKey, bSecondKey;
	for( int itr = 0; itr < WrappingKeyIndex_Amount; ++itr )
	{
		bFirstKey = bSecondKey = false;

		if( sChangeValue.cFirstKey == m_WrappingData_Pad[itr] )			bFirstKey = true;
		if( sChangeValue.cSecondKey == m_WrappingDataAssist_Pad[itr] )	bSecondKey = true;

		if( bFirstKey && bSecondKey )
			wszControlNameString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vKeyData[itr]->nLowCategoryStringID );
	}

	for( int itr = 0; itr < UIWrappingKeyIndex_Amount; ++itr )
	{
		bFirstKey = bSecondKey = false;

		if( sChangeValue.cFirstKey == m_UI_WrappingData_Pad[itr] )			bFirstKey = true;
		if( sChangeValue.cSecondKey == m_UI_WrappingDataAssist_Pad[itr] )	bSecondKey = true;

		if( bFirstKey && bSecondKey )
			wszControlNameString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vKeyData[WrappingKeyIndex_Amount + itr]->nLowCategoryStringID );
	}

	m_ChangeValue = sChangeValue;

	if( wszControlNameString.empty() )
		ChangePad();
	else
		return false;

	return true;
}

void CDnGameControlTask::ChangePad()
{
	bool bInsideUsedKey = false;
	bool bFirstKey, bSecondKey;
	for( int itr = 0; itr < WrappingKeyIndex_Amount; ++itr )
	{
		bFirstKey = bSecondKey = false;
		bInsideUsedKey = false;

		if( m_ChangeValue.cFirstKey == m_WrappingData_Pad[itr] )			bFirstKey = true;
		if( m_ChangeValue.cSecondKey == m_WrappingDataAssist_Pad[itr] )		bSecondKey = true;

		if( bFirstKey && bSecondKey )
			bInsideUsedKey = true;

		if( true == bInsideUsedKey )
		{
			m_WrappingData_Pad[itr] = CInputJoyPad::NULL_VALUE;
			m_WrappingDataAssist_Pad[itr] = CInputJoyPad::NULL_VALUE;
		}
	}

	for( int itr = 0; itr < UIWrappingKeyIndex_Amount; ++itr )
	{
		bFirstKey = bSecondKey = false;
		bInsideUsedKey = false;

		if( m_ChangeValue.cFirstKey == m_UI_WrappingData_Pad[itr] )			bFirstKey = true;
		if( m_ChangeValue.cSecondKey == m_UI_WrappingDataAssist_Pad[itr] )	bSecondKey = true;

		if( bFirstKey && bSecondKey )
			bInsideUsedKey = true;

		if( true == bInsideUsedKey )
		{
			m_UI_WrappingData_Pad[itr] = CInputJoyPad::NULL_VALUE;
			m_UI_WrappingDataAssist_Pad[itr] = CInputJoyPad::NULL_VALUE;
		}
	}

	if( eLowCategorySection_Motion == m_ChangeValue.nLowCategorySection )
	{
		m_WrappingData_Pad[ m_ChangeValue.nLowCategoryType ] = m_ChangeValue.cFirstKey;
		m_WrappingDataAssist_Pad[ m_ChangeValue.nLowCategoryType ] = m_ChangeValue.cSecondKey;
	}
	else if( eLowCategorySection_Interface == m_ChangeValue.nLowCategorySection )
	{
		m_UI_WrappingData_Pad[ m_ChangeValue.nLowCategoryType ] = m_ChangeValue.cFirstKey;
		m_UI_WrappingDataAssist_Pad[ m_ChangeValue.nLowCategoryType ] = m_ChangeValue.cSecondKey;
	}
}

void CDnGameControlTask::ChangeValue( const int nControlType )
{
	if( eControlType_Keyboard != nControlType && eControlType_CustomPad != nControlType )
		return;

	if( false == IsChangeValue( nControlType ) )
		return;

	for( int itr = 0; itr < WrappingKeyIndex_Amount; ++itr )
	{
		if( eControlType_Keyboard == nControlType )
		{
			g_WrappingKeyData[itr] = m_WrappingData[itr];
		}
		else if( eControlType_CustomPad == nControlType )
		{
			g_WrappingCustomData[itr] = m_WrappingData_Pad[itr];
			g_WrappingCustomAssistData[itr] = m_WrappingDataAssist_Pad[itr];
		}
	}

	for( int itr = 0; itr < UIWrappingKeyIndex_Amount; ++itr )
	{
		if( eControlType_Keyboard == nControlType )
		{
			g_UIWrappingKeyData[itr] = m_UI_WrappingData[itr];
		}
		else if( eControlType_CustomPad == nControlType )
		{
			g_UIWrappingCustomData[itr] = m_UI_WrappingData_Pad[itr];
			g_UIWrappingCustomAssistData[itr] = m_UI_WrappingDataAssist_Pad[itr];
		}
	}

	if( eControlType_Keyboard == nControlType )
		GetGameOptionTask().ReqSendKeySetting();
	else if( eControlType_CustomPad == nControlType )
		GetGameOptionTask().ReqSendJoypadSetting();

	GetInterface().ApplyUIHotKey();
}

bool CDnGameControlTask::IsChangeValue( const int nControlType )
{
	if( eControlType_Keyboard != nControlType && eControlType_CustomPad != nControlType )
		return false;

	for( int itr = 0; itr < WrappingKeyIndex_Amount; ++itr )
	{
		if( eControlType_Keyboard == nControlType 
			&& g_WrappingKeyData[itr] != m_WrappingData[itr] )
		{
			return true;
		}
		else if( eControlType_CustomPad == nControlType 
			&& ( g_WrappingCustomData[itr] != m_WrappingData_Pad[itr] || g_WrappingCustomAssistData[itr] != m_WrappingDataAssist_Pad[itr] ) )
		{
			return true;
		}
	}

	for( int itr = 0; itr < UIWrappingKeyIndex_Amount; ++itr )
	{
		if( eControlType_Keyboard == nControlType 
			&& g_UIWrappingKeyData[itr] != m_UI_WrappingData[itr] )
		{
			return true;
		}
		else if( eControlType_CustomPad == nControlType 
			&& ( g_UIWrappingCustomData[itr] != m_UI_WrappingData_Pad[itr] || g_UIWrappingCustomAssistData[itr] != m_UI_WrappingDataAssist_Pad[itr] ) )
		{
			return true;
		}
	}

	return false;
}

void CDnGameControlTask::DefaultValue( const int nControlType )
{
	if( eControlType_Keyboard != nControlType && eControlType_CustomPad != nControlType )
		return;

	for( DWORD itr = 0; itr < m_vKeyData.size(); ++itr )
	{
		SKeyData * pData = m_vKeyData[itr];

		if( NULL == pData || nControlType != pData->nControlType )
			continue;

		if( eLowCategorySection_Motion == pData->nLowCategorySection )
		{
			if( eControlType_Keyboard == nControlType )
			{
				m_WrappingData[pData->nLowCategoryType] = pData->nDefaultKey_first;
				m_WrappingDataAssist[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
			else if( eControlType_CustomPad == nControlType )
			{
				m_WrappingData_Pad[pData->nLowCategoryType] = pData->nDefaultKey_first;
				m_WrappingDataAssist_Pad[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
		}
		else if( eLowCategorySection_Interface == pData->nLowCategorySection )
		{
			if( eControlType_Keyboard == nControlType )
			{
				m_UI_WrappingData[pData->nLowCategoryType] = pData->nDefaultKey_first;
				m_UI_WrappingDataAssist[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
			else if( eControlType_CustomPad == nControlType )
			{
				m_UI_WrappingData_Pad[pData->nLowCategoryType] = pData->nDefaultKey_first;
				m_UI_WrappingDataAssist_Pad[pData->nLowCategoryType] = pData->nDefaultKey_second;
			}
		}
	}
}

bool CDnGameControlTask::IsDefaultValue( const int nControlType )
{
	if( eControlType_Keyboard != nControlType && eControlType_CustomPad != nControlType )
		return true;

	int nFirstKey = 0, nSecondKey = 0;
	for( DWORD itr = 0; itr < m_vKeyData.size(); ++itr )
	{
		SKeyData * pData = m_vKeyData[itr];

		if( NULL == pData || nControlType != pData->nControlType )
			continue;

		if( eLowCategorySection_Motion == pData->nLowCategorySection )
		{
			if( eControlType_Keyboard == nControlType )
			{
				nFirstKey = m_WrappingData[pData->nLowCategoryType];
				nSecondKey = m_WrappingDataAssist[pData->nLowCategoryType];
			}
			else if( eControlType_CustomPad == nControlType )
			{
				nFirstKey = m_WrappingData_Pad[pData->nLowCategoryType];
				nSecondKey = m_WrappingDataAssist_Pad[pData->nLowCategoryType];
			}
		}
		else if( eLowCategorySection_Interface == pData->nLowCategorySection )
		{
			if( eControlType_Keyboard == nControlType )
			{
				nFirstKey = m_UI_WrappingData[pData->nLowCategoryType];
				nSecondKey = m_UI_WrappingDataAssist[pData->nLowCategoryType];
			}
			else if( eControlType_CustomPad == nControlType )
			{
				nFirstKey = m_UI_WrappingData_Pad[pData->nLowCategoryType];
				nSecondKey = m_UI_WrappingDataAssist_Pad[pData->nLowCategoryType];
			}
		}

		if( pData->nDefaultKey_first != nFirstKey || pData->nDefaultKey_second != nSecondKey )
			return false;
	}

	return true;
}

void CDnGameControlTask::ChangePadType( const int nControlType )
{
	if( eControlType_Keyboard == nControlType )
		return;

	switch( nControlType )
	{
	case eControlType_XBox:
		{
			g_WrappingJoypadData = g_DefaultWrappingXBOX360Data;
			g_WrappingJoypadAssistData = g_DefaultWrappingXBOX360AssistData;
			g_UIWrappingJoypadData = g_DefaultUIWrappingXBOX360Data;
			g_UIWrappingJoypadAssistData = g_DefaultUIWrappingXBOX360AssistData;
		}
		break;

	case eControlType_Hangame:
		{
			g_WrappingJoypadData = g_DefaultWrappingHanData;
			g_WrappingJoypadAssistData = g_DefaultWrappingHanAssistData;
			g_UIWrappingJoypadData = g_DefaultUIWrappingHanData;
			g_UIWrappingJoypadAssistData = g_DefaultUIWrappingHanAssistData;
		}
		break;

	case eControlType_GAMMAC:
		{
			g_WrappingJoypadData = g_DefaultWrappingGAMMACData;
			g_WrappingJoypadAssistData = g_DefaultWrappingGAMMACAssistData;
			g_UIWrappingJoypadData = g_DefaultUIWrappingGAMMACData;
			g_UIWrappingJoypadAssistData = g_DefaultUIWrappingGAMMACAssistData;
		}
		break;

	case eControlType_CustomPad:
		{
			g_WrappingJoypadData = g_WrappingCustomData;
			g_WrappingJoypadAssistData = g_WrappingCustomAssistData;
			g_UIWrappingJoypadData = g_UIWrappingCustomData;
			g_UIWrappingJoypadAssistData = g_UIWrappingCustomAssistData;
		}
		break;
	}
}