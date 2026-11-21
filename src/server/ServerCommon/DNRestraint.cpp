
#include "stdafx.h"
#include "DNRestraint.h"
#include "DNUserBase.h"

#include "TimeSet.h"

CDNRestraint::CDNRestraint(CDNUserBase * pSession)
{
	m_pSession = pSession;
	memset(&m_RestraintData, 0, sizeof(TRestraintData));
	m_pRestraintChat = NULL;
	m_pRestraintTrade = NULL;
}

CDNRestraint::~CDNRestraint()
{
	m_pSession = NULL;
	Initialize();
}

void CDNRestraint::Initialize()
{
	memset(&m_RestraintData, 0, sizeof(TRestraintData));
	m_pRestraintChat = NULL;
	m_pRestraintTrade = NULL;
}

bool CDNRestraint::LoadRestraint(TRestraintData * pData)
{
	Initialize();
	m_RestraintData = *pData;

	time_t tt;
	time(&tt);

	//build
	for (int i = 0; i < RESTRAINTMAX; i++)
	{
		if (tt > m_RestraintData.Restraint[i]._tEndTime) continue;		//restraint has expired
		switch (m_RestraintData.Restraint[i].nRestraintType)
		{
		case _RESTRAINTTYPE_BLOCK: return false;		//cause user blocked
		case _RESTRAINTTYPE_TRADE: m_pRestraintTrade = &m_RestraintData.Restraint[i]; break;
		case _RESTRAINTTYPE_CHAT: m_pRestraintChat = &m_RestraintData.Restraint[i]; break;
		default: 
			g_Log.Log(LogType::_ERROR, L"Unknown RestraintType %d\n", m_RestraintData.Restraint[i].nRestraintType);
		}
	}
	
	return true;
}

bool CDNRestraint::DelRestraint(UINT nRestraintKey)
{
	for (int i = 0; i < RESTRAINTMAX; i++)
	{
		if (m_RestraintData.Restraint[i].nRestraintKey == nRestraintKey)
		{		
			switch (m_RestraintData.Restraint[i].nRestraintKey)
			{
			case _RESTRAINTTYPE_TRADE: 
				{

					m_pRestraintTrade = NULL;
					memset(&m_RestraintData.Restraint[i], 0, sizeof(TRestraint));
				}
				break;

			case _RESTRAINTTYPE_CHAT: 
				{
					m_pRestraintChat = NULL;
					memset(&m_RestraintData.Restraint[i], 0, sizeof(TRestraint));
				}
				break;
			}
			m_pSession->SendRestraintDel(nRestraintKey);
			return true;
		}
	}
	return false;
}

bool CDNRestraint::CheckRestraint(int nType, bool bSend)
{
	time_t tt;
	time(&tt);

	switch (nType)
	{
		case _RESTRAINTTYPE_TRADE:
		{
			if (m_pRestraintTrade)
			{
				if (m_pRestraintTrade->_tBegineTime < tt && m_pRestraintTrade->_tEndTime > tt)
				{
					if (bSend) 
						m_pSession->SendRestraint(m_pRestraintTrade);
					return false;
				}
			}
		}
		break;

		case _RESTRAINTTYPE_CHAT:
		{
			if (m_pRestraintChat)
			{
				if (m_pRestraintChat->_tBegineTime < tt && m_pRestraintChat->_tEndTime > tt)
				{
					if (bSend) 
						m_pSession->SendRestraint(m_pRestraintChat);
					return false;
				}
			}
		}
		break;
	};
	return true;
}

void CDNRestraint::SendRestraintList()
{
	m_pSession->SendRestrinatList(&m_RestraintData);
}
