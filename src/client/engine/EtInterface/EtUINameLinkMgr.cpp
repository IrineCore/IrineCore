#include "StdAfx.h"
#include "EtUINameLinkMgr.h"
#include "EtUINameLinkInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtUINameLinkMgr::CEtUINameLinkMgr()
{
}

bool CEtUINameLinkMgr::ParseText(std::wstring& parsee, int& tagOffset, CEtUINameLinkInterface* pInterface) const
{
	if (parsee.empty())
		return false;

	std::wstring command;
	command = parsee.substr(0, GetCmdLength());
	parsee = parsee.substr(GetCmdLength(), std::wstring::npos);
	tagOffset++;

	//	todo : Refactor to command pattern if it grows beyond one screen by kalliste
	if (command.compare(L"c") == 0)
	{
		DWORD color = wcstoul(parsee.c_str(), NULL, 16);
		if (parsee.length() < 8)
		{
			tagOffset += int(parsee.length());
			if (pInterface)
				pInterface->OnParseTextFailColor();
			return false;
		}

		parsee = parsee.substr(8, std::wstring::npos);
		tagOffset += 8;

		if (pInterface)
			pInterface->OnParseTextColor(color);
	}
	else if (command.compare(L"r") == 0)
	{
		if (pInterface)
			pInterface->OnParseTextRestoreColor();
	}
	else if (command.compare(L"i") == 0)
	{
		std::wstring::size_type tagContentsSize = parsee.find_first_of(GetNameLinkTagCloser());
		if (tagContentsSize != std::wstring::npos)
		{
			if (pInterface)
			{
				std::wstring args = parsee.substr(0, tagContentsSize);
				pInterface->OnParseTextItemInfo(args);
			}

			parsee = parsee.substr(tagContentsSize + 1, std::wstring::npos);
			tagOffset += int(tagContentsSize) + 1; // + closer size
		}
		else
		{
			tagOffset += int(parsee.length());
			return false;
		}
	}
	else if (command.compare(L"k") == 0)
	{
		std::wstring::size_type tagContentsSize = parsee.find_first_of(GetNameLinkTagCloser());
		if (tagContentsSize != std::wstring::npos)
		{
			if (pInterface)
			{
				std::wstring args = parsee.substr(0, tagContentsSize);
				pInterface->OnParseTextSkillInfo(args);
			}

			parsee = parsee.substr(tagContentsSize + 1, std::wstring::npos);
			tagOffset += int(tagContentsSize) + 1; // + closer size
		}
		else
		{
			tagOffset += int(parsee.length());
			return false;
		}
	}
	else if (command.compare(L"n") == 0)
	{
		std::wstring::size_type tagContentsSize = parsee.find_first_of(GetNameLinkSeperator());
		if (tagContentsSize != std::wstring::npos)
		{
			if (pInterface)
			{
				std::wstring args = parsee.substr(0, tagContentsSize);
				pInterface->OnParseTextUserName(args);
			}
		}
	}
	else if (command.compare(L"l") == 0)
	{
		std::wstring::size_type tagContentsSize = parsee.find_last_of(GetNameLinkSeperator());
		if (pInterface)
		{
			std::wstring args = parsee.substr(10, tagContentsSize-10);
			pInterface->OnOpenHyperLink(args);
		}
	}

	return true;
}

// todo by kalliste : Refactoring with ParseText(std::wstring& parsee, int& tagOffset) - ELIMINATE DUPLICATION
bool CEtUINameLinkMgr::ParseText(SWORD_PROPERTY& prop, std::wstring& parsee, int& tagOffset) const
{
	if (parsee.empty())
		return false;

	std::wstring command;
	command = parsee.substr(0, GetCmdLength());
	parsee = parsee.substr(GetCmdLength(), std::wstring::npos);
	tagOffset++;

	//	todo : Refactor to command pattern if it grows beyond one screen by kalliste
	if (command.compare(L"c") == 0)
	{
		DWORD color = wcstoul(parsee.c_str(), NULL, 16);
		if (parsee.length() < 8)
		{
			tagOffset += int(parsee.length());
			return false;
		}

		parsee = parsee.substr(8, std::wstring::npos);
		tagOffset += 8;

		prop.uiColor.dwCurrentColor = color;
	}
	else if (command.compare(L"r") == 0)
	{
		prop.uiColor.dwCurrentColor = prop.uiColor.dwColor[UI_STATE_NORMAL];
	}
	else if (command.compare(L"i") == 0 || command.compare(L"k") == 0)
	{
		std::wstring::size_type tagContentsSize = parsee.find_first_of(GetNameLinkTagCloser());
		if (tagContentsSize != std::wstring::npos)
		{
			parsee = parsee.substr(tagContentsSize + 1, std::wstring::npos);
			tagOffset += int(tagContentsSize) + 1; // + closer size
		}
		else
		{
			tagOffset += int(parsee.length());
			return false;
		}
	}

	return true;
}

// todo : Refactoring by extract method with ParseText()
bool CEtUINameLinkMgr::GetTagOffsetByParseText(const std::wstring& parsee, int& tagOffset) const
{
	if (parsee.empty())
		return false;

	std::wstring command;
	command = parsee.substr(0, GetCmdLength());

	std::wstring args;
	args = parsee.substr(GetCmdLength(), std::wstring::npos);
	tagOffset++;

	//	todo : Refactor to command pattern if it grows beyond one screen by kalliste
	if (command.compare(L"c") == 0)
	{
		if (args.length() < 8)
		{
			tagOffset += int(args.length());
			return false;
		}
		tagOffset += 8;
	}
	else if (command.compare(L"i") == 0 || command.compare(L"k") == 0)
	{
		std::wstring::size_type tagContentsSize = args.find_first_of(GetNameLinkTagCloser());
		tagOffset += int(tagContentsSize) + 1; // + closer size
	}

	return true;
}

// todo : refactoring using command pattern. by kalliste
int CEtUINameLinkMgr::TranslateText(std::wstring& translated, const WCHAR* originalStr, CEtUINameLinkInterface* pInterface) const
{
	if (originalStr == NULL || originalStr[0] == '\0')
	{
		_ASSERT(0);
		return 0;
	}

	translated.clear();

	std::wstring rest;
	const std::wstring& trs_org = originalStr;
	std::wstring::size_type linkStartPos = 0;
	int posWithoutTag, lengthWithoutTag, fullTextCaretIdx, i;
		posWithoutTag		= 
		lengthWithoutTag	= 
		i					= 
		fullTextCaretIdx	= 0;

	WCHAR sep = GetNameLinkSeperator();
	rest = originalStr;
	while(rest.size() > 0 || linkStartPos != std::wstring::npos)
	{
		linkStartPos = rest.find_first_of(sep, linkStartPos);
		if (linkStartPos == std::wstring::npos)
		{
			lengthWithoutTag = posWithoutTag + int(rest.length());
			PasteTextPiece(translated, rest);
			break;
		}
		else
		{
			std::wstring drawText = rest.substr(0, linkStartPos);
			PasteTextPiece(translated, drawText);

			rest = rest.substr(linkStartPos + 1, std::wstring::npos);
			posWithoutTag += int(drawText.length());
			lengthWithoutTag += int(drawText.length());

			int tagOffset = 1;
			if (ParseText(rest, tagOffset, pInterface) == false)
			{
				lengthWithoutTag = posWithoutTag + int(rest.length());
				PasteTextPiece(translated, rest);
				break;
			}
			else
			{
				fullTextCaretIdx += (tagOffset + 1);
			}

			linkStartPos = 0;
		}
	}

	return lengthWithoutTag;
}

int	CEtUINameLinkMgr::TranslateText(std::vector<CWord>& translatedWords, const SWORD_PROPERTY& basicProp, const WCHAR* originalStr) const
{
	if (originalStr == NULL || originalStr[0] == '\0')
	{
		return 0;
	}

	std::wstring rest;
	const std::wstring& trs_org = originalStr;
	std::wstring::size_type linkStartPos = 0;
	int posWithoutTag, lengthWithoutTag, fullTextCaretIdx, i;
	posWithoutTag		= 
		lengthWithoutTag	= 
		i					= 
		fullTextCaretIdx	= 0;

	CWord word;
	word.m_sProperty = basicProp;

	// todo by kalliste : start/end tag splitting

	WCHAR sep = GetNameLinkSeperator();
	rest = originalStr;
	bool bParsing = false;
	std::wstring tagString, lastTag, startTag;
	MakeTag(lastTag, eNLTAG_END);
	MakeTag(startTag, eNLTAG_START);

	while(rest.size() > 0 || linkStartPos != std::wstring::npos)
	{
		linkStartPos = rest.find_first_of(sep, linkStartPos);
		if (linkStartPos == std::wstring::npos)
		{
			lengthWithoutTag = posWithoutTag + int(rest.length());
			if (rest.empty() == false)
			{
				word.m_strWordWithTag.clear();
				word.m_strWord.clear();
				PasteTextPiece(word.m_strWord, rest);
				translatedWords.push_back(word);
			}
			break;
		}
		else
		{
			std::wstring drawText = rest.substr(0, linkStartPos);
			if (drawText.empty() == false)
			{
				word.m_strWordWithTag.clear();
				word.m_strWord.clear();
				PasteTextPiece(word.m_strWord, drawText);
				if (bParsing)
				{
					PasteTextPiece(word.m_strWordWithTag, tagString);
					bParsing = false;
				}
				translatedWords.push_back(word);
			}

			if (bParsing == false && rest.compare(linkStartPos, 2, startTag) == 0)
			{
				std::wstring::size_type linkEndPos = rest.find(lastTag, linkStartPos);
				if (linkEndPos != std::wstring::npos)
				{
					tagString = rest.substr(linkStartPos, (linkEndPos - linkStartPos + lastTag.length()));
					bParsing = true;
				}
			}

			rest = rest.substr(linkStartPos + 1, std::wstring::npos);
			posWithoutTag += int(drawText.length());
			lengthWithoutTag += int(drawText.length());

			int tagOffset = 1;
			if (ParseText(word.m_sProperty, rest, tagOffset) == false)
			{
				lengthWithoutTag = posWithoutTag + int(rest.length());
				if (rest.empty() == false)
				{
					word.m_strWordWithTag.clear();
					word.m_strWord.clear();
					PasteTextPiece(word.m_strWord, rest);
					translatedWords.push_back(word);
				}
				break;
			}
			else
			{
				fullTextCaretIdx += (tagOffset + 1);
			}

			linkStartPos = 0;
		}
	}

	return lengthWithoutTag;
}

void CEtUINameLinkMgr::PasteTextPiece(std::wstring& buffer, const std::wstring& piece) const
{
	if (buffer.empty())
		buffer = piece;
	else
		buffer.append(piece);
}

WCHAR CEtUINameLinkMgr::GetNameLinkSeperator() const
{
	return 0xff00;
}

WCHAR CEtUINameLinkMgr::GetNameLinkTagCloser() const
{
	return ';';
}

bool CEtUINameLinkMgr::MakeTag(std::wstring& result, eNameLinkTagType type) const
{
	switch(type)
	{
	case eNLTAG_COLOR:			result = FormatW(L"%cc", GetNameLinkSeperator()); break;
	case eNLTAG_END:			result = FormatW(L"%ce", GetNameLinkSeperator()); break;
	case eNLTAG_ITEMINFO:		result = FormatW(L"%ci", GetNameLinkSeperator()); break;
	case eNLTAG_SKILL:			result = FormatW(L"%ck", GetNameLinkSeperator()); break;
	case eNLTAG_USERNAME:		result = FormatW(L"%cn", GetNameLinkSeperator()); break;
	case eNLTAG_RESTORECOLOR:	result = FormatW(L"%cr", GetNameLinkSeperator()); break;
	case eNLTAG_START:			result = FormatW(L"%cs", GetNameLinkSeperator()); break;
	default:
		{
			_ASSERT(0);
			return false;
		}
	}

	return true;
}

// todo by kalliste : Auto-generate tags by accepting arguments matching each type
bool CEtUINameLinkMgr::AppendTag(std::wstring& result, eNameLinkTagType type) const
{
	std::wstring appendee;
	if (MakeTag(appendee, type) == false)
		return false;

	result += appendee;
	return true;
}

bool CEtUINameLinkMgr::IsTagType(const std::wstring& text, int pos, eNameLinkTagType type) const
{
	if ((pos + 1) >= (int)text.size()|| pos < 0)
		return false;

	if (text[pos] == GetNameLinkSeperator())
	{
		std::wstring command = text.substr(pos, GetCmdLength() + 1);

		std::wstring tagStr;
		MakeTag(tagStr, type);
		if (command.compare(tagStr) == 0)
			return true;
	}

	return false;
}

CEtUINameLinkMgr::eNameLinkTagType CEtUINameLinkMgr::IsTagType(const std::wstring& text, int pos) const
{
	if (pos >= (int)text.size() || pos < 0)
		return eNLTAG_NONE;

	if (text[pos] == GetNameLinkSeperator())
	{
		std::wstring parsee = text.substr(pos + 1, std::wstring::npos);
		std::wstring command;
		command = parsee[0];

		if (command.compare(L"s") == 0)
			return eNLTAG_START;
		else if (command.compare(L"e") == 0)
			return eNLTAG_END;
		else if (command.compare(L"c") == 0)
			return eNLTAG_COLOR;
		else if (command.compare(L"r") == 0)
			return eNLTAG_RESTORECOLOR;
		else if (command.compare(L"i") == 0)
			return eNLTAG_ITEMINFO;
		else if (command.compare(L"n") == 0)
			return eNLTAG_USERNAME;
		else if (command.compare(L"k") == 0)
			return eNLTAG_SKILL;
	}

	return eNLTAG_NONE;
}