#include "stdafx.h"
#include "EtActionBase.h"
#include "EtActionCoreMng.h"
#include "EtActionSignal.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CEtActionBase::CEtActionBase()
{
	m_pVecActionElementList = NULL;
	m_pMapSearch = NULL;

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	m_pCheckInitSignalFunc = CEtActionBase::pCheckInitSignal;
#endif
	m_pCheckPreSignalFunc = CEtActionBase::pCheckPreSignal;
	m_pCheckPostSignalFunc = CEtActionBase::pCheckPostSignal;
}

CEtActionBase::~CEtActionBase()
{
	FreeAction();
}

bool CEtActionBase::LoadAction( const char *szFullPathName )
{
	if( m_pVecActionElementList ) return true;
	m_pVecActionElementList = CEtActionCoreMng::GetInstance().LoadAction( szFullPathName, this );
	if( !m_pVecActionElementList ) return false;
	m_szFileName = szFullPathName;

	return true;
}


void CEtActionBase::FreeAction()
{
	if( m_pVecActionElementList )
		CEtActionCoreMng::GetInstance().RemoveAction( m_szFileName.c_str(), this );
	m_pVecActionElementList = NULL;
	m_pMapSearch = NULL;
}

DWORD CEtActionBase::GetElementCount()
{
	if( !m_pVecActionElementList ) return 0;
	return (DWORD)m_pVecActionElementList->size();
}

CEtActionBase::ActionElementStruct *CEtActionBase::GetElement( DWORD dwIndex )
{
	if( !m_pVecActionElementList ) return NULL;
	if( dwIndex < 0 || dwIndex >= m_pVecActionElementList->size() ) return NULL;
	return (*m_pVecActionElementList)[dwIndex];
}

CEtActionBase::ActionElementStruct *CEtActionBase::GetElement( const char *szName )
{
	if( !m_pVecActionElementList ) return NULL;
	std::map<std::string, int>::iterator it = m_pMapSearch->find( szName );
	if( it == m_pMapSearch->end() ) return NULL;
	return (*m_pVecActionElementList)[it->second];

	return NULL;
}

int CEtActionBase::GetElementIndex( const char *szName )
{
	if( !m_pVecActionElementList ) return -1;

	std::map<std::string, int>::iterator it = m_pMapSearch->find( szName );
	if( it == m_pMapSearch->end() ) return -1;
	return it->second;
}

bool CEtActionBase::IsExistAction( const char *szAction )
{
	if( GetElementIndex( szAction ) == -1 ) return false;
	return true;
}

DWORD CEtActionBase::GetSignalCount( DWORD dwElementIndex, int nSignalIndex )
{
	if( !m_pVecActionElementList ) return 0;
	if( dwElementIndex < 0 || dwElementIndex >= m_pVecActionElementList->size() ) return 0;
	std::map<int, std::vector<CEtActionSignal *>>::iterator it = (*m_pVecActionElementList)[dwElementIndex]->MapSearch.find( nSignalIndex );
	if( it == (*m_pVecActionElementList)[dwElementIndex]->MapSearch.end() ) return 0;

	return (DWORD)it->second.size();
}

CEtActionSignal *CEtActionBase::GetSignal( DWORD dwElementIndex, int nSignalIndex, DWORD dwIndex )
{
	if( !m_pVecActionElementList ) return NULL;
	if( dwElementIndex < 0 || dwElementIndex >= m_pVecActionElementList->size() ) return NULL;
	std::map<int, std::vector<CEtActionSignal *>>::iterator it = (*m_pVecActionElementList)[dwElementIndex]->MapSearch.find( nSignalIndex );
	if( it == (*m_pVecActionElementList)[dwElementIndex]->MapSearch.end() ) return NULL;
	if( dwIndex < 0 || dwIndex >= it->second.size() ) return NULL;

	return it->second[dwIndex];
}

CEtActionSignal *CEtActionBase::GetSignal( DWORD dwElementIndex, int nSignalArrayIndex )
{
	if( !m_pVecActionElementList ) return NULL;
	if( dwElementIndex < 0 || dwElementIndex >= m_pVecActionElementList->size() ) return NULL;
	std::map<int, CEtActionSignal *>::iterator it = (*m_pVecActionElementList)[dwElementIndex]->MapSearchByArrayIndex.find( nSignalArrayIndex );
	if( it == (*m_pVecActionElementList)[dwElementIndex]->MapSearchByArrayIndex.end() ) return NULL;

	return it->second;
}

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
void CEtActionBase::CheckInitSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	if( m_pCheckInitSignalFunc ) m_pCheckInitSignalFunc( pElement, nElementIndex, pSignal, nSignalIndex, pActionBase );
}
#endif

void CEtActionBase::CheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	if( m_pCheckPreSignalFunc ) m_pCheckPreSignalFunc( pElement, nElementIndex, pSignal, nSignalIndex, pActionBase );
}

void CEtActionBase::CheckPostSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	if( m_pCheckPostSignalFunc ) m_pCheckPostSignalFunc( pElement, nElementIndex, pSignal, nSignalIndex, pActionBase );
}

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
void CEtActionBase::SetUsingElement( std::vector<std::string> &szVecList )
{
	m_szVecUsingElementList = szVecList;
}

void CEtActionBase::AddUsingElement( const char *szName )
{
	if( !m_pVecActionElementList ) {
		m_szVecUsingElementList.push_back( szName );
		return;
	}

	CEtActionCoreMng::ActionContainerStruct *pContainer = CEtActionCoreMng::GetInstance().GetContainer( m_szFileName.c_str() );
	if( !pContainer ) return;

	pContainer->szVecUsingElementList.push_back( szName );
}

void CEtActionBase::ClearUsingElement()
{
	if( !m_pVecActionElementList ) {
		m_szVecUsingElementList.clear();
		return;
	}

	CEtActionCoreMng::ActionContainerStruct *pContainer = CEtActionCoreMng::GetInstance().GetContainer( m_szFileName.c_str() );
	if( !pContainer ) return;

	pContainer->szVecUsingElementList.clear();
}

void CEtActionBase::RefreshUsingElement()
{
	if( !m_pVecActionElementList ) return;

	CEtActionCoreMng::ActionContainerStruct *pContainer = CEtActionCoreMng::GetInstance().GetContainer( m_szFileName.c_str() );
	if( !pContainer ) return;

	if( !m_szVecUsingElementList.empty() ) {
		pContainer->szVecUsingElementList = m_szVecUsingElementList;
		m_szVecUsingElementList.clear();
	}
	if( pContainer->szVecUsingElementList.empty() ) return;

	for( DWORD i=0; i<m_pVecActionElementList->size(); i++ ) {
		ActionElementStruct *pElement = (*m_pVecActionElementList)[i];

		if( std::find( pContainer->szVecUsingElementList.begin(), pContainer->szVecUsingElementList.end(), pElement->szParentFileName ) == pContainer->szVecUsingElementList.end() ) {
			if( pElement->bUsing ) {
				for( DWORD j=0; j<pElement->pVecSignalList.size(); j++ ) {
					CheckPostSignal( pElement, j, pElement->pVecSignalList[j], j, this );
				}
				pElement->bUsing = false;
			}
		}
		else {
			if( !pElement->bUsing ) {
				for( DWORD j=0; j<pElement->pVecSignalList.size(); j++ ) {
					CheckPreSignal( pElement, j, pElement->pVecSignalList[j], j, this );
				}
				pElement->bUsing = true;
			}
		}
	}
}

void CEtActionBase::RefreshUsingElement( const char *szElementName, bool bUse )
{
	if( !m_pVecActionElementList ) return;

	CEtActionCoreMng::ActionContainerStruct *pContainer = CEtActionCoreMng::GetInstance().GetContainer( m_szFileName.c_str() );
	if( !pContainer ) return;

	if( pContainer->szVecUsingElementList.empty() ) return;

	ActionElementStruct *pElement = GetElement( szElementName );
	if( !pElement ) return;
	if( bUse ) {
		if( !pElement->bUsing ) {
			for( DWORD j=0; j<pElement->pVecSignalList.size(); j++ ) {
				CheckPreSignal( pElement, j, pElement->pVecSignalList[j], j, this );
			}
			pElement->bUsing = true;
		}
	}
	else {
		if( pElement->bUsing ) {
			for( DWORD j=0; j<pElement->pVecSignalList.size(); j++ ) {
				CheckPostSignal( pElement, j, pElement->pVecSignalList[j], j, this );
			}
			pElement->bUsing = false;
		}
	}
}

#endif