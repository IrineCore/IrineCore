#include "StdAfx.h"
#include "EtWater.h"
#include "DnWorldProp.h"
#include "EtMatrixEx.h"
#include "DnWorldSector.h"
#include "DnTableDB.h"
#include "DNProtocol.h"
#include "EtOptionController.h"
#include "PropHeader.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

DECL_SMART_PTR_STATIC( CDnWorldProp, 1000 )

CDnWorldProp::CDnWorldProp()
{
	EtMatrixIdentity( &m_matWorld );

	m_IntersectionType = Alpha;
	m_PropType = PTE_Static;
	m_fVisibleRange = 0.f;
	m_bIsStaticCollision = true;
	m_nSpecLevel = 0;
	m_bShow = true;
	m_bProjectileSkip = false;
	m_bCastLightmap = true;
	m_bLightmapInfluence = false;
	m_bCastWater = true;

#ifdef PRE_FIX_PROP_RANDOM_VISIBLE
	m_bFirstProcess = true;
	#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
	#else
	m_bRandomResultIsVisible = true;
	#endif
	m_iVisiblePercent = 100;
#endif // #ifdef PRE_FIX_PROP_RANDOM_VISIBLE
#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
	ClearVisibleOptions();
#endif
	m_bProcess = false; 
}

CDnWorldProp::~CDnWorldProp()
{
	CDnWorld::GetInstance().CheckAndRemoveVisibleProp( this );
}

bool CDnWorldProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
	ClearVisibleOptions();
#endif

	bool bResult = CEtWorldProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	if( bResult == false ) return false;

	int nTableID = ((CDnWorldSector*)pParentSector)->GetPropTableID( szPropName );
	if( nTableID != -1 ) {
		if( InitializeTable( nTableID ) == false ) return false;
	}
	if( CreateObject() == false ) return false;

	if( m_Handle ) {
		m_Handle->SetCollisionScale( max( max( vScale.x, vScale.y ), vScale.z ) );
		if( m_bIsStaticCollision ) m_Handle->SetCollisionGroup( COLLISION_GROUP_STATIC( 1 ) );
		else m_Handle->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 2 ) );
		m_Handle->SetTargetCollisionGroup( 0 );
		m_Handle->EnableShadowReceive( m_bReceiveShadow );
		m_Handle->EnableShadowCast( m_bCastShadow );
		m_Handle->EnableLightMapCast( m_bCastLightmap );
		if( GetEtOptionController()->GetWaterQuality() == WQ_HIGH )
		{
			m_Handle->EnableWaterCast( m_bCastWater );
		}
		else
		{
			m_Handle->EnableWaterCast( false );
		}
		m_Handle->EnableLightMapInfluence( m_bLightmapInfluence );
	}
	UpdateMatrix();

	return true;
}

bool CDnWorldProp::InitializeTable( int nTableID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPROP );

	m_PropType = (PropTypeEnum)pSox->GetFieldFromLablePtr( nTableID, "_ClassID" )->GetInteger();

	// Check probability to be applied and return
#ifdef PRE_FIX_PROP_RANDOM_VISIBLE
	// Create all probability-spawned props first, then query the server to see if any should be hidden.
	m_iVisiblePercent = (int)( pSox->GetFieldFromLablePtr( nTableID, "_VisibleProb" )->GetFloat() * 100.f );
#else
	int nValue = (int)( pSox->GetFieldFromLablePtr( nTableID, "_VisibleProb" )->GetFloat() * 100.f );
	if( _rand()%100 > nValue ) return false;
#endif // #ifdef PRE_FIX_PROP_RANDOM_VISIBLE

	// Check weather and return
	std::string szWeather;
	szWeather = pSox->GetFieldFromLablePtr( nTableID, "_VisibleWeather" )->GetString();
	char *szWeatherString[CDnWorld::WeatherEnum_Amount] = { "FineDay", "FineGlow", "FineNight", "FineDawn", "CloudyDay", "CloudyGlow", "CloudyNight", "CloudyDawn", "RainDay", "RainNight", "HeavyrainDay", "HeavyrainNight" };
	bool bVisible = false;
	if( strcmp( szWeather.c_str(), "All" ) == NULL ) bVisible = true;
	else { // Check weather if not All
		CDnWorld::WeatherEnum Weather = CDnWorld::GetInstance().GetCurrentWeather();
		for( int i=0; ; i++ ) {
			//char *szTemp = _GetSubStrByCount( i, (char*)szWeather.c_str(), '+' );
			std::string strValue = _GetSubStrByCountSafe( i, (char*)szWeather.c_str(), '+' );
			if( strValue.size() == 0 ) break;
			if( strcmp( strValue.c_str(), szWeatherString[Weather] ) == NULL ) {
				bVisible = true;
				break;
			}
		}
	}
	if( bVisible == false ) return false;

	m_IntersectionType = (IntersectionEnum)pSox->GetFieldFromLablePtr( nTableID, "_IntersectionType" )->GetInteger();
	m_fVisibleRange = pSox->GetFieldFromLablePtr( nTableID, "_CullingRange" )->GetInteger() * 100.f;
	if( m_Handle && m_fVisibleRange > 0.f ) {
		// Set Visible Range later once m_Handle supports it.
	}
	m_bIsStaticCollision = ( pSox->GetFieldFromLablePtr( nTableID, "_IsStaticCollision" )->GetInteger() == TRUE ) ? true : false;

	EnableCastLightmap( ( pSox->GetFieldFromLablePtr( nTableID, "_IsCastLightmap" )->GetInteger() == TRUE ) ? true : false );
	EnableCastWater( ( pSox->GetFieldFromLablePtr( nTableID, "_IsCastWater" )->GetInteger() == TRUE ) ? true : false );
	EnableLightmapInfluence( ( pSox->GetFieldFromLablePtr( nTableID, "_IsLightmapInfluence" )->GetInteger() == TRUE ) ? true : false );

	m_nSpecLevel		= pSox->GetFieldFromLablePtr( nTableID, "_SpecLevel" )->GetInteger();
	m_bProjectileSkip	= ( pSox->GetFieldFromLablePtr( nTableID, "_IsProjectileSkip" )->GetInteger() == TRUE ) ? true : false;

	return true;
}

bool CDnWorldProp::CreateObject()
{
	m_Handle = EternityEngine::CreateStaticObject( GetPropName() );
	if ( !m_Handle ) return false;
	if( m_IntersectionType == Alpha ) {
		m_Handle->GetSkinInstance()->SetRenderAlphaTwoPass( true );
	}
	return true;
}

void CDnWorldProp::FreeObject()
{
	SAFE_RELEASE_SPTR( m_Handle );
}

void CDnWorldProp::UpdateMatrix()
{
	EtVector3 vOffset = *m_pParentSector->GetOffset();
	vOffset.x -= ( m_pParentSector->GetTileWidthCount() * m_pParentSector->GetTileSize() ) / 2.f;
	vOffset.z -= ( m_pParentSector->GetTileHeightCount() * m_pParentSector->GetTileSize() ) / 2.f;
	vOffset.y = 0.f;

	m_matExWorld.m_vPosition = m_vPosition + vOffset;
	m_matExWorld.RotatePitch( m_vRotation.x );
	m_matExWorld.RotateRoll( m_vRotation.z );
	m_matExWorld.RotateYaw( m_vRotation.y );

	EtMatrix matScale;
	m_matWorld = *m_matExWorld;
	EtMatrixScaling( &matScale, m_vScale.x, m_vScale.y, m_vScale.z );
	EtMatrixMultiply( &m_matWorld, &matScale, &m_matWorld );

	if( m_Handle ) m_Handle->Update( &m_matWorld );
}

void CDnWorldProp::EnableCastShadow( bool bEnable )
{
	CEtWorldProp::EnableCastShadow( bEnable );

	if( m_Handle ) {
		m_Handle->EnableShadowCast( bEnable );
		/*
		if( !IsCastShadow() && !IsReceiveShadow() ) {
			m_Handle->EnableLightMapCast( true );
			m_Handle->EnableLightMapInfluence( false );
		}
		else {
			m_Handle->EnableLightMapCast( false );
			m_Handle->EnableLightMapInfluence( true );
		}
		*/
	}
}

void CDnWorldProp::EnableReceiveShadow( bool bEnable )
{
	CEtWorldProp::EnableReceiveShadow( bEnable );

	if( m_Handle ) {
		m_Handle->EnableShadowReceive( bEnable );
		/*
		if( !IsCastShadow() && !IsReceiveShadow() ) {
			m_Handle->EnableLightMapCast( true );
			m_Handle->EnableLightMapInfluence( false );
		}
		else {
			m_Handle->EnableLightMapCast( false );
			m_Handle->EnableLightMapInfluence( true );
		}
		*/
	}
}

void CDnWorldProp::EnableCastLightmap( bool bEnable )
{
	CEtWorldProp::EnableCastLightmap( bEnable );
	if( m_Handle ) m_Handle->EnableLightMapCast( bEnable );
}

void CDnWorldProp::EnableLightmapInfluence( bool bEnable )
{
	CEtWorldProp::EnableLightmapInfluence( bEnable );
	if( m_Handle ) m_Handle->EnableLightMapInfluence( bEnable );
}

DnPropHandle CDnWorldProp::FindPropFromUniqueID( DWORD dwUniqueID )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	DnPropHandle hProp;
	int nCount = GetItemCount();
	for( int i=0; i<nCount; i++ ) {
		hProp = GetItem(i);
		if( hProp ) {
			if( hProp->GetUniqueID() == dwUniqueID ) return hProp;
		}
	}
	return CDnWorldProp::Identity();
}

void CDnWorldProp::OnDispatchMessage( DWORD dwProtocol, BYTE *pPacket )
{
	switch( dwProtocol ) {
		case eProp::SC_CMDSHOW:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				bool bShow;

				Stream.Read( &bShow, sizeof(bool) );
				Show( bShow );
			}
			break;
		case eProp::SC_CMDCHATBALLOON_AS_INDEX:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				int nUIStringIndex;
				Stream.Read( &nUIStringIndex, sizeof(int) );

				if( nUIStringIndex == 0 )
				{
					StopRenderChatBalloon();
					break;
				}

				std::wstring wszChat = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nUIStringIndex );
				if( !wszChat.empty() )
				{
					SetChatBalloonText( wszChat.c_str(), GetTickCount(), 1 );
				}
			}
			break;

#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
		case eProp::SC_SEND_SHOWINFO:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				const DWORD dwVisibleOptionSize = (sizeof(PropDef::Option::eValue) * PropDef::Option::eMaxVisibleOption);

				Stream.Read(m_VisibleOptions, dwVisibleOptionSize);
			}
			break;
#endif
	}
}

void CDnWorldProp::CalcCustomRenderDepth()
{
	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if( hCamera ) {
		SAABox box;
		GetBoundingBox(box);
		float fHeight = box.Max.y - box.Min.y;
		EtVector3 vPos = m_matExWorld.m_vPosition;
		vPos.y += fHeight;
		m_fCustomRenderDepth = EtVec3Dot(&vPos, &hCamera->GetMatEx()->m_vZAxis);
	}
}

void CDnWorldProp::RenderCustom( float fElapsedTime )
{
	if( IsShow() && IsRenderChatBalloon() ) {
		CalcCustomRenderDepth();

		SAABox box;
		GetBoundingBox(box);
		float fHeight = box.Max.y - box.Min.y;

		EtVector3 vPos = m_matExWorld.m_vPosition;
		vPos.y += fHeight;
		RenderChatBalloon( vPos, vPos, fElapsedTime );
	}
}

//void CDnWorldProp::CmdOperation()
//{
//	Send( eProp::CS_CMDOPERATION, NULL );
//}

void CDnWorldProp::Show( bool bShow )
{
	if( m_bShow == bShow ) return;
	m_bShow = bShow;
	if( !m_Handle ) return;
	m_Handle->ShowObject( bShow );
	EnableCollision( bShow );
}


void CDnWorldProp::EnableCollision( bool bEnable )
{
	if( IsIgnoreBuildColMesh() ) return;
	if( m_Handle ) m_Handle->EnableCollision( bEnable );
}

void CDnWorldProp::EnableIgnoreBuildColMesh( bool bEnable )
{
	CEtWorldProp::EnableIgnoreBuildColMesh( bEnable );
	if( m_Handle ) m_Handle->EnableCollision( !bEnable );
}

void* CDnWorldProp::AllocPropData(int& usingCount)
{
#ifdef WIN64
	usingCount = GetPropSignalDataUsingCount64(GetClassID());
#else
	usingCount = GetPropSignalDataUsingCount(GetClassID());
#endif
	if (usingCount > 0)
		return new int[usingCount];
	return NULL;
}

#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
void CDnWorldProp::ClearVisibleOptions()
{
	int i = 0;
	for (; i < PropDef::Option::eMaxVisibleOption; ++i)
	{
		m_VisibleOptions[i] = PropDef::Option::NotSet;
	}
}

bool CDnWorldProp::IsVisible() const
{
	int i = PropDef::Option::eMaxVisibleOption - 1;
	for (; i >= PropDef::Option::eMinVisibleOption && i >= 0; --i)
	{
		if (m_VisibleOptions[i] != PropDef::Option::NotSet)
		{
			return (m_VisibleOptions[i] == PropDef::Option::True);
		}
	}

	return true;
}
#endif