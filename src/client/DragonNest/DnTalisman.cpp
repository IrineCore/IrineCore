#include "StdAfx.h"

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnTalisman.h"
#include "DnTableDB.h"
#include "DnUIString.h"
#include "SyncTimer.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

DECL_DN_SMART_PTR_STATIC( CDnTalisman, 2000 )

CDnTalisman::CDnTalisman()
: CDnUnknownRenderObject( false )
, m_TalismanType( Normal )
, m_nTalismanIndex( 0 )
, m_fExpectation( 0.0f )
, m_fEfficiency( 1.0f )
{
}

CDnTalisman::~CDnTalisman()
{
}

DnTalismanHandle CDnTalisman::CreateTalisman( int nTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound, int nLookItemID )
{
	if( nTableID < 1 ) return CDnTalisman::Identity();

	DnTalismanHandle hTalisman = (new CDnTalisman)->GetMySmartPtr();
	if( !hTalisman ) return CDnTalisman::Identity();

	hTalisman->Initialize( nTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound, nLookItemID );
	return hTalisman;
}

bool CDnTalisman::Initialize( int nTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound, int nLookItemID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TTALISMAN );
	if( !pSox ) return false;
	if( !pSox->IsExistItem( nTableID ) ) return false;

	m_TalismanType = (TalismanTypeEnum)pSox->GetFieldFromLablePtr( nTableID, "_Type" )->GetInteger();
	m_nTalismanIndex = pSox->GetFieldFromLablePtr( nTableID, "_Index" )->GetInteger();
	m_fExpectation = pSox->GetFieldFromLablePtr( nTableID, "_Expectation" )->GetFloat();
	m_fEfficiency = 1.0f;

	CDnItem::Initialize( nTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound, nLookItemID );

	return true;
}

bool CDnTalisman::IsExpireItem() const
{
	if( !IsEternityItem() )
	{
		const __time64_t *pExpireDate = GetExpireDate();
		if( pExpireDate && *pExpireDate <= CSyncTimer::GetInstance().GetCurTime() )
			return true;
	}
	return false;
}

void CDnTalisman::GetTalismanExplainString( std::wstring &wszOut, float fEfficiency )
{
	wszOut.clear();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TTALISMAN );
	if( !pSox || !pSox->IsExistItem( GetClassID() ) )
		return;

	int nDescID = pSox->GetFieldFromLablePtr( GetClassID(), "_Description" )->GetInteger();
	if( nDescID <= 0 )
		return;

	std::string szDescParam = pSox->GetFieldFromLablePtr( GetClassID(), "_DescriptionParam" )->GetString();
	std::vector<std::string> vecTokens;
	TokenizeA( szDescParam, vecTokens, "," );

	if( !vecTokens.empty() )
	{
		if( fEfficiency > 1.0f )
		{
			for( size_t i = 0; i < vecTokens.size(); ++i )
			{
				// Skip UIString placeholders such as {12345}
				if( vecTokens[i].find( '{' ) == std::string::npos )
				{
					long nVal = atol( vecTokens[i].c_str() );
					if( nVal != 0 )
					{
						int nScaled = (int)( nVal * fEfficiency );
						char szBuf[128] = {0,};
						_itoa_s( nScaled, szBuf, _countof(szBuf), 10 );
						vecTokens[i] = szBuf;
					}
				}
			}
		}

		std::string szJoined;
		for( size_t i = 0; i < vecTokens.size(); ++i )
		{
			if( i > 0 ) szJoined += ",";
			szJoined += vecTokens[i];
		}

		MakeUIStringUseVariableParam( wszOut, nDescID, const_cast<char*>( szJoined.c_str() ) );
	}
	else
	{
		wszOut = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nDescID );
	}

	m_wszExplainString = wszOut;
}

#endif // PRE_ADD_TALISMAN_SYSTEM
