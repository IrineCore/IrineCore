#include "stdafx.h"
#include "MASingleBody.h"

MASingleBody::MASingleBody()
{
}

MASingleBody::~MASingleBody()
{
	SAFE_RELEASE_SPTR( m_hObject );
}

void MASingleBody::LoadSkin( const char *szSkinName, const char *szAniName )
{
	m_szSkinFileName = szSkinName;
	m_szAniFileName = szAniName;

	if( m_hObject ) return;
	m_hObject = EternityEngine::CreateAniObject( STATICFUNC() szSkinName, szAniName );
}
