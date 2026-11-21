
#pragma once

#include "CriticalSection.h"

class CDNUserDeleteManager
{

public:
	typedef struct USERDELDAT
	{
	public:
		USERDELDAT(UINT pAccountDBID)
		{
			DN_ASSERT(0 != pAccountDBID,	"Invalid!");
			m_AccountDBID = pAccountDBID;
			SetTick();
		}
		USERDELDAT(const USERDELDAT& pRv)
		{
			DN_ASSERT(pRv.IsSet(),	"Invalid!");
			(*this) = pRv;
		}
		bool IsSet() const
		{ 
			return(0 != m_AccountDBID);
		}
		void SetTick()
		{
			m_OldTick = ::GetTickCount();
		}
	public:
		UINT m_AccountDBID;
		DWORD m_OldTick;
	} * LPUSERDELDAT;

public:
	typedef	std::map<UINT, USERDELDAT>		TP_LIST;
	typedef	TP_LIST::iterator				TP_LIST_ITR;
	typedef	TP_LIST::const_iterator			TP_LIST_CTR;
	typedef	std::vector<USERDELDAT>			TP_LISTAUTO;
	typedef	TP_LISTAUTO::iterator			TP_LISTAUTO_ITR;
	typedef	TP_LISTAUTO::const_iterator		TP_LISTAUTO_CTR;
	typedef	CSyncLock						TP_LOCK;
	typedef	ScopeLock<CSyncLock>			TP_LOCKAUTO;

public:
	CDNUserDeleteManager();

	bool Add(UINT pAccountDBID, bool bDoLock);
	void Remove(UINT pAccountDBID, bool bDoLock);

	bool IsExist(UINT pAccountDBID, bool bDoLock) const;

	void DoUpdate();

private:
	void GetList(TP_LISTAUTO& pList, bool bDoLock) const;

private:
	TP_LIST m_List;
	mutable TP_LOCK m_Lock;

};

extern CDNUserDeleteManager* g_pUserDeleteManager;
