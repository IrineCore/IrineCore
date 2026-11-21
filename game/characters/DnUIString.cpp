#include "stdafx.h"
#include "EtUIXML.h"
#include "DnUIString.h"
#if defined(_CLIENT)
#include "DnInterfaceString.h"
#endif	// #if defined(_CLIENT)

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#ifdef PRE_ADD_WORD_GENDER
void ChangeWordOrder( std::vector<std::string>& vecParam )
{
	std::wstring strString;
	std::string strSubStr;
	int nAdjectiveIndex = -1;
	int nNounIndex = -1;

	for( int i=0; i<static_cast<int>( vecParam.size() ); i++ )
	{
		std::string strParm = vecParam[i];
		if( strParm[0] == '{' && strParm[strParm.size()-1] == '}' )
			strSubStr = strParm.substr( 1, strParm.size() - 2 );
		strString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, atoi( strSubStr.c_str() ) );
		std::wstring::size_type GenderTag = strString.find( ADJECTIVE_GENDER_TAGE );
		if( GenderTag != std::wstring::npos )
		{
			nAdjectiveIndex = i;
			for( int j=i+1; j<static_cast<int>( vecParam.size() ); j++ )
			{
				strParm = vecParam[j];
				if( strParm[0] == '{' && strParm[strParm.size()-1] == '}' )
					strSubStr = strParm.substr( 1, strParm.size() - 2 );
				strString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, atoi( strSubStr.c_str() ) );
				GenderTag = strString.find( NOUN_GENDER_TAGE );
				if( GenderTag != std::wstring::npos )
				{
					nNounIndex = j;
					break;
				}
			}

			break;
		}
	}

	if( nAdjectiveIndex >= 0 && nNounIndex > nAdjectiveIndex )
	{
		std::string strNounParam = vecParam[nNounIndex];
		vecParam.erase( vecParam.begin() + nNounIndex );
		vecParam.insert( vecParam.begin() + nAdjectiveIndex + 1, strNounParam );
	}
}

std::wstring ApplyWordGenderString( bool bIsAdjective, std::wstring& wszParamString, std::vector<std::string>& vecParam )
{
	std::wstring strApplyWordGenderString = wszParamString;

	if( bIsAdjective )
	{
		std::wstring strNounGender;
		std::string strSubStr;
		for( int i=0; i<static_cast<int>( vecParam.size() ); i++ )
		{
			std::string strParm = vecParam[i];
			if( strParm[0] == '{' && strParm[strParm.size()-1] == '}' )
				strSubStr = strParm.substr( 1, strParm.size() - 2 );
			strNounGender = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, atoi( strSubStr.c_str() ) );
			std::wstring::size_type GenderTag = strNounGender.find( NOUN_GENDER_TAGE );
			if( GenderTag != std::wstring::npos )
			{
				strNounGender.erase( 0, GenderTag + lstrlenW( NOUN_GENDER_TAGE ) );
				std::transform( strNounGender.begin(), strNounGender.end(), strNounGender.begin(), towlower );
				break;
			}
		}

		if( strNounGender != L"" )
		{
			std::wstring strGenderTag;
			std::wstring::size_type GenderTag = strApplyWordGenderString.find( ADJECTIVE_GENDER_TAGE );
			if( GenderTag != std::wstring::npos )
			{
				strGenderTag = strApplyWordGenderString;
				strGenderTag.erase( 0, GenderTag );
				strApplyWordGenderString.erase( GenderTag, strApplyWordGenderString.length() - GenderTag );

				if( wcscmp( strNounGender.c_str(), L"m" ) == 0 )
				{
					std::wstring::size_type SlashIndex = strGenderTag.find( L"/" );
					if( SlashIndex != std::wstring::npos )
					{
						strGenderTag.erase( SlashIndex, strGenderTag.length() - SlashIndex );
						strGenderTag.erase( 0, lstrlenW( NOUN_GENDER_TAGE ) );
						strApplyWordGenderString += strGenderTag;
					}
				}
				else if( wcscmp( strNounGender.c_str(), L"f" ) == 0 )
				{
					std::wstring::size_type SlashIndex = strGenderTag.rfind( L"/" );
					if( SlashIndex != std::wstring::npos )
					{
						strGenderTag.erase( SlashIndex, strGenderTag.length() - SlashIndex );
						SlashIndex = strGenderTag.find( L"/" );
						if( SlashIndex != std::wstring::npos )
						{
							strGenderTag.erase( 0, SlashIndex + 1 );
							strApplyWordGenderString += strGenderTag;
						}
					}
				}
				else if( wcscmp( strNounGender.c_str(), L"n" ) == 0 )
				{
					std::wstring::size_type SlashIndex = strGenderTag.rfind( L"/" );
					if( SlashIndex != std::wstring::npos )
					{
						strGenderTag.erase( 0, SlashIndex + 1 );
						strApplyWordGenderString += strGenderTag;
					}
				}
			}
		}
	}
	else
	{
		std::wstring::size_type GenderTag = strApplyWordGenderString.find( NOUN_GENDER_TAGE );
		if( GenderTag != std::wstring::npos )
			strApplyWordGenderString.erase( GenderTag, strApplyWordGenderString.length() - GenderTag );
	}

	return strApplyWordGenderString;
}
#endif // PRE_ADD_WORD_GENDER

#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
void MakeUIStringUseVariableParamByBaseMsg( std::wstring &wszStr, std::wstring& wszBaseMessage, char *szParam, std::vector <std::pair<std::wstring, int>> * pvList )
#else		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
void MakeUIStringUseVariableParamByBaseMsg( std::wstring &wszStr, std::wstring& wszBaseMessage, char *szParam )
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
{
	// English, Korean, or decimals will never appear here, so only handle {int} or numbers.
	// Store parameter in string form.
	std::vector<std::string> vecParam;
	if( szParam != NULL && (int)strlen(szParam) > 0 )
	{
		for( int i=0;; i++ )
		{
			const char *pStr = _GetSubStrByCount( i, szParam, ',' );
			if( pStr == NULL || (int)strlen(pStr) == 0 ) break;
			vecParam.push_back(pStr);
		}
	}

#ifdef PRE_ADD_WORD_GENDER
	if( static_cast<int>( vecParam.size() ) > 2 )
		ChangeWordOrder( vecParam );
#endif // PRE_ADD_WORD_GENDER

	int nOffset = 0;
	// Find and replace substitution strings in BaseMessage.
	while(1)
	{
		// nOffset is used to search after the placeholder string when it was not replaced (e.g., due to missing parameter).
		// If replaced properly, keep searching from the beginning.
		std::wstring::size_type nPos = wszBaseMessage.find_first_of( L"{", nOffset );
		std::wstring::size_type nPos2 = wszBaseMessage.find_first_of( L"}", nOffset );
		if( (nPos != std::wstring::npos) && (nPos2 != std::wstring::npos) )
		{
			// Exception handling: when } appears before {, search again from that point.
			if( nPos2 < nPos )
			{
				nOffset = (int)nPos2+1;
				continue;
			}

			std::wstring wszParamIndex = wszBaseMessage.substr(nPos+1, nPos2-nPos-1);

#ifndef _FINAL_BUILD
			// Exception handling: when nothing is inside {},
			if( wszParamIndex.size() == 0 )
				_ASSERT(0&&"No number inside replacement string braces while composing UIString.");

			// Exception handling: when something other than a number is inside {},
			for( int i = 0; i < (int)wszParamIndex.size(); ++i )
			{
				if( L'0' <= wszParamIndex[i] && wszParamIndex[i] <= L'9' )
				{
				}
				else
				{
					_ASSERT(0&&"Non-numeric string inside replacement string braces while composing UIString.");
				}
			}
#endif

			int nIndex = _wtoi(wszParamIndex.c_str());

			// Check if index is valid; if invalid, leave replacement content as is.
			if( nIndex < 0 || nIndex+1 > (int)vecParam.size() )
			{
				nOffset = (int)nPos2+1;
				continue;
			}

			// Retrieves the parameter matching the parameter index.
			//std::string szParam = vecParam[nIndex]; // Local declaration hides declaration of the same name in outer scope (code analysis)
			std::string wszParam = vecParam[nIndex];		
			std::wstring wszParamString;
#ifdef PRE_ADD_WORD_GENDER
			bool bIsAdjective = false;
#endif // PRE_ADD_WORD_GENDER
			// Determines whether the parameter is a plain number or {number} and converts it to a string (assuming no whitespace in data).
			if( wszParam[0] == '{' && wszParam[wszParam.size()-1] == '}' )
			{
				std::string szSubStr = wszParam.substr(1, wszParam.size()-2);

#ifndef _FINAL_BUILD
				// Exception handling: when nothing is inside {},
				if( szSubStr.size() == 0 )
					_ASSERT(0&&"No number inside parameter braces while composing UIString.");

				// Exception handling: when something other than a number is inside {},
				for( int i = 0; i < (int)szSubStr.size(); ++i )
				{
					if( L'0' <= szSubStr[i] && szSubStr[i] <= L'9' )
					{
					}
					else
					{
						_ASSERT(0&&"Non-numeric string inside parameter braces while composing UIString.");
					}
				}
#endif

				wszParamString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, atoi(szSubStr.c_str()) );

#ifdef PRE_ADD_WORD_GENDER
				std::wstring strParam;
				strParam = wszParamString;
				std::wstring::size_type GenderTag = strParam.find( ADJECTIVE_GENDER_TAGE );
				if( GenderTag != std::wstring::npos )
				{
					bIsAdjective = true;
					strParam.erase( GenderTag, strParam.length() - GenderTag );
				}
				GenderTag = strParam.find( NOUN_GENDER_TAGE );
				if( GenderTag != std::wstring::npos )
					strParam.erase( GenderTag, strParam.length() - GenderTag );
#endif // PRE_ADD_WORD_GENDER

#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
				if (pvList)
				{
#ifdef PRE_ADD_WORD_GENDER
					pvList->push_back(std::make_pair(strParam, atoi(szSubStr.c_str())));
#else // PRE_ADD_WORD_GENDER
					pvList->push_back(std::make_pair(wszParamString, atoi(szSubStr.c_str())));
#endif // PRE_ADD_WORD_GENDER
				}
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE

#ifndef _FINAL_BUILD
				// Exception handling: when {} in parameter is converted to UIString and still contains braces {}
				for( int i = 0; i < (int)wszParamString.size(); ++i )
				{
					if( wszParamString[i] == L'{' || wszParamString[i] == L'}' )
					{
						_ASSERT(0&&"Braces {} found inside parameter brace value while composing UIString.");
					}
				}
#endif
			}
			else
			{
				ToWideString( wszParam, wszParamString );
			}
#ifdef PRE_ADD_WORD_GENDER
			wszParamString = ApplyWordGenderString( bIsAdjective, wszParamString, vecParam );
#endif // PRE_ADD_WORD_GENDER
			// Insert parameter string value at the {number} placeholder.
			wszBaseMessage.replace( nPos, nPos2-nPos+1, wszParamString );
		}
		else
		{
			// Return result value and exit
			wszStr = wszBaseMessage;
			break;
		}
	}
}

#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
void MakeUIStringUseVariableParam( std::wstring &wszStr, int nMessageId, char *szParam, std::vector <std::pair<std::wstring, int>> * pvList )
#else		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
void MakeUIStringUseVariableParam( std::wstring &wszStr, int nMessageId, char *szParam )
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
{
	std::wstring wszBaseMessage = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMessageId );

#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
	MakeUIStringUseVariableParamByBaseMsg(wszStr, wszBaseMessage, szParam, pvList);
#else		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
	MakeUIStringUseVariableParamByBaseMsg(wszStr, wszBaseMessage, szParam);
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE

#ifdef PRE_ADD_EXCEPTIONAL_MAKESTRING
	if( szParam != NULL && static_cast<int>( strlen( szParam ) ) > 0 )
	{
		std::wstring strExceptional = GetEtExeptionalUIXML().GetReplacementUIString( szParam );
		if( strExceptional.length() > 0 )
			wszStr = strExceptional;
	}
#endif // PRE_ADD_EXCEPTIONAL_MAKESTRING
}

#if defined(PRE_ADD_ITEM_GAINTABLE) && defined(_CLIENT)
void MakeUIStringItemGainDescription(std::wstring& result, const std::string& baseMessage)
{
	if (baseMessage.empty())
		return;

	std::wstring convertBaseMessage;
	DN_INTERFACE::UTIL::String2Wstring(convertBaseMessage, baseMessage);

	int nOffset = 0;
	while(1)
	{
		// nOffset is used to search after the placeholder string when it was not replaced (e.g., due to missing parameter).
		// If replaced properly, keep searching from the beginning.
		std::wstring::size_type nPos = convertBaseMessage.find_first_of(L"{", nOffset);
		std::wstring::size_type nPos2 = convertBaseMessage.find_first_of(L"}", nOffset);
		if ((nPos != std::wstring::npos) && (nPos2 != std::wstring::npos))
		{
			// Exception handling: when } appears before {, search again from that point.
			if (nPos2 < nPos)
			{
				nOffset = (int)nPos2+1;
				continue;
			}

			std::wstring wszParamIndex = convertBaseMessage.substr(nPos+1, nPos2-nPos-1);

#ifndef _FINAL_BUILD
			// Exception handling: when nothing is inside {},
			if( wszParamIndex.size() == 0 )
				_ASSERT(0&&"No number inside replacement string braces while composing UIString.");

			// Exception handling: when something other than a number is inside {},
			for( int i = 0; i < (int)wszParamIndex.size(); ++i )
			{
				if( L'0' <= wszParamIndex[i] && wszParamIndex[i] <= L'9' )
				{
				}
				else
				{
					_ASSERT(0&&"Non-numeric string inside replacement string braces while composing UIString.");
				}
			}
#endif
			int nIndex = _wtoi(wszParamIndex.c_str());

			// Check if index is valid; if invalid, leave replacement content as is.
			if (nIndex <= 0)
			{
				nOffset = (int)nPos2+1;
				continue;
			}

			std::wstring wszParamString = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nIndex);
			if (wszParamString.empty() == false)
				convertBaseMessage.replace(nPos, nPos2-nPos+1, wszParamString);
		}
		else
		{
			// Return result value and exit
			result = convertBaseMessage;
			break;
		}
	}
}
#endif


// String parsing - fetch and combine the trailing string according to the format of the leading string.
//
// : "{0} {1} Achieved on kill" | "{foo},{abc}" => foo abc Achieved on kill
// 
std::wstring ParseUIParamString( int destID, char * pStrParam )
{
	std::wstring strParam;
	ToWideString( pStrParam, strParam );

	std::vector< std::wstring > tokens;
	TokenizeW( strParam, tokens, L"," );

	std::wstring str( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, destID ) );	
	std::wstring strTemp( str );

	std::wstring::size_type preBegin = 0;
	std::wstring::size_type begin = 0;
	std::wstring::size_type end = 0;
	while( 1 )
	{
		begin = strTemp.find_first_of( L"{" );
		if( std::wstring::npos == begin )
			break;

		end = strTemp.find_first_of( L"}" );
		if( std::wstring::npos == end )
			break;	

		std::wstring strNum = strTemp.substr( begin + 1, (end-1) - begin );
		int num = _wtoi( strNum.c_str() );

		std::wstring::size_type addSize = 0;
		const wchar_t * strL = NULL;
		std::wstring & strToken = tokens[num];
		if( std::wstring::npos == strToken.find( L"{" ) )
		{
			str.replace( preBegin+begin, end-begin+1, strToken );
			addSize = wcslen( strToken.c_str() );
		}
		else if( strToken.size() > 2 )
		{
			std::wstring _str = strToken.substr( 1, strToken.size()-2 );
			strL = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, _wtoi(_str.c_str()) );
			str.replace( preBegin+begin, end-begin+1, strL );

			addSize = wcslen( strL );
		}

		strTemp = strTemp.substr( end+1, strTemp.size() );

		/*if( strL )
			preBegin += wcslen( strL );
		else
			preBegin += end;*/
			
		preBegin += begin + addSize;
			
	}

	return str;
}