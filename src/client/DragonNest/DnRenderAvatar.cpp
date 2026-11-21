#include "StdAfx.h"
#include "DnRenderAvatar.h"
#include "DnActorClassDefine.h"
#include "DnPlayerActor.h"
#include "DnItemTask.h"
#include "DnCharStatusDlg.h"
#include "DnTableDB.h"
#include "TaskManager.h"
#include "DnLoadingTask.h"
#include "DnInterface.h"
#include "DnPetActor.h"
#ifdef PRE_FIX_MEMOPT_EXT
#include "DnCommonUtil.h"
#endif


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

std::vector<CDnRenderAvatarNew::sTempTargetInfo> CDnRenderAvatarNew::s_vecTempTarget;
int CDnRenderAvatarNew::s_nTempTargetRefCount = 0;
CEtColorAdjustTexFilter *CDnRenderAvatarNew::s_pColorAdjFilter = NULL;
EtLightHandle CDnRenderAvatarNew::s_hTempLight[2];
EtTextureHandle CDnRenderAvatarNew::s_hTempEnviTexture;
EtColor CDnRenderAvatarNew::s_TempAmbient = 0xFFFFFFFF;
EtColor CDnRenderAvatarNew::s_TempLightDiffuse[2] = { 0xFFFFFFFF, 0xFFFFFFFF };
EtVector3 CDnRenderAvatarNew::s_TempLightDir[2] = { EtVector3( -0.58f, -0.58f, -0.58f ), EtVector3( 0.0f, -1.0f, 0.0f ) };
bool CDnRenderAvatarNew::s_bInitializeStaticObject = false;

CDnRenderAvatarNew::CDnRenderAvatarNew()
{
	m_bUseColorAdjFilter = false;
	m_bUseBorderAlpha = false;
	m_fDefaultCameraYPos = 56.0f;
	m_fDefaultCameraZPos = 150.0f;
	m_eActorType = ActorEnum::None;
	InitCameraControl();

	m_bUseTempLight = false;

	s_nTempTargetRefCount += 1;
	m_nDefaultViewPortWidth = 0;
	m_nDefaultViewPortHeight = 0;
	m_nDirLightCount = 0;
}

CDnRenderAvatarNew::~CDnRenderAvatarNew()
{
	ClearRenderObject();
	SAFE_RELEASE_SPTR( m_hActor );
	SAFE_RELEASE_SPTR( m_hEtc );

	// Mostly shared, so it is kept without releasing midway. If mid-way release is needed, change this approach.
	s_nTempTargetRefCount -= 1;
	if( s_nTempTargetRefCount == 0 )
	{
		for( int i = 0; i < (int)s_vecTempTarget.size(); ++i )
		{
			SAFE_RELEASE_SPTR( s_vecTempTarget[i].hTexture );
		}
		s_vecTempTarget.clear();
		FinalizeStaticObject();
	}
}

void CDnRenderAvatarNew::InitializeStaticObject()
{
	if( s_bInitializeStaticObject ) return;

	// Pre-create a temporary light object to use when the current world has no lights.
	CEtLight *pLight[2];
	SLightInfo sDefaultLightInfo;
	for( int i = 0; i < 2; ++i )
	{
		pLight[i] = new CEtLight();
		pLight[i]->SetLightInfo( &sDefaultLightInfo );
		s_hTempLight[i] = pLight[i]->GetMySmartPtr();
	}

	// Load the Env file provided by the designer.
	CEnvInfo EnvInfo;
	bool bResult = EnvInfo.Load( CEtResourceMng::GetInstance().GetFullName( "CashShop01.env" ).c_str() );
	if( bResult )
	{
		// Color Filter
		s_pColorAdjFilter = (CEtColorAdjustTexFilter*)EternityEngine::CreateFilter( SF_COLORADJUSTTEX );
		s_pColorAdjFilter->SetParam( *EnvInfo.GetColorAdjShadow(), *EnvInfo.GetColorAdjMidtones(), *EnvInfo.GetColorAdjHighlights(), EnvInfo.GetColorAdjSaturation() );
		s_pColorAdjFilter->Enable( false );

		// Environment Texture Setting
		if( EnvInfo.GetEnvironmentTexture() && strlen(EnvInfo.GetEnvironmentTexture()) > 0 )
			s_hTempEnviTexture = EternityEngine::LoadTexture( EnvInfo.GetEnvironmentTexture() );

		// Ambient
		EtColor Ambient = EnvInfo.GetAmbient();
		s_TempAmbient = EtColor( Ambient.b, Ambient.g, Ambient.r, 1.0f );

		// Light
		CEnvInfo::LightInfoStruct *pStruct;
		int nDirLightCount = EnvInfo.GetLightCount();
		nDirLightCount = min( nDirLightCount, 2 );
		for( int i = 0; i < nDirLightCount; ++i )
		{
			pStruct = EnvInfo.GetLight(i);
			pStruct->pInfo->Diffuse.r *= pStruct->pInfo->Diffuse.a;
			pStruct->pInfo->Diffuse.g *= pStruct->pInfo->Diffuse.a;
			pStruct->pInfo->Diffuse.b *= pStruct->pInfo->Diffuse.a;
			s_TempLightDiffuse[i] = pStruct->pInfo->Diffuse;
			s_TempLightDir[i] = pStruct->pInfo->Direction;
		}
		EnvInfo.Reset();
	}
	else
	{
		// Color Filter
		s_pColorAdjFilter = (CEtColorAdjustTexFilter*)EternityEngine::CreateFilter( SF_COLORADJUSTTEX );
		s_pColorAdjFilter->SetParam( EtVector4(-0.071f, -0.107f, -0.071f, 0.0f), EtVector4(0.0f, 0.0f, 0.0f, 0.0f), EtVector4(0.821f, 0.393f, 0.464f, 0.0f), 0.9f );
		s_pColorAdjFilter->Enable( false );

		// Ambient
		s_TempAmbient = EtColor( 0.568f, 0.752f, 0.850f, 1.0f );

		// Light
		for( int i = 0; i < 2; ++i )
		{
			s_TempLightDiffuse[i] = ( i == 0 ? EtColor( 1.164f, 1.116f, 0.828f, 1.2f ) : EtColor( 0.28f, 0.44f, 0.81f, 1.0f ) );
			s_TempLightDir[i] = ( i == 0 ? EtVector3( -0.143f, -0.806f, -0.573f ) : EtVector3( -0.607f, -0.716f, 0.343f ) );
		}
	}

	// Since it keeps failing to apply unless the environment map is set, force-check and set it here.
	if( !s_hTempEnviTexture )
		s_hTempEnviTexture = EternityEngine::LoadTexture( "Environment_Texture06.dds" );

	s_bInitializeStaticObject = true;
}

void CDnRenderAvatarNew::FinalizeStaticObject()
{
	if( !s_bInitializeStaticObject ) return;

	SAFE_DELETE( s_pColorAdjFilter );
	SAFE_RELEASE_SPTR( s_hTempLight[0] );
	SAFE_RELEASE_SPTR( s_hTempLight[1] );
	SAFE_RELEASE_SPTR( s_hTempEnviTexture );

	s_bInitializeStaticObject = false;
}

void CDnRenderAvatarNew::Initialize( int nWidth, int nHeight, SCameraInfo &CameraInfo, int nViewPortWidth, int nViewPortHeight, EtFormat Format, bool bUseColorAdjFilter, bool bUseBorderAlpha )
{
	CEtRTTRenderer::Initialize( CameraInfo, nWidth, nHeight, nViewPortWidth, nViewPortHeight, Format );

	// Handle this during actual Initialize.
	InitializeStaticObject();

	m_bUseColorAdjFilter = bUseColorAdjFilter;
	m_bUseBorderAlpha = bUseBorderAlpha;

	// Default camera setup. As long as z value is not too close to clip, use a fixed value.
	m_hCamera->LookAt( EtVector3(0.0f, m_fDefaultCameraYPos, m_fDefaultCameraZPos), EtVector3(0.0f, m_fDefaultCameraYPos, 0.0f), EtVector3(0.0f, 1.0f, 0.0f) );
	m_hCamera->CalcInvViewMat();

	m_nDefaultViewPortWidth = nViewPortWidth;
	m_nDefaultViewPortHeight = nViewPortHeight;
}

void CDnRenderAvatarNew::CameraLookAt( EtVector3 &Eye, EtVector3 &At, EtVector3 &Up )
{
	m_hCamera->LookAt( Eye, At, Up );
}

bool CDnRenderAvatarNew::SetActor( DnActorHandle hPlayer, bool bShowWeapon, bool bForceCashWeapon )
{
	// Refer to VillageTask OnLoadRecvCharUserEnteredCallback (exclude monsters for now; start with inspect and cash shop).
	DnActorHandle hActor = CreateActor( hPlayer->GetClassID(), false, false, false );
	if( !hActor ) return false;
	hActor->Show( false );
	//hActor->SetSoftAppear( true );

	hActor->Initialize();

	// Equip parts with AttachItem.
	m_hActor = hActor;
	CheckType();

	if( m_eActorType == ActorEnum::PlayerActor )
	{
		CDnPlayerActor *pPlayer = (CDnPlayerActor *)hPlayer.GetPointer();
		CDnPlayerActor *pActor = (CDnPlayerActor *)hActor.GetPointer();

		// Note: When someone enters view in town, only their latest job is set here, not their job history.
		// If needed later, history must be configured..
		pActor->SetWeaponViewOrder( 0, ( bForceCashWeapon ) ? true : pPlayer->IsViewWeaponOrder(0) );
		pActor->SetWeaponViewOrder( 1, ( bForceCashWeapon ) ? true : pPlayer->IsViewWeaponOrder(1) );
		for( int i = 0; i < CDnParts::SwapParts_Amount; ++i )
			pActor->SetPartsViewOrder( i, ( bForceCashWeapon ) ? true : pPlayer->IsViewPartsOrder(i) );
		pActor->SetHideHelmet( ( bForceCashWeapon ) ? false : pPlayer->IsHideHelmet() );

		int nDefaultPartsInfo[4];
		for( int i = 0; i < 4; i++ ) {
			nDefaultPartsInfo[ i ] = pPlayer->GetDefaultPartsInfo( (CDnParts::PartsTypeEnum)(CDnParts::Body + i) );
		}
		pActor->SetDefaultPartsInfo( nDefaultPartsInfo );
		pActor->SetPartsColor( MAPartsBody::HairColor, pPlayer->GetPartsColor(MAPartsBody::HairColor) );
		pActor->SetPartsColor( MAPartsBody::EyeColor, pPlayer->GetPartsColor(MAPartsBody::EyeColor) );
		pActor->SetPartsColor( MAPartsBody::SkinColor, pPlayer->GetPartsColor(MAPartsBody::SkinColor) );

		// When created as Sorceress in Normal_Stand state, set BattleMode=false and call AttachWeapon so the equipped book stays closed on her back.
		// This is required so DnWeapon::ResetDefaultAction is called,
		// Calls CDnPlayerActor::OnResetAttachWeaponAction,
		// Start the book action as Normal_Stand here.
		// In other words, it is better to set BattleMode before attaching the weapon.
		pActor->SetBattleMode( false );

		// For players, set RTTRenderMode first (no need to do it first for mounts or pets).
		// This is required to set the left-hand weapon for RenderTarget when attaching Kali's weapon.
		pActor->SetRTTRenderMode( true, this );

		// Note: the old Cash Shop called default parts unless it was a nude part, so
		// Normal parts appearance with grade-based visual changes does not display properly.
		for( int i = 0; i < CDnParts::PartsTypeEnum_Amount; i++ )
		{
			DnPartsHandle hParts = pPlayer->GetParts( (CDnParts::PartsTypeEnum)i );
			if( !hParts ) continue;

			if( (CDnParts::PartsTypeEnum)i == CDnParts::Helmet && pActor->IsHideHelmet() )
				continue;
			AttachItem( hParts->GetClassID(), 0, 0, hParts->GetLookItemID() );
		}

		if( bShowWeapon )
		{
			for( int i = 0; i < CDnWeapon::EquipSlot_Amount; i++ )
			{
				DnWeaponHandle hWeapon = pPlayer->GetWeapon( i );
				if( !hWeapon ) continue;

				AttachItem( hWeapon->GetClassID(), hWeapon->GetEnchantLevel() );
			}
		}

		for( int i = 0; i < CDnParts::CashPartsTypeEnum_Amount; i++ )
		{
			DnPartsHandle hParts = pPlayer->GetCashParts( (CDnParts::PartsTypeEnum)i );
			if( !hParts ) continue;

			if( i < CDnParts::SwapParts_Amount && pActor->IsViewPartsOrder(i) == false )
				continue;

			if( (CDnParts::PartsTypeEnum)i == CDnParts::CashHelmet && pActor->IsHideHelmet() )
				continue;

			AttachItem( hParts->GetClassID(), 0, ( (i == CDnParts::CashRing2) ? 1 : 0 ), hParts->GetLookItemID() );
		}

		if( bShowWeapon )
		{
			for( int i = 0; i < CDnWeapon::EquipSlot_Amount; i++ )
			{
				DnWeaponHandle hWeapon = pPlayer->GetCashWeapon( i );
				if( !hWeapon ) continue;

				if( pActor->IsViewWeaponOrder(i) == false )
					continue;

				AttachItem( hWeapon->GetClassID(), hWeapon->GetEnchantLevel(), 0, hWeapon->GetLookItemID() );
			}
		}

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
		// Like login character select screen, CombineParts is not called, so forcibly call hiding right glove mesh when warrior equips gauntlet.
		pActor->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND

		//EtVector2 vDir( sin( EtToRadian( pPacket->fRotate ) ), cos( EtToRadian( pPacket->fRotate ) ) );
		EtVector2 vDir( 0.0f, 1.0f );
		pActor->Look( vDir );

		if( CDnActor::s_hLocalActor )
		{
			LOCAL_TIME LocalTime = CDnActor::s_hLocalActor->GetLocalTime();
			EtMatrix WorldMat = *pActor->GetMatEx();
			pActor->MAActorRenderBase::Process( &WorldMat, LocalTime, 0.0f );
		}

		pActor->SetActionQueue( "Normal_Stand" );
		pActor->SetGuildSelfView( pPlayer->GetGuildSelfView() );

		//pActor->CombineParts(); // Do not combine in render avatar.
		pActor->SetHP( 1 );		// Some code checks HP <= 0 directly, so force HP to 1 to prevent skipping.
		hActor->Show( true );	// Because it must still draw to the render target,

		bool bShowFieldActor = false;
		if( bShowFieldActor )
		{
			// Running Process performs various checks (collision, height adjustment, death check, etc.), so even if placed at 0, 0, 0, it moves up onto the terrain height.
			// Seems better to control directly without running Process.

			pActor->SetPosition( *hPlayer->GetPosition() );	// Set position to display on the field for verification.
			pActor->SetHP( 100 );	// Because it won't transition to ProcessDie if Process is not run,
			pActor->SetProcess( true );	// No need to run the actual Process (but to show particles... shouldn't it run? Maybe selectively run parts since full Process does too much?)
		}

		MAPartsBody *pActorPartsBody = dynamic_cast<MAPartsBody*>(hActor.GetPointer());
		pActorPartsBody->SetIgnoreDeleteWaitOnepiece( true );
		AddObject( pActorPartsBody->GetObjectHandle() );
		//AddObject( pActorPartsBody->GetCombineObject() ); // Alpha (head) and weapon are excluded from combine object.
		SetRenderFrameCount( -1 );	// Setting to -1 keeps rendering until the object is released.

#ifdef PRE_ADD_CASH_COSTUME_AURA
		pActor->ComputeRTTModeCostumeAura(hPlayer);
#endif
	}
	else if( m_eActorType == ActorEnum::VehicleActor )
	{

		CDnVehicleActor *pVehicleActor = (CDnVehicleActor *)hPlayer.GetPointer();
		CDnVehicleActor *pActor = (CDnVehicleActor *)hActor.GetPointer();

		// For players, it is made of parts so it was only set on parts, but for horses, it must be done starting from the main body.
		if( pActor->GetObjectHandle() ) pActor->GetObjectHandle()->SetRenderType( RT_TARGET );

		// Fetch and set horse info
		for( int i = Vehicle::Slot::Saddle; i < Vehicle::Slot::Max; ++i )
		{
			if( !pVehicleActor->GetMyPlayerActor() || pVehicleActor->GetMyPlayerActor()->IsDestroy() ) continue;
			if( !pVehicleActor->GetMyPlayerActor()->GetVehicleInfo().Vehicle[i].nItemID ) continue;
			pActor->EquipItem( pVehicleActor->GetMyPlayerActor()->GetVehicleInfo().Vehicle[i] );

			DnSimplePartsHandle hParts = pActor->GetVehicleParts( (Vehicle::Parts::eVehicleParts)(i - Vehicle::Slot::Saddle) );
			if( hParts && hParts->GetObjectHandle() ) hParts->GetObjectHandle()->SetRenderType( RT_TARGET );

			if( i == Vehicle::Slot::Hair )	// For manes, force-enable alpha blend and two-pass.
			{
				if( hParts && hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
				{
					hParts->GetObjectHandle()->GetSkinInstance()->EnableAlphaBlend( true );
					hParts->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
				}
			}
		}
		pActor->ChangeHairColor( pVehicleActor->GetMyPlayerActor()->GetVehicleInfo().dwPartsColor1 );

		EtVector2 vDir( 0.0f, 1.0f );
		pActor->Look( vDir );

		// Actors used in RenderAvatar are not in process list and run separately, so also exclude from CDnActor::ProcessClass.
		// MAActorRenderBase::Process is called in ProcessClass, and this is also excluded.
		// Because of this, stored LocalTime is used when determining animation frame, and this LocalTime is at its initial value,
		// Animation is properly applied only after the animation loop runs at least once.
		// So force passing local time to the animation calculation as below.
		// For players it was barely noticeable because of Normal_Stand, but it actually had the same issue.
		if( CDnActor::s_hLocalActor )
		{
			LOCAL_TIME LocalTime = CDnActor::s_hLocalActor->GetLocalTime();
			EtMatrix WorldMat = *pActor->GetMatEx();
			pActor->MAActorRenderBase::Process( &WorldMat, LocalTime, 0.0f );
			pActor->SetMyPlayerActor(CDnActor::s_hLocalActor);
		}

		pActor->SetRTTRenderMode( true, this );
		pActor->SetActionQueue( "Stand" );
		pActor->SetHP( 1 );		// Some code checks HP <= 0 directly, so force HP to 1 to prevent skipping.
		hActor->Show( true );	// Because it must still draw to the render target,

		AddObject( pActor->GetObjectHandle() );
		SetRenderFrameCount( -1 );	// Setting to -1 keeps rendering until the object is released.

	}
	else if( m_eActorType == ActorEnum::PetActor )
	{
		CDnPetActor* pPlayerPetActor = (CDnPetActor *)hPlayer.GetPointer();
		CDnPetActor* pPetActor = (CDnPetActor *)hActor.GetPointer();

		pPetActor->SetDefaultMesh();
		if( pPetActor->GetObjectHandle() ) pPetActor->GetObjectHandle()->SetRenderType( RT_TARGET );

		for( int i=Pet::Slot::Accessory1; i<Pet::Slot::Max; ++i )
		{
			if( !pPlayerPetActor->GetPetInfo().Vehicle[i].nItemID ) continue;
			pPetActor->EquipItem( pPlayerPetActor->GetPetInfo().Vehicle[i] );

			DnSimplePartsHandle hParts = pPetActor->GetPetParts( (Vehicle::Parts::eVehicleParts)(i - Pet::Slot::Accessory1) );
			if( hParts && hParts->GetObjectHandle() ) hParts->GetObjectHandle()->SetRenderType( RT_TARGET );
		}

		pPetActor->ChangeColor( ePetParts::PET_PARTS_BODY, pPlayerPetActor->GetPetInfo().dwPartsColor1 );
		pPetActor->ChangeColor( ePetParts::PET_PARTS_NOSE, pPlayerPetActor->GetPetInfo().dwPartsColor2 );

		EtVector2 vDir( 0.0f, 1.0f );
		pPetActor->Look( vDir );

		LOCAL_TIME LocalTime = CDnActor::s_hLocalActor->GetLocalTime();
		EtMatrix WorldMat = *pPetActor->GetMatEx();
		pPetActor->MAActorRenderBase::Process( &WorldMat, LocalTime, 0.0f );
		pPetActor->SetMyMaster( dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer()) );

		pPetActor->SetRTTRenderMode( true, this );
		pPetActor->SetActionQueue( "Stand" );
		pPetActor->SetHP( 1 );		// Some code checks HP <= 0 directly, so force HP to 1 to prevent skipping.
		hActor->Show( true );	// Because it must still draw to the render target,
		AddObject( pPetActor->GetObjectHandle() );
		SetRenderFrameCount( -1 );	// Setting to -1 keeps rendering until the object is released.
	}

	// In each part's link bone processing, get the actor's position to calculate matrices of linked children,
	// Since Process is not run, the initial position must be set separately like this.
	if( hActor->GetObjectHandle() )
	{
		EtMatrix Mat;
		EtMatrixIdentity( &Mat );
		hActor->GetObjectHandle()->SetWorldMat( Mat );
	}

	return true;
}

bool CDnRenderAvatarNew::SetActor( int nClassID, bool bShowWeapon )
{
	DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );

	// Refer to VillageTask OnLoadRecvCharUserEnteredCallback (exclude monsters for now; start with inspect and cash shop).
	DnActorHandle hActor = CreateActor( nClassID, false, false, false );
	if( !hActor ) return false;
	hActor->Show( false );
	//hActor->SetSoftAppear( true );

	hActor->Initialize();

	// Assign to m_hActor right after creation because parts in the DefaultCreate table must be attached while loading.
	m_hActor = hActor;
	CheckType();

	if( m_eActorType == ActorEnum::PlayerActor )
	{
		// Just calling up to hActor->Initialize causes player characters to appear headless without parts properly attached.
		// So need to read the default parts and set them separately.
		CDnPlayerActor *pActor = (CDnPlayerActor *)hActor.GetPointer();

		pActor->SetWeaponViewOrder( 0, true );
		pActor->SetWeaponViewOrder( 1, true );
		for( int i = 0; i < CDnParts::SwapParts_Amount; ++i )
			pActor->SetPartsViewOrder( i, true );
		pActor->SetHideHelmet( false );

		char *szLabels[CDnParts::PartsTypeEnum_Amount] = { "_Face", "_Hair", "_Helmet", "_Body", "_Leg", "_Hand", "_Foot", NULL, NULL, NULL, NULL };
		char szBuffer[128] = { 0, };
		DNTableFileFormat*  pDefaultCreateTable = GetDNTable( CDnTableDB::TDEFAULTCREATE );

		int nDefaultPartsInfo[4];
		nDefaultPartsInfo[ CDnParts::Body-CDnParts::DefaultPartsType_Min ] = pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_DefaultBody" )->GetInteger();
		nDefaultPartsInfo[ CDnParts::Leg-CDnParts::DefaultPartsType_Min ] = pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_DefaultLeg" )->GetInteger();
		nDefaultPartsInfo[ CDnParts::Hand-CDnParts::DefaultPartsType_Min ] = pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_DefaultHand" )->GetInteger();
		nDefaultPartsInfo[ CDnParts::Foot-CDnParts::DefaultPartsType_Min ] = pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_DefaultFoot" )->GetInteger();
		pActor->SetDefaultPartsInfo( nDefaultPartsInfo );

		// For now, show only the very first item among the default parts.
		float fIntensity = 1.1f;
		DWORD dwColorR10G10B10 = 0;
		D3DCOLOR dwHairColor = D3DCOLOR_XRGB(
			pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_HairColor1R" )->GetInteger(),
			pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_HairColor1G" )->GetInteger(),
			pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_HairColor1B" )->GetInteger() );
		CDnParts::ConvertD3DCOLORToR10G10B10( &dwColorR10G10B10, dwHairColor, fIntensity );
		pActor->SetPartsColor( MAPartsBody::HairColor, dwColorR10G10B10 );

		fIntensity = 1.0f;
		D3DCOLOR dwEyeColor = D3DCOLOR_XRGB(
			pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_EyeColor1R" )->GetInteger(),
			pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_EyeColor1G" )->GetInteger(),
			pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_EyeColor1B" )->GetInteger() );
		CDnParts::ConvertD3DCOLORToR10G10B10( &dwColorR10G10B10, dwEyeColor, fIntensity );
		pActor->SetPartsColor( MAPartsBody::EyeColor, dwColorR10G10B10 );

		int nA = (int)(pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_SkinColor1A" )->GetFloat() * 255);
		int nR = (int)(pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_SkinColor1R" )->GetFloat() * 255);
		int nG = (int)(pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_SkinColor1G" )->GetFloat() * 255);
		int nB = (int)(pDefaultCreateTable->GetFieldFromLablePtr( nClassID, "_SkinColor1B" )->GetFloat() * 255);
		DWORD dwSkinColor = D3DCOLOR_ARGB( nA, nR, nG, nB );
		pActor->SetPartsColor( MAPartsBody::SkinColor, dwSkinColor );

		pActor->SetBattleMode( false );
		pActor->SetRTTRenderMode( true, this );

		for( int nIndex = 0; nIndex < CDnParts::PartsTypeEnum_Amount; ++nIndex )
		{
			if( NULL == szLabels[nIndex] )
				continue;

			sprintf_s( szBuffer, "%s%d", szLabels[nIndex], 1 );
			int nPartItemID = pDefaultCreateTable->GetFieldFromLablePtr( nClassID, szBuffer )->GetInteger();
			AttachItem( nPartItemID );
		}

		if( bShowWeapon )
		{
			for( int nIndex = 0; nIndex < CDnWeapon::EquipSlot_Amount; ++nIndex )
			{
				sprintf_s( szBuffer, "_Weapon%d", nIndex+1 );
				int nWeaponItemID = pDefaultCreateTable->GetFieldFromLablePtr( nClassID, szBuffer )->GetInteger();
				AttachItem( nWeaponItemID );
			}
		}

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
		// Like login character select screen, CombineParts is not called, so forcibly call hiding right glove mesh when warrior equips gauntlet.
		pActor->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND

		EtVector2 vDir( 0.0f, 1.0f );
		pActor->Look( vDir );

		if( CDnActor::s_hLocalActor )
		{
			LOCAL_TIME LocalTime = CDnActor::s_hLocalActor->GetLocalTime();
			EtMatrix WorldMat = *pActor->GetMatEx();
			pActor->MAActorRenderBase::Process( &WorldMat, LocalTime, 0.0f );
		}

		pActor->SetActionQueue( "Normal_Stand" );

		//pActor->CombineParts(); // Do not combine in render avatar.
		pActor->SetHP( 1 );		// Some code checks HP <= 0 directly, so force HP to 1 to prevent skipping.
		hActor->Show( true );	// Because it must still draw to the render target,

		MAPartsBody *pActorPartsBody = dynamic_cast<MAPartsBody*>(hActor.GetPointer());
		pActorPartsBody->SetIgnoreDeleteWaitOnepiece( true );
		AddObject( pActorPartsBody->GetObjectHandle() );
		//AddObject( pActorPartsBody->GetCombineObject() ); // Alpha (head) and weapon are excluded from combine object.
		SetRenderFrameCount( -1 );	// Setting to -1 keeps rendering until the object is released.
	}
	else if( m_eActorType == ActorEnum::VehicleActor )
	{

		CDnVehicleActor *pActor = (CDnVehicleActor *)hActor.GetPointer();
		if( pActor->GetObjectHandle() ) pActor->GetObjectHandle()->SetRenderType( RT_TARGET );

		EtVector2 vDir( 0.0f, 1.0f );
		pActor->Look( vDir );

		if( CDnActor::s_hLocalActor )
		{
			LOCAL_TIME LocalTime = CDnActor::s_hLocalActor->GetLocalTime();
			EtMatrix WorldMat = *pActor->GetMatEx();
			pActor->MAActorRenderBase::Process( &WorldMat, LocalTime, 0.0f );
		}

		pActor->SetRTTRenderMode(true,this);
		pActor->SetActionQueue( "Stand" );
		pActor->SetHP( 1 );		// Some code checks HP <= 0 directly, so force HP to 1 to prevent skipping.
		hActor->Show( true );	// Because it must still draw to the render target,

		AddObject( pActor->GetObjectHandle() );
		SetRenderFrameCount( -1 );	// Setting to -1 keeps rendering until the object is released.

	}
	else if( m_eActorType == ActorEnum::PetActor )
	{
		CDnPetActor *pPetActor = (CDnPetActor *)hActor.GetPointer();
		pPetActor->SetDefaultMesh();
		if( pPetActor->GetObjectHandle() ) pPetActor->GetObjectHandle()->SetRenderType( RT_TARGET );

		EtVector2 vDir( 0.0f, 1.0f );
		pPetActor->Look( vDir );

		if( CDnActor::s_hLocalActor )
		{
			LOCAL_TIME LocalTime = CDnActor::s_hLocalActor->GetLocalTime();
			EtMatrix WorldMat = *pPetActor->GetMatEx();
			pPetActor->MAActorRenderBase::Process( &WorldMat, LocalTime, 0.0f );
		}

		pPetActor->SetRTTRenderMode( true, this );
		pPetActor->SetActionQueue( "Stand" );
		hActor->Show( true );

		AddObject( pPetActor->GetObjectHandle() );
		SetRenderFrameCount( -1 );	// Setting to -1 keeps rendering until the object is released.
	}

	// In each part's link bone processing, get the actor's position to calculate matrices of linked children,
	// Since Process is not run, the initial position must be set separately like this.
	if( hActor->GetObjectHandle() )
	{
		EtMatrix Mat;
		EtMatrixIdentity( &Mat );
		hActor->GetObjectHandle()->SetWorldMat( Mat );
	}

	return true;
}

void CDnRenderAvatarNew::CheckType()
{
	m_eActorType = ActorEnum::None;

	if( m_hActor )
		CheckActorType();
	else if( m_hEtc )
		CheckEtcType();
}

void CDnRenderAvatarNew::CheckActorType()
{
	CDnPlayerActor *pPlayerActor = dynamic_cast<CDnPlayerActor *>(m_hActor.GetPointer());

	CDnVehicleActor *pVehicleActor = dynamic_cast<CDnVehicleActor *>(m_hActor.GetPointer());
	CDnPetActor *pPetActor = dynamic_cast<CDnPetActor *>(m_hActor.GetPointer());

	if( pPlayerActor && !pVehicleActor )
		m_eActorType = ActorEnum::PlayerActor;
	else if( !pPlayerActor && pVehicleActor )
		m_eActorType = ActorEnum::VehicleActor;
	else if( !pPlayerActor && pPetActor )
		m_eActorType = ActorEnum::PetActor;
}

void CDnRenderAvatarNew::CheckEtcType()
{
	m_eActorType = ActorEnum::EtcObject;
}

bool CDnRenderAvatarNew::ResetActor( bool bInitCamera )
{
	if( m_hActor )
	{
		m_vecPartsInfo.clear();
		ClearRenderObject();
		//m_hActor->SetDestroy(); // SetProcess must be enabled for ProcessClass to check Destroy and delete it; otherwise it won't be deleted.
		SAFE_RELEASE_SPTR( m_hActor );	// Since SetProcess is not enabled, it must be released directly like this.
		m_eActorType = ActorEnum::None;
		SetRenderFrameCount( 0 );	// Reset what was set to -1 back to 0 so it doesn't get included during RTT render.
	}
	if( bInitCamera ) InitCameraControl();
	return true;
}

int CDnRenderAvatarNew::GetActorID()
{
	if( m_hActor )
		return m_hActor->GetClassID();
	return 0;
}

bool CDnRenderAvatarNew::AttachItem( int nItemID, int nEnchantLevel, int nSubArg1, int nLookItemID )
{
	if( !m_hActor ) return true;

	if( m_eActorType == ActorEnum::PlayerActor )
	{
		CDnPlayerActor *pActor = (CDnPlayerActor *)m_hActor.GetPointer();
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
		if( !pSox ) return false;
		if( !pSox->IsExistItem( nItemID ) ) return false;

#ifdef PRE_ADD_CASH_COSTUME_AURA
		pActor->SetAuraRenderType(RT_TARGET);
#endif

		eItemTypeEnum ItemType = CDnItem::GetItemType( nItemID );
		if( ItemType == ITEMTYPE_HAIRDYE )
		{
			DWORD dwColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
			pActor->SetPartsColor( MAPartsBody::HairColor, dwColor );
		}
		else if( ItemType == ITEMTYPE_EYEDYE )
		{
			DWORD dwColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
			pActor->SetPartsColor( MAPartsBody::EyeColor, dwColor );
		}
		else if( ItemType == ITEMTYPE_SKINDYE )
		{
			DWORD dwColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
			pActor->SetPartsColor( MAPartsBody::SkinColor, dwColor );
		}
		else if( ItemType == ITEMTYPE_PARTS )
		{
			DnPartsHandle hParts = CDnParts::CreateParts( nItemID, 0, 0, 0, 0, 0, false, nLookItemID);
			if( hParts )
			{
				hParts->SetRTTRenderMode( true, this );
				bool bCash = CDnItem::IsCashItem( nItemID );

				// Thought no special handling was needed since the actor is used as-is, but
				// Had to process without Combine for performance, which caused an issue with one-piece outfits.
				// One-piece outfit handling is all done when combining, but missing when not combining.
				// This is the same issue even on the login character selection screen where Combine is not performed.
				// Instead, on the login screen, after trying on normal cash outfits then equipping a one-piece outfit (or vice versa: equipping normal cash parts after a one-piece outfit)
				// Because it wasn't done, there was no issue.
				//
				// Currently since we do not combine, we clear old info without remembering it when equipping new parts, but
				// Because of this, when attaching another cash upper part while a one-piece outfit is equipped, info for previously worn lower, gloves, and pants is missing.
				//
				// To prevent this, we need to remember normal parts and cash parts separately just like character equipment info,
				// This ends up pulling out all the equipment functionality from ItemTask.
				// Since this render avatar code was meant to touch only the rendering routine as much as possible (that is why AttachItem is supported while DetachItem was intentionally omitted).
				// Decided not to support this feature.
				//
				// Still, to make it look as natural as possible, handle one-piece outfits as an exception and keep their info.

				if( !bCash )
				{
					// Since RenderAvatar does not call CombineParts, when a normal item comes in, check matching cash item
					// If there is a cash item, skip equipping and move on.
					bool bEquip = true;
					if( hParts->GetPartsType() >= CDnParts::Helmet && hParts->GetPartsType() <= CDnParts::Foot )
					{
						if( pActor->GetCashParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() - 2 ) ) )
						{
							bEquip = false;
						}
					}

					// Check if it is an equipped one-piece outfit part (looks like one-piece sub-parts come through here?).
					for( int i = 0; i < (int)m_vecPartsInfo.size(); ++i )
					{
						if( m_vecPartsInfo[i].nType == hParts->GetPartsType() )
						{
							bEquip = false;
							break;
						}
					}

					if( bEquip )
					{
						pActor->DetachParts( hParts->GetPartsType() );
						pActor->AttachParts( hParts, hParts->GetPartsType(), true, false );
						if( hParts->GetObjectHandle() ) hParts->GetObjectHandle()->SetRenderType( RT_TARGET );

						if( hParts->GetPartsType() == CDnParts::Hair )
						{
							// For hair, alpha blending is sometimes not set in the tool. Just force-enable it.
							if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
								hParts->GetObjectHandle()->GetSkinInstance()->EnableAlphaBlend( true );
						}
					}
					else
					{
						SAFE_RELEASE_SPTR( hParts );
					}
				}
				else
				{
					// If currently wearing a one-piece outfit,
					if( m_vecPartsInfo.size() )
					{
						// Check whether the part currently being equipped corresponds to a one-piece outfit slot.
						// Currently all one-piece outfits have the same sub-parts, but if that ever changes,
						// Should not just check hParts->GetPartsType(), but extract and check all subparts too.
						bool bDetachOnepiece = false;
						for( int i = 0; i < (int)m_vecPartsInfo.size(); ++i )
						{
							if( (m_vecPartsInfo[i].nType - 2) == hParts->GetPartsType() )
							{
								bDetachOnepiece = true;
								break;
							}
						}

						if( bDetachOnepiece )
						{
							std::vector<SPartsInfo> vecPartsInfo;
							vecPartsInfo = m_vecPartsInfo;
							m_vecPartsInfo.clear();

							// Unequip the one-piece outfit first (the first index is the one-piece part slot).
							pActor->DetachCashParts( (CDnParts::PartsTypeEnum)( vecPartsInfo[0].nType - 2 ) );

							// After iterating through all previous info
							// Since all types are already populated, just fetch the item IDs matching the type and re-save.
							for( int i = 0; i < (int)vecPartsInfo.size(); ++i )
								AttachItem( vecPartsInfo[i].nItemID );

							// Run the routine below as if equipping a normal cash item.
						}
					}

#ifdef PRE_ADD_TRANSPARENCY_COSTUME
					bool bTransparentSkin = CDnParts::IsTransparentSkin( hParts->GetClassID(), pActor->GetClassID() );
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME

					// If the outfit to equip is a one-piece, separately store the parts in the slots covered by it.
					// If only sub parts are processed, the self slot is omitted, so add self separately.
					// (Must remember this so that when wearing cash gloves while wearing a one-piece outfit, it can revert to the previously worn top.)
					if( hParts->IsExistSubParts() )
					{
						SPartsInfo Info;
						Info.nType = (int)hParts->GetPartsType() + 2;	// Since one-piece outfits are cash items, add 2 to match the normal item type.
						Info.nItemID = 0;
						m_vecPartsInfo.push_back( Info );
						for( int i = 0; i < hParts->GetSubPartsCount(); ++i )
						{
							Info.nType = (int)hParts->GetSubPartsIndex(i) + 2;	// The written subtype is also a cash item type, so add +2 to match the normal item type.
							m_vecPartsInfo.push_back( Info );
						}

						// Since all types are already populated, just fetch the item IDs matching the type and re-save.
						for( int i = 0; i < (int)m_vecPartsInfo.size(); ++i )
						{
							if( pActor->GetCashParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType - 2 ) ) )
							{
								m_vecPartsInfo[i].nItemID = pActor->GetCashParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType - 2 ) )->GetClassID();
								pActor->DetachCashParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType - 2 ) );
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
								// Show normal parts appearance when wearing a transparent one-piece costume.
								if( bTransparentSkin )
								{
									DnPartsHandle hNormalParts = pActor->GetParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) );
									if( hNormalParts && !hNormalParts->GetObjectHandle() )
									{
										DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
	#ifdef PRE_FIX_MEMOPT_EXT
										std::string szAniName;
										CommonUtil::GetFileNameFromFileEXT(szAniName, pActorSox, pActor->GetClassID(), "_AniName");
	#else
										std::string szAniName = pActorSox->GetFieldFromLablePtr( pActor->GetClassID(), "_AniName" )->GetString();
	#endif
										hNormalParts->CreateObject( m_hActor, szAniName.c_str() );
									}
								}
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME
							}
							else if( pActor->GetParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) ) )
							{
								m_vecPartsInfo[i].nItemID = pActor->GetParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) )->GetClassID();
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
								if( !bTransparentSkin ) pActor->GetParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) )->FreeObject();
#else
								pActor->DetachParts( (CDnParts::PartsTypeEnum)( m_vecPartsInfo[i].nType ) );
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME
							}
							// If detached properly on the actor side, all default parts would be present even in empty slots.
							// Just in case, add the handling below.
							else if( m_vecPartsInfo[i].nType >= CDnParts::DefaultPartsType_Min && m_vecPartsInfo[i].nType <= CDnParts::DefaultPartsType_Max )
							{
								m_vecPartsInfo[i].nItemID = pActor->GetDefaultPartsInfo( (CDnParts::PartsTypeEnum)m_vecPartsInfo[i].nType );
							}
						}
					}

					// Since RenderAvatar does not call CombineParts, remove normal items here.
					if( hParts->GetPartsType() >= CDnParts::CashHelmet && hParts->GetPartsType() <= CDnParts::CashFoot )
					{
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
						// Change to FreeObject instead of Detach due to transparent one-piece costume, so CreateObject can be called again later when needed.
						DnPartsHandle hNormalParts = pActor->GetParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() + 2 ) );
						if( hNormalParts && !bTransparentSkin ) hNormalParts->FreeObject();
#else
						if( pActor->GetParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() + 2 ) ) )
							pActor->DetachParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() + 2 ) );
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME
					}

					if( nSubArg1 == 0 )
					{
						pActor->DetachCashParts( hParts->GetPartsType() );
						pActor->AttachCashParts( hParts, hParts->GetPartsType(), true, false );
					}
					else if( nSubArg1 == 1 )
					{
						if( hParts->GetPartsType() == CDnParts::CashRing )
						{
							pActor->DetachCashParts( CDnParts::CashRing2 );
							pActor->AttachCashParts( hParts, CDnParts::CashRing2, true, false );
						}
					}
					if( hParts->GetObjectHandle() ) hParts->GetObjectHandle()->SetRenderType( RT_TARGET );
#ifdef PRE_ADD_TRANSPARENCY_COSTUME
					else if( bTransparentSkin )
					{
						DnPartsHandle hNormalParts = pActor->GetParts( (CDnParts::PartsTypeEnum)( hParts->GetPartsType() + 2 ) );
						if( hNormalParts && !hNormalParts->GetObjectHandle() )
						{
							DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
	#ifdef PRE_FIX_MEMOPT_EXT
							std::string szAniName;
							CommonUtil::GetFileNameFromFileEXT(szAniName, pActorSox, pActor->GetClassID(), "_AniName");
	#else
							std::string szAniName = pActorSox->GetFieldFromLablePtr( pActor->GetClassID(), "_AniName" )->GetString();
	#endif
							hNormalParts->CreateObject( m_hActor, szAniName.c_str() );
						}
					}
#endif	// #ifdef PRE_ADD_TRANSPARENCY_COSTUME

					// Do this only for parts for now. Weapons rarely seem to have this case.
					if( hParts->GetPartsType() == CDnParts::CashHelmet )
					{
						// Some headwear parts use alpha textures at mesh edges even though alpha blending is not enabled in the tool.
						// Force-enable it so it displays properly even like this.
						// However, after making PostProcess preserve the alpha value as-is,
						// The issue where border alpha looked weird seems mostly resolved.
						// So force disabling alpha blend.
						//if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
						//	hParts->GetObjectHandle()->GetSkinInstance()->EnableAlphaBlend( true );
					}
					else if( hParts->GetPartsType() == CDnParts::CashWing )
					{
						// For wings, force alpha blend and two-pass on for clean rendering.
						if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
						{
							hParts->GetObjectHandle()->GetSkinInstance()->EnableAlphaBlend( true );
							hParts->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
						}
					}
				}
			}

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
			// Like login character select screen, CombineParts is not called, so forcibly call hiding right glove mesh when warrior equips gauntlet.
			pActor->OnCombineParts();
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
		}
		else if( ItemType == ITEMTYPE_WEAPON )
		{
			DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nItemID, 0, 0, nEnchantLevel, 0, 0, false, false, true, ITEMCLSID_NONE );
			if( hWeapon )
			{
				hWeapon->SetRTTRenderMode( true, this );
				bool bCash = CDnItem::IsCashItem( nItemID );

				int nEquipIndex = CDnCharStatusDlg::EQUIPTYPE_2_EQUIPINDEX( hWeapon->GetEquipType() ) - EQUIP_WEAPON1;
				ASSERT( nEquipIndex == 0 || nEquipIndex == 1 );

				if( !bCash )
				{
					pActor->DetachWeapon( nEquipIndex );
					pActor->AttachWeapon( hWeapon, nEquipIndex, true );
				}
				else
				{
					pActor->DetachCashWeapon( nEquipIndex );
					pActor->AttachCashWeapon( hWeapon, nEquipIndex, true, false );
				}

				if( hWeapon->GetObjectHandle() )
				{
					// Combining a normal mace with a cash flail is problematic.
					// Too much resource work would be required (new mace animations, mace act files, etc.), so do not force disabling simulation.
					//hWeapon->GetObjectHandle()->DisableSimulation();
					hWeapon->GetObjectHandle()->SetRenderType( RT_TARGET );
				}
			}
		}
		// Face product and hairstyle product.
		else if( ItemType == ITEMTYPE_FACIAL || ItemType == ITEMTYPE_HAIRDRESS )
		{
			int nParam = pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
			AttachItem( nParam );

			// Since alpha objects are not sorted by distance, re-equip the cash hat when changing Hair so it renders behind Hair.
			if( ItemType == ITEMTYPE_HAIRDRESS )
			{
				if( pActor->GetCashParts( CDnParts::CashHelmet ) )
				{
					DnPartsHandle hHelmet = pActor->GetCashParts( CDnParts::CashHelmet );
					if (!hHelmet)
					{
#ifdef _WORK
						std::wstring str = FormatW(L"DnRenderAvatar::AttachItem - Character CashHelmet does not exist : (%d)", nItemID);
						GetInterface().AddChatMessage(CHATTYPE_NOTICE, L"", str.c_str(), true);
#endif
						return false;
					}

					int nAttachHelmetId = (hHelmet->GetLookItemID() != ITEMCLSID_NONE) ? hHelmet->GetLookItemID() : hHelmet->GetClassID();
					pActor->DetachCashParts( CDnParts::CashHelmet );
					AttachItem(nAttachHelmetId);
				}
			}
		}
	}
	else if( m_eActorType == ActorEnum::VehicleActor )
	{
		// Currently mounts only use SetActor by receiving a created actor, so AttachItem is still unimplemented.
	}

	return true;
}

bool CDnRenderAvatarNew::DetachItem( int nItemID )
{
	if( !m_hActor ) return true;

	if( m_eActorType == ActorEnum::PlayerActor )
	{
		CDnPlayerActor *pActor = (CDnPlayerActor *)m_hActor.GetPointer();
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
		if( !pSox ) return false;
		if( !pSox->IsExistItem( nItemID ) ) return false;

		eItemTypeEnum ItemType = CDnItem::GetItemType( nItemID );
		if( ItemType == ITEMTYPE_PARTS )
		{
			// Switching a cash item while already wearing one is already handled, so no need to worry about it.
			// To switch to a normal item while wearing a cash item, we only need to force-detach the current cash parts.
			if( CDnItem::IsCashItem( nItemID ) )
			{
				CDnParts::PartsTypeEnum PartsType = CDnParts::GetPartsType( nItemID );
				pActor->DetachCashParts( PartsType );

				// Additional handling is needed when wearing a one-piece outfit.
				bool bDetachOnepiece = false;
				if( m_vecPartsInfo.size() )
				{
					// Check whether the part currently being unequipped corresponds to a one-piece outfit slot.
					int nItemID = 0;
					for( int i = 0; i < (int)m_vecPartsInfo.size(); ++i )
					{
						if( (m_vecPartsInfo[i].nType - 2) == PartsType )
						{
							bDetachOnepiece = true;
							break;
						}
					}

					if( bDetachOnepiece )
					{
						std::vector<SPartsInfo> vecPartsInfo;
						vecPartsInfo = m_vecPartsInfo;
						m_vecPartsInfo.clear();

						for( int i = 0; i < (int)vecPartsInfo.size(); ++i )
							AttachItem( vecPartsInfo[i].nItemID );
					}
				}
			}
		}
		else if( ItemType == ITEMTYPE_WEAPON )
		{
			if( CDnItem::IsCashItem( nItemID ) )
			{
				int nEquipIndex = CDnCharStatusDlg::EQUIPTYPE_2_EQUIPINDEX( CDnWeapon::GetEquipType( nItemID ) ) - EQUIP_WEAPON1;
				ASSERT( nEquipIndex == 0 || nEquipIndex == 1 );
				pActor->DetachCashWeapon( nEquipIndex );
			}
		}
	}
	return true;
}

bool CDnRenderAvatarNew::RefreshEquip( DnActorHandle hActor )
{
	if( !hActor ) return true;
	if( !m_hActor ) return true;

	if( m_eActorType == ActorEnum::PlayerActor )
	{
	}
	else if( m_eActorType == ActorEnum::VehicleActor )
	{
		CDnVehicleActor *pVehicleActor = (CDnVehicleActor *)hActor.GetPointer();
		CDnVehicleActor *pActor = (CDnVehicleActor *)m_hActor.GetPointer();

		// Fetch and set horse info
		for( int i = Vehicle::Slot::Saddle; i < Vehicle::Slot::Max; ++i )
		{
			if( !pVehicleActor->GetMyPlayerActor() || pVehicleActor->GetMyPlayerActor()->IsDestroy()) continue;
			if( !pVehicleActor->GetMyPlayerActor()->GetVehicleInfo().Vehicle[i].nItemID )
			{
				Vehicle::Parts::eVehicleParts ePartsType = Vehicle::Parts::Default;
				switch(i)
				{
				case Vehicle::Slot::Saddle:	ePartsType = Vehicle::Parts::Saddle; break;
				case Vehicle::Slot::Hair: ePartsType = Vehicle::Parts::Hair; break;
				}
				pActor->UnEquipItem( ePartsType );
			}
			else
			{
				pActor->EquipItem( pVehicleActor->GetMyPlayerActor()->GetVehicleInfo().Vehicle[i] );

				DnSimplePartsHandle hParts = pActor->GetVehicleParts( (Vehicle::Parts::eVehicleParts)(i - Vehicle::Slot::Saddle) );
				if( hParts && hParts->GetObjectHandle() ) hParts->GetObjectHandle()->SetRenderType( RT_TARGET );

				if( i == Vehicle::Slot::Hair )
				{
					if( hParts->GetObjectHandle() && hParts->GetObjectHandle()->GetSkinInstance() )
					{
						hParts->GetObjectHandle()->GetSkinInstance()->EnableAlphaBlend( true );
						hParts->GetObjectHandle()->GetSkinInstance()->SetRenderAlphaTwoPass( true );
					}
				}
			}
		}
		pActor->ChangeHairColor( pVehicleActor->GetMyPlayerActor()->GetVehicleInfo().dwPartsColor1 );
	}
	else if( m_eActorType == ActorEnum::PetActor )
	{
		CDnPetActor* pPetActor = (CDnPetActor *)hActor.GetPointer();
		CDnPetActor *pActor = (CDnPetActor *)m_hActor.GetPointer();

		for( int i = Pet::Slot::Accessory1; i < Pet::Slot::Max; ++i )
		{
			if( !pPetActor->GetPetInfo().Vehicle[i].nItemID )
			{
				Pet::Parts::ePetParts ePartsType = Pet::Parts::Default;
				switch( i )
				{
					case Pet::Slot::Accessory1: ePartsType = Pet::Parts::PetAccessory1; break;
					case Pet::Slot::Accessory2: ePartsType = Pet::Parts::PetAccessory2; break;
				}
				pActor->UnEquipItem( ePartsType );
			}
			else
			{
				pActor->EquipItem( pPetActor->GetPetInfo().Vehicle[i] );

				DnSimplePartsHandle hParts = pActor->GetPetParts( (Vehicle::Parts::eVehicleParts)(i - Pet::Slot::Accessory1) );
				if( hParts && hParts->GetObjectHandle() ) hParts->GetObjectHandle()->SetRenderType( RT_TARGET );
			}
		}

		pActor->ChangeColor( ePetParts::PET_PARTS_BODY, pPetActor->GetPetInfo().dwPartsColor1 );
		pActor->ChangeColor( ePetParts::PET_PARTS_NOSE, pPetActor->GetPetInfo().dwPartsColor2 );
	}

	return true;
}

void CDnRenderAvatarNew::InitCameraControl()
{
	m_nRotateYaw = 0;
	m_fSmoothRotateYaw = 0.0f;
	m_nZoomLevel = 0;
	m_fSmoothZoomLevel = 0.0f;
	m_fPanX = 0.0f;
	m_fPanY = 0.0f;
	m_fSmoothPanX = 0.0f;
	m_fSmoothPanY = 0.0f;
}

void CDnRenderAvatarNew::CalcUVCoord( float fUIWidth, float fUIHeight )
{
	int nWidth = int(fUIWidth * DEFAULT_UI_SCREEN_WIDTH);
	int nHeight = int(fUIHeight * DEFAULT_UI_SCREEN_HEIGHT);

	if( fUIWidth == 0.0f || fUIHeight == 0.0f )
		nWidth = nHeight = 0;

	if( nWidth == nHeight )
	{
		m_RTTUVCoord.fX = m_RTTUVCoord.fY = 0.0f;
		m_RTTUVCoord.fWidth = m_RTTUVCoord.fHeight = 1.0f;
	}
	else if( nWidth < nHeight )
	{
		m_RTTUVCoord.fY = 0.0f;
		m_RTTUVCoord.fHeight = 1.0f;
		m_RTTUVCoord.fWidth = fUIWidth / fUIHeight;
		m_RTTUVCoord.fX = (1.0f - m_RTTUVCoord.fWidth) / 2.0f;
	}
	else if( nWidth > nHeight )
	{
		m_RTTUVCoord.fX = 0.0f;
		m_RTTUVCoord.fWidth = 1.0f;
		m_RTTUVCoord.fHeight = fUIHeight / fUIWidth;
		m_RTTUVCoord.fY = (1.0f - m_RTTUVCoord.fHeight) / 2.0f;
	}
}

void CDnRenderAvatarNew::Process( float fElapsedTime )
{
	LOCAL_TIME LocalTime = 0;

	if( CDnActor::s_hLocalActor )
	{
		LocalTime = CDnActor::s_hLocalActor->GetLocalTime();
	}
	else
	{
		CTask *pTask = CTaskManager::GetInstance().GetTask( "ItemTask" );
		if(pTask)
			LocalTime = pTask->GetLocalTime();
	}

	if( m_eActorType == ActorEnum::PlayerActor )
	{
		if( !m_hActor )
			return;

		// -- Required processes --
		// Process action of "ActionBase" inherited by actor (monsters etc. attach particles directly here to render)
		// "Cash Weapon" process, "Normal Weapon" process
		// "Parts" process
		CDnActor*		pActor  = (CDnActor*)m_hActor.GetPointer();
		CDnPlayerActor*	pPlayer = (CDnPlayerActor*)m_hActor.GetPointer();

		pActor->CDnActionBase::ProcessAction( LocalTime, fElapsedTime );
#ifdef PRE_ADD_CASH_COSTUME_AURA 
		pPlayer->ShowCostumeAura();
		CDnEtcObject::ProcessClass( LocalTime, fElapsedTime );
#endif
		pActor->CDnActionSignalImp::Process( LocalTime, fElapsedTime );

		// Animation calculation
		pActor->MAActorRenderBase::PreProcess( LocalTime, fElapsedTime );

		// Should we run MAPartsBody::Process for all of the below?
		EtMatrix WorldMat = *pActor->GetMatEx();
		pActor->MAActorRenderBase::Process( &WorldMat, LocalTime, fElapsedTime );

		// Normal weapon
		for( int i = 0; i < CDnWeapon::EquipSlot_Amount; i++ )
		{
			DnWeaponHandle hWeapon = pPlayer->GetWeapon( i );
			if( !hWeapon ) continue;

			hWeapon->Process( LocalTime, fElapsedTime );
		}

		// Try rotating only cash parts separately for now.
		for( int i = 0; i < CDnParts::CashPartsTypeEnum_Amount; i++ )
		{
			DnPartsHandle hParts = pPlayer->GetCashParts( (CDnParts::PartsTypeEnum)i );
			if( !hParts ) continue;

			hParts->Process( LocalTime, fElapsedTime );
		}

		// Also add cash weapons.
		for( int i = 0; i < CDnWeapon::EquipSlot_Amount; i++ )
		{
			DnWeaponHandle hWeapon = pPlayer->GetCashWeapon( i );
			if( !hWeapon ) continue;

			hWeapon->Process( LocalTime, fElapsedTime );
		}

#ifdef PRE_MOD_KALI_WEAPON
		// Since CDnPlayerActor::Process has too much in it, only necessary Process functions are called separately as above,
		// Cannot perform LeftWeapon-Process inside TDnPlayerKali::Process.
		// Still, we cannot call CDnPlayerActor::Process, so call a separate function like this.
		pPlayer->ProcessAdditionalWeapon( LocalTime, fElapsedTime );
#endif
	}
	else if( m_eActorType == ActorEnum::VehicleActor || m_eActorType == ActorEnum::PetActor )
	{
		if( !m_hActor )
			return;

		CDnActor *pActor = (CDnActor *)m_hActor.GetPointer();

		pActor->CDnActionBase::ProcessAction( LocalTime, fElapsedTime );
		pActor->CDnActionSignalImp::Process( LocalTime, fElapsedTime );

		// Animation calculation
		pActor->MAActorRenderBase::PreProcess( LocalTime, fElapsedTime );

		EtMatrix WorldMat = *pActor->GetMatEx();
		pActor->MAActorRenderBase::Process( &WorldMat, LocalTime, fElapsedTime );
	}
	else if( m_eActorType == ActorEnum::EtcObject )
	{
		if( !m_hEtc )
			return;

		CDnEtcObject * pObject = m_hEtc.GetPointer();
		if( NULL == pObject )
			return;
		EtObjectHandle hObject = pObject->GetObjectHandle();
		if( CEtObject::Identity() == hObject )
			return;

		LOCAL_TIME LocalTime = CDnActor::s_hLocalActor->GetLocalTime();

		pObject->CDnActionBase::ProcessAction( LocalTime, fElapsedTime );

		pObject->CDnActionSignalImp::Process( LocalTime, fElapsedTime );

		EtMatrix WorldMat = *pObject->GetMatEx();
		pObject->CDnActionRenderBase::Process( &WorldMat, LocalTime, fElapsedTime );
		return;
	}

	// Camera setup.
	// Rotation
	float fCurrentRotateYaw = EtToRadian( (float)m_nRotateYaw / ROTATE_SCALE );
	float fDistYaw = (fCurrentRotateYaw - m_fSmoothRotateYaw );
	m_fSmoothRotateYaw += fDistYaw * EtMin(1.0f, (fElapsedTime * 15.f));

	EtMatrix CamMat, CamPos, CamRotate, CamPI;
	EtMatrixRotationY( &CamPI, ET_PI );	// Since it must look at the origin from slightly above at a positive z distance (far from screen), make it face the opposite direction from the start.
	EtMatrixTranslation( &CamPos, 0.0f, m_fDefaultCameraYPos, m_fDefaultCameraZPos );
	EtMatrixRotationY( &CamRotate, -m_fSmoothRotateYaw );	// Placed in reverse, so apply Yaw in reverse as well.
	EtMatrixMultiply( &CamMat, &CamPI, &CamPos );
	EtMatrixMultiply( &CamMat, &CamMat, &CamRotate );
	UpdateCamera( CamMat );

	// Zoom
	static float fScale = 1.2f;
	float fZoomLevel[5] = {	1.0f, 
		fScale, 
		fScale*fScale, 
		fScale*fScale*fScale,
		fScale*fScale*fScale*fScale };

	m_fSmoothZoomLevel += ( m_nZoomLevel * 0.999f - m_fSmoothZoomLevel ) * EtMin( 1.0f, fElapsedTime * 25.0f );	// Value between 0.0 and 4.0 depending on m_nZoomLevel.
	float fMod = fmodf( m_fSmoothZoomLevel, 1.0f );	// Do not interpolate at 1.0, 2.0, 3.0, 4.0; if in between, compute the interpolation value,
	float fResultScale = fZoomLevel[ (int)m_fSmoothZoomLevel ] * (1.0f-fMod) + fZoomLevel[ (int)(m_fSmoothZoomLevel+1) ] * fMod;	// Interpolate.

	// Position.
	static float fXLimit = 1.4f;
	static float fYLimit = 2.0f;
	float fPanLimitWidth = float(m_nDefaultViewPortWidth) / fResultScale * 0.5f * ( EtMin(fXLimit, fResultScale) - 1.0f );
	float fPanLimitHeight = float(m_nDefaultViewPortHeight) / fResultScale * 0.5f * ( EtMin(fYLimit, fResultScale) - 1.0f );
	m_fPanX = EtClamp( m_fPanX, -fPanLimitWidth, fPanLimitWidth );
	m_fPanY = EtClamp( m_fPanY, -fPanLimitHeight, fPanLimitHeight );
	m_fSmoothPanX += ( m_fPanX - m_fSmoothPanX ) * EtMin( 1.0f, fElapsedTime * 10.0f );
	m_fSmoothPanY += ( m_fPanY - m_fSmoothPanY ) * EtMin( 1.0f, fElapsedTime * 10.0f );

	// Set camera.
	m_hCamera->SetOrthogonalView( int(m_nDefaultViewPortWidth / fResultScale), int(m_nDefaultViewPortHeight / fResultScale), m_fSmoothPanX, -m_fSmoothPanY );
}

bool CDnRenderAvatarNew::IsLoading()
{
	bool bLoading = false;
	if( CDnLoadingTask::IsActive() )
	{
		// Do not create texture if actually on the loading screen.
		if( CDnLoadingTask::GetInstance().IsEnableBackgroundLoading() == false )
			bLoading = true;
		if( GetInterface().IsPVPLobby() )
			bLoading = false;
	}
	return bLoading;
}

void CDnRenderAvatarNew::BeginRTT()
{
	// RenderAvatar changes many global lights and environments related to filtering
	// Must not run while baking lightmaps.
	if( IsLoading() ) return;

	CEtRTTRenderer::BeginRTT();

	// Not just the color filter, world lighting must also be changed. Values below are taken from Prairie settings (filter is also from Prairie).
	m_TempAmbient = *CEtLight::GetGlobalAmbient();
	CEtLight::SetGlobalAmbient( &s_TempAmbient );

	// Originally the settings below were done in the Initialize function,
	// If RenderAvatar::Initialize is called before environment loading, two extra unnecessary lights get added,
	// Decided to check in BeginRTT like this.
	// When there is no light object at all (when environment is not loaded because map was not loaded), create and process a temporary light directly.
	if( CEtLight::GetDirLightCount() == 0 )
	{
		if( s_hTempLight[0] && s_hTempLight[1] )
		{
			CEtLight::ForcePushDirLightInfo( s_hTempLight[0].GetPointer() );
			CEtLight::ForcePushDirLightInfo( s_hTempLight[1].GetPointer() );
			m_bUseTempLight = true;
		}
	}

	m_nDirLightCount = CEtLight::GetDirLightCount();
	m_nDirLightCount = min( m_nDirLightCount, 2 );
	for( int i = 0; i < m_nDirLightCount; ++i )
	{
		m_TempLightDiffuse[i] = CEtLight::GetDirLightInfo(i)->Diffuse;
		m_TempLightDir[i] = CEtLight::GetDirLightInfo(i)->Direction;
		CEtLight::GetDirLightInfo(i)->Diffuse = s_TempLightDiffuse[i];
		CEtLight::GetDirLightInfo(i)->Direction = s_TempLightDir[i];
	}

	// Environment Texture Setting
	if( CEtTexture::GetEnvTexture() == NULL && s_hTempEnviTexture )
	{
		CEtTexture::SetEnvTexture( s_hTempEnviTexture->GetTexturePtr() );
	}
}

void CDnRenderAvatarNew::EndRTT()
{
	if( IsLoading() ) return;

	CEtLight::SetGlobalAmbient( &m_TempAmbient );
	for( int i = 0; i < m_nDirLightCount; ++i )
	{
		CEtLight::GetDirLightInfo(i)->Diffuse = m_TempLightDiffuse[i];
		CEtLight::GetDirLightInfo(i)->Direction = m_TempLightDir[i];
	}
	if( m_bUseTempLight )
	{
		CEtLight::ForceClearDirLightInfo();
		m_bUseTempLight = false;
	}
	CEtRTTRenderer::EndRTT();
}

void CDnRenderAvatarNew::RenderRTT( float fElapsedTime )
{
	if( IsLoading() ) return;

	if( m_hActor )
	{
		DnActorHandle hActor = m_hActor;

		if( m_eActorType == ActorEnum::PlayerActor )
		{
			MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(hActor.GetPointer());
			EtObjectHandle hObject = pPartsBody->GetObjectHandle();
			hObject->InitRender();
			CEtRTTRenderer::RenderRTT( fElapsedTime );

			// Apply color filter through post-processing
			PostProcess();

			// When each task calls EternityEngine::RenderFrame( fDelta );
			// If the matrix was not computed beforehand, CEtObject::InitRenderObjectList(); computes and stores the matrix to use for that frame when called.
			// At this time CEtAniObject::InitRender is called for each CEtAniObject, which is calculated via CEtAniObject::CalcAni.
			//
			// If done in the UI Process stage like above, it has not been calculated yet in that frame, so -1 is stored.
			// If called at the proper place, the value will be populated.
		}
		else if( m_eActorType == ActorEnum::VehicleActor || m_eActorType == ActorEnum::PetActor )
		{
			CDnVehicleActor *pActor = (CDnVehicleActor *)hActor.GetPointer();
			EtObjectHandle hObject = pActor->GetObjectHandle();

			hObject->InitRender();
			CEtRTTRenderer::RenderRTT( fElapsedTime );
			PostProcess();

		}
	}
	else if( m_hEtc )
	{
		if( m_eActorType == ActorEnum::EtcObject )
		{
			CDnEtcObject * pObject = m_hEtc.GetPointer();
			if( NULL == pObject )
				return;
			EtObjectHandle hObject = pObject->GetObjectHandle();
			if( CEtObject::Identity() == hObject )
				return;

			CEtRTTRenderer::InitRenderObject();
			CEtRTTRenderer::RenderRTT( fElapsedTime );
			PostProcess();
		}
	}
}

void CDnRenderAvatarNew::PostProcess()
{
	// Since it draws onto a transparent plate, normal blending inevitably makes alpha objects look blurry.
	// Also, Dragon Nest has a color adjustment filter, so we couldn't draw the UI first and then particles on top (the filter would affect the underlying UI too).
	// So what I came up with is,
	// Draw onto a transparent plate while preserving the initial alpha value (whether via filter or BorderAlpha).
	// For this reason, alpha blending uses D3DBLEND_ONE, D3DBLEND_ONE.
	// Since it only draws once from source texture to target texture anyway,
	// If both color and alpha values are copied once as-is, it renders properly without losing alpha.

	if( m_bUseColorAdjFilter )
	{
		// After copying current RTT contents to temp texture
		EtTextureHandle hTempTarget = GetTempRenderTarget( m_nWidth, m_nHeight, m_Format );
		hTempTarget->Copy( GetRTT() );

		// Unify to use the one used in Prairie.
		float fSat = 0.f;
		fSat = s_pColorAdjFilter->GetSceneSaturation();
		s_pColorAdjFilter->SetSceneSaturation( 1.0f );
		bool bEnableZ = GetEtDevice()->EnableZ( false );	
		bool bAlphaEnable = GetEtDevice()->EnableAlphaBlend( true );
		// Since the background is 0x0, draw with One-One blending to copy Source texture pixels as-is.
		GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		GetEtDevice()->SetDepthStencilSurface( NULL );
		s_pColorAdjFilter->SetTexture( hTempTarget->GetMyIndex() );
		s_pColorAdjFilter->DrawFilter( GetRTT(), 1, EtVector2(0, 0), EtVector2(1, 1) , true );
		GetEtDevice()->RestoreRenderTarget();
		GetEtDevice()->RestoreDepthStencil();
		GetEtDevice()->EnableZ( bEnableZ );
		GetEtDevice()->EnableAlphaBlend( bAlphaEnable );
		s_pColorAdjFilter->SetSceneSaturation( fSat );
	}

	if( m_bUseBorderAlpha )
	{
		// After copying current RTT contents to temp texture
		EtTextureHandle hTempTarget = GetTempRenderTarget( m_nWidth, m_nHeight, m_Format );
		hTempTarget->Copy( GetRTT() );

		// Reset RTT texture as render target.
		bool bEnableZ = GetEtDevice()->EnableZ( false );
		bool bAlphaEnable = GetEtDevice()->EnableAlphaBlend( true );
		// Since the background is 0x0, draw with One-One blending to copy Source texture pixels as-is.
		GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		GetEtDevice()->SetDepthStencilSurface( NULL );

		GetEtDevice()->SetRenderTarget( GetRTT()->GetSurfaceLevel() );
		GetEtDevice()->ClearBuffer( 0x0, 1.0f, 0, true, false, false );

		// Apply boundary alpha blending using vertex alpha.
		STextureDiffuseVertex2D Vertices[ 16 ];
		int nWidth, nHeight;
		EtViewPort Viewport;

		GetEtDevice()->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
		GetEtDevice()->SetPixelShader( NULL );
		GetEtDevice()->SetVertexShader( NULL );
		GetEtDevice()->SetTexture( 0, hTempTarget->GetTexturePtr() );
		GetEtDevice()->GetViewport( &Viewport );

		nWidth = Viewport.Width;
		nHeight = Viewport.Height;

		static float fBorder = 0.04f;	// Apply 4% edge alpha blending at the border.
		float XPos[4] = { m_RTTUVCoord.fX, 0.0f, 0.0f, m_RTTUVCoord.fX + m_RTTUVCoord.fWidth };	// Assume values have been set in RTTUVCoord
		float YPos[4] = { m_RTTUVCoord.fY, 0.0f, 0.0f, m_RTTUVCoord.fY + m_RTTUVCoord.fHeight };
		XPos[1] = XPos[0] + (XPos[3]-XPos[0]) * fBorder;
		XPos[2] = XPos[3] - (XPos[3]-XPos[0]) * fBorder;
		YPos[1] = YPos[0] + (YPos[3]-YPos[0]) * fBorder;
		YPos[2] = YPos[3] - (YPos[3]-YPos[0]) * fBorder;

		float UVXPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float UVYPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		for( int i = 0; i < 4; ++i )
		{
			UVXPos[i] = XPos[i];
			UVYPos[i] = YPos[i];
			XPos[i] = XPos[i] * nWidth - 0.5f;
			YPos[i] = YPos[i] * nHeight - 0.5f;
		}

		for( int i = 0; i < 4; i++)
		{
			Vertices[i*4+0].Position.y = Vertices[i*4+1].Position.y = Vertices[i*4+2].Position.y = Vertices[i*4+3].Position.y = YPos[i];
			Vertices[ i ].Position.x = Vertices[4 + i].Position.x = Vertices[8 + i].Position.x = Vertices[12 + i].Position.x = XPos[i];
			Vertices[ i ].Position.z = Vertices[4 + i].Position.z = Vertices[8 + i].Position.z = Vertices[12 + i].Position.z = 0.0f;
			Vertices[ i ].Position.w = Vertices[4 + i].Position.w = Vertices[8 + i].Position.w = Vertices[12 + i].Position.w = 1.0f;
			Vertices[ i ].Color = Vertices[4 + i].Color = Vertices[8 + i].Color = Vertices[12 + i].Color = D3DCOLOR_ARGB(255, 255, 255, 255);

			Vertices[i*4+0].TexCoordinate.y = Vertices[i*4+1].TexCoordinate.y = Vertices[i*4+2].TexCoordinate.y = Vertices[i*4+3].TexCoordinate.y = UVYPos[i];
			Vertices[ i ].TexCoordinate.x = Vertices[4 + i].TexCoordinate.x = Vertices[8 + i].TexCoordinate.x = Vertices[12 + i].TexCoordinate.x = UVXPos[i];
		}
		Vertices[ 0 ].Color = Vertices[ 1 ].Color = Vertices[ 2 ].Color = Vertices[ 3 ].Color = Vertices[ 4 ].Color = Vertices[ 7 ].Color = D3DCOLOR_ARGB(0, 255, 255, 255);
		Vertices[ 8 ].Color = Vertices[ 11 ].Color = Vertices[ 12 ].Color = Vertices[ 13 ].Color = Vertices[ 14 ].Color = Vertices[ 15 ].Color = D3DCOLOR_ARGB(0, 255, 255, 255);

		unsigned short Indices[54] = {
			0, 5, 4, 0, 1, 5, 1, 2, 5, 6, 5, 2, 2, 3, 6, 7, 6, 3,
			4, 5, 8, 9, 8, 5, 5, 6, 9, 10, 9, 6, 6, 7, 10, 11, 10, 7,
			8, 9, 12, 13, 12, 9, 9, 10, 13, 14, 13, 10, 10, 15, 14, 10, 11, 15
		};
		GetEtDevice()->DrawIndexedPrimitiveUP( PT_TRIANGLELIST, 0, 16, 18, Indices, FMT_INDEX16, Vertices, sizeof( STextureDiffuseVertex2D ) );

		// Restore target
		GetEtDevice()->RestoreRenderTarget();
		GetEtDevice()->RestoreDepthStencil();
		GetEtDevice()->EnableZ( bEnableZ );
		GetEtDevice()->EnableAlphaBlend( bAlphaEnable );
	}
}

void CDnRenderAvatarNew::SetFrontView()
{
	m_nRotateYaw = (m_nRotateYaw + (ROTATE_SCALE * 180) * (m_nRotateYaw >= 0 ? 1 : -1 ) ) / (ROTATE_SCALE*360) * (ROTATE_SCALE*360);
}

void CDnRenderAvatarNew::SetRearView()
{
	m_nRotateYaw = (m_nRotateYaw ) / (ROTATE_SCALE*360) * (ROTATE_SCALE*360) + (ROTATE_SCALE * 180) * (m_nRotateYaw >= 0 ? 1 : -1 ) ;
}

void CDnRenderAvatarNew::ZoomIn()
{
	if( m_nZoomLevel < 4 )
		++m_nZoomLevel;
}

void CDnRenderAvatarNew::ZoomOut()
{
	if( m_nZoomLevel > 0 )
		--m_nZoomLevel;
}

void CDnRenderAvatarNew::Panning( float fX, float fY )
{
	m_fPanX += fX / 5.0f;
	m_fPanY += fY / 5.0f;
}

EtTextureHandle CDnRenderAvatarNew::GetTempRenderTarget( int nWidth, int nHeight, EtFormat Format )
{
	for( int i = 0; i < (int)s_vecTempTarget.size(); ++i )
	{
		if( s_vecTempTarget[i].nWidth == nWidth &&
			s_vecTempTarget[i].nHeight == nHeight &&
			s_vecTempTarget[i].Format == Format )
		{
			return s_vecTempTarget[i].hTexture;
		}
	}

	sTempTargetInfo Info;
	Info.nWidth = nWidth;
	Info.nHeight = nHeight;
	Info.Format = Format;
	Info.hTexture = CEtTexture::CreateRenderTargetTexture( nWidth, nHeight, Format );
	s_vecTempTarget.push_back( Info );
	return Info.hTexture;
}

void CDnRenderAvatarNew::SetEtc( const char *szSkinName, const char *szAniName, const char *szActName )
{
	m_hEtc = (new CDnEtcObject)->GetMySmartPtr();
	m_hEtc->Initialize( szSkinName, szAniName, szActName );

	CheckType();

	CDnEtcObject * pObject = m_hEtc.GetPointer();
	if( NULL == pObject )
		return;

	m_hEtc->SetRTTRenderMode( true, this );
	m_hEtc->SetActionQueue( "Stand" );
	m_hEtc->Show( true );
	AddObject( pObject->GetObjectHandle() );
	SetRenderFrameCount( -1 );

	return;
}

void CDnRenderAvatarNew::ResetEtc( bool bInitCamera )
{
	if( m_hActor )
	{
		SAFE_RELEASE_SPTR( m_hEtc );
		m_eActorType = ActorEnum::None;
		SetRenderFrameCount( 0 );
	}

	if( bInitCamera )
		InitCameraControl();
}