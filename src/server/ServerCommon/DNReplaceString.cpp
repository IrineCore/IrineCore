#include "stdafx.h"
#include "DNReplaceString.h"
#include "EtUIXML.h"

DNReplaceString g_ReplaceString;
DNReplaceString::DNReplaceString()
{
	m_StaticReplaceList.clear();
	m_StaticReplaceList.push_back( __StaticReplaceStringInfo( __StaticReplaceStringInfo::user_nick,		L"user_nick") );
	m_StaticReplaceList.push_back( __StaticReplaceStringInfo( __StaticReplaceStringInfo::user_class,	L"user_class") );
	m_StaticReplaceList.push_back( __StaticReplaceStringInfo( __StaticReplaceStringInfo::monster,		L"monster") );
	m_StaticReplaceList.push_back( __StaticReplaceStringInfo( __StaticReplaceStringInfo::npc,			L"npc") );
	m_StaticReplaceList.push_back( __StaticReplaceStringInfo( __StaticReplaceStringInfo::item,			L"item") );
	m_StaticReplaceList.push_back( __StaticReplaceStringInfo( __StaticReplaceStringInfo::skill,			L"skill") );
	m_StaticReplaceList.push_back( __StaticReplaceStringInfo( __StaticReplaceStringInfo::map,			L"map") );
}

DNReplaceString::~DNReplaceString()
{

}

std::wstring DNReplaceString::_GetStaticString(int nType, std::vector<std::wstring>& tokens)
{
	switch( nType )
	{

	case __StaticReplaceStringInfo::user_nick:
		{
			//if ( CDnActor::s_hLocalActor ) return CDnActor::s_hLocalActor->GetName();
		}
		break;
	case __StaticReplaceStringInfo::user_class:
		break;
	case __StaticReplaceStringInfo::monster:
		{
			DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMONSTER );
			int nMonID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nMonID, "_NameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;
		}
		break;
	case __StaticReplaceStringInfo::npc:	
		{
			DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TNPC );
			int nID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_NameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;

		}
		break;
	case __StaticReplaceStringInfo::item:
		{
			DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TITEM );
			int nID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_NameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;
		}
		break;
	case __StaticReplaceStringInfo::skill:	
		{
			DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TSKILL );
			int nID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_NameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;
		}
		break;
	case __StaticReplaceStringInfo::map:
		{
			DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
			int nID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_MapNameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;
		}
		break;
	}

	return std::wstring(L"");
}

std::wstring DNReplaceString::_GetString(int nStringID)
{
	std::wstring wszString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID );

	return wszString;
}


bool DNReplaceString::Relpace(IN OUT std::wstring& str, std::vector<DynamicReplaceStringInfo>& info)
{

	ReplaceDynamicString(str,info);
	ReplaceStaticString(str);

	return true;
}

void DNReplaceString::ReplaceStaticString(IN OUT std::wstring& str)
{
	// Finds keywords enclosed in {}.
	std::vector<std::wstring> tokens;
	TokenizeW(str, tokens, std::wstring(L"{"), std::wstring(L"}"), false);

	for ( int j = 0 ; j < (int)tokens.size() ; j++ )
	{
		std::vector<std::wstring> child_tokens;
		TokenizeW(tokens[j], child_tokens, L":");

		// Separate tokens that have ':' attached after the keyword.
		for ( int k = 0 ; k < (int)child_tokens.size() ; k++ )
		{
			for ( int i = 0 ; i < (int)m_StaticReplaceList.size() ; i++ )
			{
				if ( child_tokens[0] == m_StaticReplaceList[i].szKey )
				{
					std::wstring wsz = _GetStaticString(m_StaticReplaceList[i].nType, child_tokens);
					AllReplaceW(str, tokens[j], wsz);
					RemoveStringW(str, std::wstring(L"{"));
					RemoveStringW(str, std::wstring(L"}"));
				}
			}
		}
	}
}

void DNReplaceString::ReplaceDynamicString(IN OUT std::wstring& str, std::vector<DynamicReplaceStringInfo>& info)
{
	for ( size_t i = 0 ; i < info.size() ; i++ )
	{
		std::wstring wszKey;
		std::wstring wsz;
		if ( info[i].cValueType == DynamicReplaceStringInfo::INT )
		{
			wsz = FormatW( L"%d", info[i].nValue );
		}
		else if ( info[i].cValueType == DynamicReplaceStringInfo::STRING )
		{
			wsz = _GetString(info[i].nValue);
		}
		else
		{
			// Ugh
		}

		if ( !wsz.empty() )
		{
			AllReplaceW( str, info[i].szKey, wsz);
		}
	}

}