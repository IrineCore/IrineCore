#pragma once


class DNReplaceString
{
public:
	struct  __StaticReplaceStringInfo
	{
		enum 
		{
			user_nick = 1,
			user_class,
			monster,
			npc,
			item,
			skill,
			map
		};
		int nType;
		std::wstring	szKey;
		__StaticReplaceStringInfo(int type, const WCHAR* wszKey)
		{
			nType = type;
			szKey = wszKey;
		}
	};

	struct DynamicReplaceStringInfo 
	{
		enum
		{
			STRING = 1,
			INT,
		};
		char		 cValueType; 
		std::wstring szKey;
		int			 nValue;
	};

	DNReplaceString();
	virtual ~DNReplaceString();

	// Complete the string by replacing placeholders.
	// Call ReplaceStaticString first for predefined substitution
	// Call ReplaceDynamicString for dynamic substitution.
	bool Relpace(IN OUT std::wstring& str, std::vector<DynamicReplaceStringInfo>& info);

	// Name of monster #10
	// Replaces predefined-type strings such as {%monster:10}.
	void ReplaceStaticString(IN OUT std::wstring& str);

	// Index stored separately in uistring == aaa, specified externally
	// Replaces dynamic-type strings such as {%aaa}.
	void ReplaceDynamicString(IN OUT std::wstring& str, std::vector<DynamicReplaceStringInfo>& info);

private:
	std::wstring _GetStaticString(int nType, std::vector<std::wstring>& tokens);
	std::wstring _GetString(int nStringID);

	std::vector<__StaticReplaceStringInfo>	m_StaticReplaceList;

};

extern DNReplaceString	g_ReplaceString;