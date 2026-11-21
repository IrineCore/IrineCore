#pragma once

#include "DnWeapon.h"
#include "TDnPlayerCommon.h"
#include "DnParts.h"
#ifdef PRE_MOD_WARRIOR_RIGHTHAND
#include "DnLoadingTask.h"
#include "VillageClientSession.h"
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND

template < class T >

class TDnPlayerWarrior : public TDnPlayerCommon<T>
{
public:
	TDnPlayerWarrior( int nClassID, bool bProcess = true )
		: TDnPlayerCommon<T>(nClassID, bProcess)
	{
		memset( m_nKickActionIndex, -1, sizeof(m_nKickActionIndex) );
#ifdef PRE_MOD_WARRIOR_RIGHTHAND
		m_bUseThreadOnCombineParts = false;
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
	}

	virtual ~TDnPlayerWarrior()
	{
		for( int i=0; i<4; i++ ) SAFE_RELEASE_SPTR( m_hKick[i] );
	}

protected:
	int m_nKickActionIndex[4];
	DnWeaponHandle m_hKick[4];

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

		m_hKick[0] = CDnWeapon::CreateWeapon( 5, 0 );
		m_hKick[1] = CDnWeapon::CreateWeapon( 6, 0 );
		m_hKick[2] = CDnWeapon::CreateWeapon( 7, 0 );
		m_hKick[3] = CDnWeapon::CreateWeapon( 5, 0 );
		for( int i=0; i<4; i++ ) {
			m_hKick[i]->CreateObject();
			m_hKick[i]->SetLinkActor( GetMySmartPtr() );
		}

		m_nKickActionIndex[0] = GetElementIndex( "Attack_SideKick" );
		m_nKickActionIndex[1] = GetElementIndex( "Attack_SoccerKick" );
		m_nKickActionIndex[2] = GetElementIndex( "Attack_Down" );
		m_nKickActionIndex[3] = GetElementIndex( "Jump_Kick" );

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
	virtual void AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false )
	{
		TDnPlayerCommon<T>::AttachWeapon( hWeapon, nEquipIndex, bDelete );

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
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

			// If CombineParts is called in both Attach and Detach like this, it gets called twice when equipping.
			// This is for the same reason that combining keeps happening because unequipping parts arrives split across multiple calls for one-piece outfits anyway, so
			// Resolve by checking for changes and combining at the combine stage itself.
		}
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
	}
	virtual void DetachWeapon( int nEquipIndex = 0 )
	{
		TDnPlayerCommon<T>::DetachWeapon( nEquipIndex );

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
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
#endif	// #ifdef PRE_MOD_WARRIOR_RIGHTHAND
	}

#ifdef PRE_MOD_WARRIOR_RIGHTHAND
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

	// If we call ItemTask's InsertChangePartsThread, it only works for the character itself inside the world, so
	// Must invoke the thread here so it also applies to the Warrior on the login screen.
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
					if( pSubMesh && strstr( pSubMesh->GetSubMeshName(), "Glove_R" ) ) {
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
		DnWeaponHandle hWeapon = GetActiveWeapon(1);
		if( hSkin && hSkin->GetMeshHandle() && hSkin->GetMeshHandle()->GetSubMeshCount() > 0 && hWeapon && hWeapon->IsShow() ) 
		{
			for( int i=0; i<hSkin->GetMeshHandle()->GetSubMeshCount(); i++ ) 
			{
				CEtSubMesh *pSubMesh = hSkin->GetMeshHandle()->GetSubMesh(i);
				// && !strstr( pSubMesh->GetSubMeshName(), "Basic" ) Adding this check shows bare hands with bare-hand+gauntlet, and hides basic parts when wearing gloves.
				// This is not what we want. To do it properly, when equipping a gauntlet while wearing gloves, it should combine with basic parts, but
				// Otherwise, basic parts would need to stay loaded even while wearing gloves. Skipping for now due to structure.
				if( pSubMesh && strstr( pSubMesh->GetSubMeshName(), "Glove_R" )	) {
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

		// Except for normal equipping, do not use threads when initializing on stage entry or creating a character in RenderAvatar.
		m_bUseThreadOnCombineParts = bFlag;
	}

	virtual void ShowWeapon( int nEquipIndex, bool bShow )
	{
		bool bRefreshCombine = false;
		if( nEquipIndex == 1 && m_hWeapon[nEquipIndex] && m_hWeapon[nEquipIndex]->IsShow() != bShow ) bRefreshCombine = true;

		// Bug issue where warrior torso is invisible on drop-in.
		// Checked and found CDnPlayerActor::Show is called consecutively with false, true during join-in-progress,
		// ShowWeapon is also called here, so Combine processing is executed.
		// The problem is when Show(false) is called and Show(true) is called while Combine is in progress, CDnPlayerActor::ShowParts gets called,
		// Inside here, m_MergedObject is in Identity state (because it is combining), so the next Combine proceeds without calling Show(true) on MergedObject.
		// So the Show(false) state is maintained.
		//
		// First solution is locking s_PartsLock around MergedObject Show setting in CDnPlayerActor::ShowParts, skipped because locks are disliked.
		// The second solution is to get the actor's IsShow instead of the previous MergedObject's IsShow in Combine, but skipping since it is unproven code,
		// The third solution is checking MergedObject's IsShow in ProcessHidePlayerActor, but it's too forceful so skip it.
		// So as a last resort, when ShowWeapon(false) is called, if the current character is hidden, there is no need to combine, so skip combining altogether.
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