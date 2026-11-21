#pragma once

// Removed from DnInterfaceString because the server also needs to use it.

#ifdef PRE_ADD_WORD_GENDER
#define ADJECTIVE_GENDER_TAGE L"@gender:"
#define NOUN_GENDER_TAGE L"#gender:"

void ChangeWordOrder( std::vector<std::string>& vecParam );
std::wstring ApplyWordGenderString( bool bIsAdjective, std::wstring& wszParamString, std::vector<std::string>& vecParam );
#endif // PRE_ADD_WORD_GENDER

#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
void MakeUIStringUseVariableParamByBaseMsg( std::wstring &wszStr, std::wstring& wszBaseMessage, char *szParam, std::vector <std::pair<std::wstring, int>> * pvList = NULL );
void MakeUIStringUseVariableParam( std::wstring &wszStr, int nMessageId, char *szParam, std::vector <std::pair<std::wstring, int>> * pvList = NULL );
#else		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
void MakeUIStringUseVariableParamByBaseMsg( std::wstring &wszStr, std::wstring& wszBaseMessage, char *szParam );
void MakeUIStringUseVariableParam( std::wstring &wszStr, int nMessageId, char *szParam );
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
#if defined(PRE_ADD_ITEM_GAINTABLE) && defined(_CLIENT)
void MakeUIStringItemGainDescription(std::wstring& result, const std::string& baseMessage);
#endif


// String parsing - fetch and combine the trailing string according to the format of the leading string.
//
// : "{0} {1} Achieved on kill" | "{foo},{abc}" => foo abc Achieved on kill
// 
std::wstring ParseUIParamString( int destID, char * pStrParam );