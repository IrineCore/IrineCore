#pragma once

#include "MemPool.h"

class CEtActionSignal;
class CEtActionBase {
public:
	CEtActionBase();
	virtual ~CEtActionBase();

	struct ActionElementStruct : public TBoostMemoryPool< ActionElementStruct >
	{
		std::string szName;
		std::string szLinkAniName;
		std::string szNextActionName;
		DWORD dwLength;
		DWORD dwBlendFrame;
		DWORD dwNextActionFrame;
		std::vector<CEtActionSignal *> pVecSignalList;
		std::map<int, std::vector<CEtActionSignal *>> MapSearch;
		std::map<int, CEtActionSignal *> MapSearchByArrayIndex;

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
		std::string szParentFileName;
		bool bUsing;
#endif

#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
		DWORD dwLastUsedTime;
		bool bCheckPostSignal;
#endif
	};
protected:
	std::string m_szFileName;
	std::vector<ActionElementStruct *> *m_pVecActionElementList;
	std::map<std::string, int> *m_pMapSearch;
	std::vector<std::string> m_szVecUsingElementList;

public:
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	void (__cdecl *m_pCheckInitSignalFunc)( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );
#endif
	void (__cdecl *m_pCheckPreSignalFunc)( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );
	void (__cdecl *m_pCheckPostSignalFunc)( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );

public:
	virtual bool LoadAction( const char *szFullPathName );
	virtual void FreeAction();

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	static void pCheckInitSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase ) {}
#endif
	static void pCheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase ) {}
	static void pCheckPostSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase ) {}

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	void CheckInitSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );
#endif
	void CheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );
	void CheckPostSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	void SetUsingElement( std::vector<std::string> &szVecList );
	void AddUsingElement( const char *szName );
	void ClearUsingElement();
	virtual void RefreshUsingElement();
	virtual void RefreshUsingElement( const char *szElementName, bool bUse = true );
#endif

	virtual bool IsIgnoreSignal( int nSignalIndex ) { return false; }

	DWORD GetElementCount();
	ActionElementStruct *GetElement( DWORD dwIndex );
	ActionElementStruct *GetElement( const char *szName );
	int GetElementIndex( const char *szName );
	virtual bool IsExistAction( const char *szAction );

	void SetSearchMap( std::map<std::string, int> *pMap ) { m_pMapSearch = pMap; }

	DWORD GetSignalCount( DWORD dwElementIndex, int nSignalIndex );
	CEtActionSignal *GetSignal( DWORD dwElementIndex, int nSignalIndex, DWORD dwIndex );
	CEtActionSignal *GetSignal( DWORD dwElementIndex, int nSignalArrayIndex );
	const char *GetActionFileName() { return m_szFileName.c_str(); }
	std::vector<std::string> &GetUsingElementList() { return m_szVecUsingElementList; }

};