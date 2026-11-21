#include "StdAfx.h"

#include "DnMiniSiteChecker.h"
#include "DnCommonUtil.h"
#include "GameOption.h"
#include "DnInterface.h"
#include "DnMainDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef _WORK
#define MINISITE_CHECK_TIME 1.f	// todo by kalliste : set from global value
#else
#define MINISITE_CHECK_TIME 30.f	// todo by kalliste : set from global value
#endif

CDnMiniSiteChecker::CDnMiniSiteChecker()
{
	m_MiniSiteCheckTimer = GetMiniSiteCheckTime();
	m_HTMLReader.setEventHandler(this);
	m_pHtml = NULL;
}

CDnMiniSiteChecker::~CDnMiniSiteChecker()
{
	SAFE_DELETEA(m_pHtml);
}

float CDnMiniSiteChecker::GetMiniSiteCheckTime() const
{
	return MINISITE_CHECK_TIME * 60.f;
}

bool CDnMiniSiteChecker::Process(float fDelta)
{
	m_MiniSiteCheckTimer -= fDelta;

	if (m_MiniSiteCheckTimer < 0.f)
	{
		m_MiniSiteCheckTimer = GetMiniSiteCheckTime();
		SAFE_DELETEA(m_pHtml);

		std::string strURL;
#if defined(_TEST_CODE_KAL)
		strURL = "d:\\minisite_test.htm";
#endif
		if (CommonUtil::GetHtmlFromURL(strURL.c_str(), &m_pHtml) == false)
			return false;

		m_HTMLReader.Read(m_pHtml);
	}

	return true;
}

void CDnMiniSiteChecker::Characters(const CStringW &rText, DWORD dwAppData, bool &bAbort)
{
	if (!CGameOption::IsActive() || !CDnInterface::IsActive())
	{
		bAbort = true;
		return;
	}

	std::wstring textWrap(rText);

	std::wstring::size_type separator = textWrap.find_first_of(L"=");
	if (separator != std::wstring::npos)
	{
		std::wstring category = textWrap.substr(0, separator);
		std::wstring valueString = textWrap.substr(separator + 1);

		if (category.compare(L"MiniSiteVersion") == 0)
		{
			int value = _wtoi(valueString.c_str());
			if (value == 0)
			{
				CDnMainDlg* pMainDlg = GetInterface().GetMainBarDialog();
				if (pMainDlg)
					pMainDlg->ShowMiniSiteNotify(false, false);
				CGameOption::GetInstance().m_nMiniSiteVersion = value;
			}
			else
			{
				if (value != CGameOption::GetInstance().m_nMiniSiteVersion)
				{
					CDnMainDlg* pMainDlg = GetInterface().GetMainBarDialog();
					if (pMainDlg)
						pMainDlg->ShowMiniSiteNotify(true, true);

					CGameOption::GetInstance().m_nMiniSiteVersion = value;
				}
			}
		}
		else if (category.compare(L"MiniSiteURL") == 0)
		{
#define MAX_INTERNET_ADDRESS 512
			char szTemp[MAX_INTERNET_ADDRESS] = { 0, };
			WideCharToMultiByte( CP_ACP, 0, valueString.c_str(), -1, szTemp, MAX_INTERNET_ADDRESS, NULL, NULL );
			m_MiniSiteURL = szTemp;
		}
		else if (category.compare(L"MiniSiteTooltip") == 0)
		{
			m_MiniSiteButtonTooltip = valueString;
		}
	}
}

const std::wstring& CDnMiniSiteChecker::GetMiniSiteTooltip()
{
	if (m_MiniSiteButtonTooltip.empty())
	{
#ifdef _TEST_CODE_KAL
		if (CommonUtil::GetHtmlFromURL("d:\\minisite_test.htm", &m_pHtml))
#else
		if (CommonUtil::GetHtmlFromURL("http://dn.sdo.com/web8/minisite/ingame/inc.html", &m_pHtml))
#endif
			m_HTMLReader.Read(m_pHtml);
	}

	return m_MiniSiteButtonTooltip;
}