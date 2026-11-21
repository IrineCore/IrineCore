#pragma once

#include "DnWeapon.h"
#include "DnLocalPlayerActor.h"
#include "TDnPlayerCommon.h"
#ifdef PRE_MOD_WARRIOR_RIGHTHAND
#include "DnLoadingTask.h"
#include "VillageClientSession.h"
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND

#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnActorClassDefine.h"

#if defined(PRE_ADD_ACADEMIC)

template < class T >

class TDnPlayerAcademic : public TDnPlayerCommon<T>
{
public:
	TDnPlayerAcademic( int nClassID, bool bProcess = true )
		: TDnPlayerCommon<T>(nClassID, bProcess)
	{
		memset( m_nKickActionIndex, -1, sizeof(m_nKickActionIndex) );
		m_fPitchAngle = 0.f;
#ifdef PRE_MOD_WARRIOR_RIGHTHAND
		m_bUseThreadOnCombineParts = false;
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
	}

	virtual ~TDnPlayerAcademic()
	{
		for( int i=0; i<4; i++ )
			SAFE_RELEASE_SPTR( m_hKick[i] );
	}

	struct RotateBoneStruct {
		char *szBoneName;
		int nBoneIndex;
		float fWeight;
		bool bBattleMode;
	};

protected:
	int m_nKickActionIndex[4];
	DnWeaponHandle m_hKick[4];
	float m_fPitchAngle;

	static RotateBoneStruct s_RotateBoneDefine[];

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
	bool m_bUseThreadOnCombineParts;
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND

public:
	// Actor Message
	virtual bool Initialize()
	{
		TDnPlayerCommon<T>::Initialize();

		int j = 0;
		for (; j < 4; j++)
			SAFE_RELEASE_SPTR(m_hKick[j]);

		m_hKick[0] = CDnWeapon::CreateWeapon( 5036, 0 );
		m_hKick[1] = CDnWeapon::CreateWeapon( 5037, 0 );
		m_hKick[2] = CDnWeapon::CreateWeapon( 5038, 0 );
		m_hKick[3] = CDnWeapon::CreateWeapon( 5036, 0 );
		for( int i=0; i<4; i++ ) {
			m_hKick[i]->CreateObject();
			m_hKick[i]->SetLinkActor( GetMySmartPtr() );
		}

		m_nKickActionIndex[0] = GetElementIndex( "Skill_SpannerSmashing" );	// Work is already completed, so follow the resource.
		m_nKickActionIndex[1] = GetElementIndex( "Skill_MegaHammer" );
		m_nKickActionIndex[2] = GetElementIndex( "Skill_MagnifyAttack" );
		m_nKickActionIndex[3] = GetElementIndex( "Skill_AirBomb" );

		for( int i=0; ; i++ ) {
			if( s_RotateBoneDefine[i].szBoneName == NULL ) break;
			s_RotateBoneDefine[i].nBoneIndex = GetBoneIndex( s_RotateBoneDefine[i].szBoneName );
		}
		return true;
	}

	virtual DnWeaponHandle GetWeapon( int nEquipIndex = 0, bool bActionMatchWeapon = true )
	{
		if( IsGhost() || !bActionMatchWeapon ) return m_hWeapon[nEquipIndex];
		if( nEquipIndex == 0 ) {
			for( int i=0; i<4; i++ ) {
				if( m_nActionIndex == m_nKickActionIndex[i] ) return m_hKick[i];
			}
		}
		return TDnPlayerCommon<T>::GetWeapon( nEquipIndex, bActionMatchWeapon );
	}

	virtual DnWeaponHandle GetActiveWeapon( int nEquipIndex )
	{
		if( IsGhost() ) return m_hWeapon[nEquipIndex];
		if( nEquipIndex == 0 ) {
			for( int i=0; i<4; i++ ) {
				if( m_nActionIndex == m_nKickActionIndex[i] ) return m_hKick[i];
			}
		}
		return TDnPlayerCommon<T>::GetActiveWeapon( nEquipIndex );
	}

	virtual void PreProcess( LOCAL_TIME LocalTime, float fDelta )
	{
		TDnPlayerCommon<T>::PreProcess( LocalTime, fDelta );
		if( IsSwapSingleSkin() ) return;
		if( strcmp( GetCurrentAction(), "Social_Sit01" ) == 0 ) return;

		if( m_fPitchAngle < -45.f ) m_fPitchAngle = -45.f;
		else if( m_fPitchAngle > 45.f ) m_fPitchAngle = 45.f;

		EtVector3 vRotate( 0.f, 0.f, 0.f );
		for( int i=0; ; i++ ) {
			if( s_RotateBoneDefine[i].szBoneName == NULL ) break;
			if( IsBattleMode() && !s_RotateBoneDefine[i].bBattleMode ) continue;

			// Rotha: Do not use camera look-at in vehicle mode.
			if( IsVehicleMode() )
				break;

			if( s_RotateBoneDefine[i].nBoneIndex == -1 )
			{
				s_RotateBoneDefine[i].nBoneIndex = GetBoneIndex( s_RotateBoneDefine[i].szBoneName );
			}
			else
			{
				vRotate.x = m_fPitchAngle * s_RotateBoneDefine[i].fWeight;
				m_hObject->SetBoneRotation( s_RotateBoneDefine[i].nBoneIndex, vRotate );
			}
		}
	}
	virtual void OnStopReady()
	{
		ResetCustomAction();
		ResetMixedAnimation();
	}

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
	virtual void AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false )
	{
		TDnPlayerCommon<T>::AttachWeapon( hWeapon, nEquipIndex, bDelete );

		if( nEquipIndex == 1 ) {
			if( m_bUseThreadOnCombineParts ) {
				DWORD dwUniqueID = GetUniqueID();
				CDnLoadingTask::GetInstance().InsertLoadObject(
					OnLoadChangeParts,
					OnLoadChangePartsUniqueID,
					&CVillageClientSession::OnCheckLoadingPacket,
					NULL,
					(void*)this, (void*)&dwUniqueID, sizeof(DWORD), 0 );
			}
			else {
				OnLoadChangeParts( this, NULL, NULL, 0 );
			}
		}
	}
	virtual void DetachWeapon( int nEquipIndex = 0 )
	{
		TDnPlayerCommon<T>::DetachWeapon( nEquipIndex );

		if( nEquipIndex == 1 ) {
			if( m_bUseThreadOnCombineParts ) {
				DWORD dwUniqueID = GetUniqueID();
				CDnLoadingTask::GetInstance().InsertLoadObject(
					OnLoadChangeParts,
					OnLoadChangePartsUniqueID,
					&CVillageClientSession::OnCheckLoadingPacket,
					NULL,
					(void*)this, (void*)&dwUniqueID, sizeof(DWORD), 0 );
			}
			else {
				OnLoadChangeParts( this, NULL, NULL, 0 );
			}
		}
	}
	virtual void AttachCashWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false, bool bRefreshWeaponOrder = true )
	{
		TDnPlayerCommon<T>::AttachCashWeapon( hWeapon, nEquipIndex, bDelete, bRefreshWeaponOrder );

		if( nEquipIndex == 1 ) {
			if( m_bUseThreadOnCombineParts ) {
				DWORD dwUniqueID = GetUniqueID();
				CDnLoadingTask::GetInstance().InsertLoadObject(
					OnLoadChangeParts,
					OnLoadChangePartsUniqueID,
					&CVillageClientSession::OnCheckLoadingPacket,
					NULL,
					(void*)this, (void*)&dwUniqueID, sizeof(DWORD), 0 );
			}
			else {
				OnLoadChangeParts( this, NULL, NULL, 0 );
			}
		}
	}
	virtual void DetachCashWeapon( int nEquipIndex = 0 )
	{
		TDnPlayerCommon<T>::DetachCashWeapon( nEquipIndex );

		if( nEquipIndex == 1 ) {
			if( m_bUseThreadOnCombineParts ) {
				DWORD dwUniqueID = GetUniqueID();
				CDnLoadingTask::GetInstance().InsertLoadObject(
					OnLoadChangeParts,
					OnLoadChangePartsUniqueID,
					&CVillageClientSession::OnCheckLoadingPacket,
					NULL,
					(void*)this, (void*)&dwUniqueID, sizeof(DWORD), 0 );
			}
			else {
				OnLoadChangeParts( this, NULL, NULL, 0 );
			}
		}
	}

	// Loading-related functions
	static int __stdcall OnLoadChangePartsUniqueID( void *pParam, int nSize ) { return *((DWORD *)pParam); }
	static bool __stdcall OnLoadChangeParts( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime )
	{
		if( nSize > 0 && CDnLoadingTask::GetInstance().CheckSameLoadObject( pParam, nSize ) ) return true;

		TDnPlayerCommon<T>* pTemp = (TDnPlayerCommon<T>*)pThis;
		MAPartsBody *pBody = dynamic_cast<MAPartsBody*>(pTemp);
		if( pBody ) pBody->CombineParts();
		return true;
	}

	// Here when IgnoreCombine is true
	virtual void OnCombineParts()
	{
		MAPartsBody *pBody = dynamic_cast<MAPartsBody*>(this);
		if( pBody ) {
			DnPartsHandle hHand = pBody->GetParts( CDnParts::Hand );
			DnPartsHandle hCashHand = pBody->GetCashParts( CDnParts::CashHand );
			DnPartsHandle hParts = pBody->GetCashParts( CDnParts::CashBody );	// Check Body first in case of a one-piece outfit.
			if( hHand && hHand->GetObjectHandle() && hHand->IsShow() ) hParts = hHand;
			if( hCashHand && hCashHand->GetObjectHandle() && hCashHand->IsShow() ) hParts = hCashHand;
			if( hParts && hParts->GetObjectHandle() ) {
				EtAniObjectHandle hHandle = hParts->GetObjectHandle();
				for( int i=0; i<hHandle->GetSubMeshCount(); i++ ) {
					CEtSubMesh *pSubMesh = hParts->GetObjectHandle()->GetSubMesh(i);
					if( pSubMesh && strstr( pSubMesh->GetSubMeshName(), "Glove_L" ) ) {
						bool bShowSubMesh = true;
						if( m_hWeapon[1] && m_hWeapon[1]->IsShow() ) bShowSubMesh = false;
						if( m_hCashWeapon[1] && m_hCashWeapon[1]->IsShow() ) bShowSubMesh = false;
						hHandle->ShowSubmesh( i, bShowSubMesh );
						break;
					}
				}
			}
		}
	}

	// Here when IgnoreCombine is false
	virtual void OnCombineParts( EtSkinHandle hSkin, CEtSkinInstance **ppSkinInstance )
	{
		if( hSkin && hSkin->GetMeshHandle() && hSkin->GetMeshHandle()->GetSubMeshCount() > 0 && GetActiveWeapon(1) && GetActiveWeapon(1)->IsShow() ) {
			for( int i=0; i<hSkin->GetMeshHandle()->GetSubMeshCount(); i++ ) {
				CEtSubMesh *pSubMesh = hSkin->GetMeshHandle()->GetSubMesh(i);
				if( pSubMesh && strstr( pSubMesh->GetSubMeshName(), "Glove_L" )	) {
					(*ppSkinInstance)->GetSkinRenderInfo( i )->bShowSubmesh = false;
					break;
				}
			}
		}
		else {
			MAPartsBody::OnCombineParts( hSkin, ppSkinInstance );
		}
	}

	virtual void SetProcess( bool bFlag )
	{
		if( IsProcess() == bFlag ) return;
		TDnPlayerCommon<T>::SetProcess( bFlag );

		m_bUseThreadOnCombineParts = bFlag;
	}

	virtual void ShowWeapon( int nEquipIndex, bool bShow )
	{
		bool bRefreshCombine = false;
		if( nEquipIndex == 1 && m_hWeapon[nEquipIndex] && m_hWeapon[nEquipIndex]->IsShow() != bShow ) bRefreshCombine = true;

		if( bShow == false && IsShow() == false ) bRefreshCombine = false;
		if( IsSwapSingleSkin() ) bRefreshCombine = false;

		TDnPlayerCommon<T>::ShowWeapon( nEquipIndex, bShow );
		if( bRefreshCombine ) {
			CombineParts();
		}
	}

	virtual void ShowCashWeapon( int nEquipIndex, bool bShow )
	{
		bool bRefreshCombine = false;
		if( nEquipIndex == 1 && m_hCashWeapon[nEquipIndex] && m_hCashWeapon[nEquipIndex]->IsShow() != bShow ) bRefreshCombine = true;

		if( bShow == false && IsShow() == false ) bRefreshCombine = false;
		if( IsSwapSingleSkin() ) bRefreshCombine = false;

		TDnPlayerCommon<T>::ShowCashWeapon( nEquipIndex, bShow );
		if( bRefreshCombine ) {
			CombineParts();
		}
	}
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
};

#endif	// #if defined(PRE_ADD_ACADEMIC)