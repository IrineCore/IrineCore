#pragma once

#include "DnWeapon.h"
#include "DnLocalPlayerActor.h"
#include "TDnPlayerCommon.h"

#ifdef PRE_ADD_KALI

template < class T >

class TDnPlayerKali : public TDnPlayerCommon<T>
{
public:
	TDnPlayerKali( int nClassID, bool bProcess = true )
		: TDnPlayerCommon<T>(nClassID, bProcess)
	{
		memset( m_nKickActionIndex, -1, sizeof(m_nKickActionIndex) );
		m_fPitchAngle = 0.f;
	}

	virtual ~TDnPlayerKali()
	{
		for( int i=0; i<4; i++ ) SAFE_RELEASE_SPTR( m_hKick[i] );
#ifdef PRE_MOD_KALI_WEAPON
		SAFE_RELEASE_SPTR( m_hLeftHandWeapon );
#endif
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
#ifdef PRE_MOD_KALI_WEAPON
	DnWeaponHandle m_hLeftHandWeapon;
#endif

	static RotateBoneStruct s_RotateBoneDefine[];

public:
	// Actor Message
	virtual bool Initialize()
	{
		TDnPlayerCommon<T>::Initialize();

		int j = 0;
		for (; j < 4; j++)
			SAFE_RELEASE_SPTR(m_hKick[j]);

		m_hKick[0] = CDnWeapon::CreateWeapon( 5525, 0 );
		m_hKick[1] = CDnWeapon::CreateWeapon( 5526, 0 );
		m_hKick[2] = CDnWeapon::CreateWeapon( 5527, 0 );
		m_hKick[3] = CDnWeapon::CreateWeapon( 5525, 0 );
		for( int i=0; i<4; i++ ) {
			m_hKick[i]->CreateObject();
			m_hKick[i]->SetLinkActor( GetMySmartPtr() );
		}

		m_nKickActionIndex[0] = GetElementIndex( "Skill_SlightTurn" );	// Work is already completed, so follow the resource.
		m_nKickActionIndex[1] = GetElementIndex( "Skill_ReturnSpin" );
		m_nKickActionIndex[2] = GetElementIndex( "Skill_SoulKiss" );
		m_nKickActionIndex[3] = GetElementIndex( "Skill_Ghostkick" );

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

#ifdef PRE_MOD_KALI_WEAPON
	virtual void ReleaseWeaponSignalImp()
	{
		TDnPlayerCommon<T>::ReleaseWeaponSignalImp();
		if( m_hLeftHandWeapon ) {
			m_hLeftHandWeapon->ReleaseSignalImp();
			m_hLeftHandWeapon->ResetDefaultAction( 0 );
		}
	}
	virtual void Process( LOCAL_TIME LocalTime, float fDelta )
	{
		TDnPlayerCommon<T>::Process( LocalTime, fDelta );
		ProcessAdditionalWeapon( LocalTime, fDelta );
	}
	// Separate function created because RenderAvatar does not call CDnPlayerActor::Process directly.
	virtual void ProcessAdditionalWeapon( LOCAL_TIME LocalTime, float fDelta )
	{
		if( m_hLeftHandWeapon ) m_hLeftHandWeapon->Process( LocalTime, fDelta );
	}
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
	{
		TDnPlayerCommon<T>::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
		switch( Type ) {
			case STE_ShowSwordTrail:
				{
					ShowSwordTrailStruct *pStruct = (ShowSwordTrailStruct *)pPtr;
					if( pStruct->nWeaponIndex == 0 ) {
						if( m_hLeftHandWeapon )
							m_hLeftHandWeapon->ShowTrail( SignalEndTime - LocalTime, pStruct );
					}
				}
				break;
			case STE_ChangeWeaponLink:
				{
					if( CGlobalInfo::GetInstance().IsPlayingCutScene() )
					{
						if( m_hLeftHandWeapon ) {
							ChangeWeaponLinkStruct* pStruct = (ChangeWeaponLinkStruct *)pPtr;
							if( pStruct->szLinkBoneName ) {
								if( pStruct->nWeaponIndex == 0 ) {
									std::string szName = pStruct->szLinkBoneName;
									bool bBattleMode = false;
									if( szName == "~BoxBone01" ) bBattleMode = true;
									else if( szName == "~BoxBone01_1" ) bBattleMode = false;
									const char *szBoneName = GetLinkWeaponBoneName( 1, bBattleMode );
									m_hLeftHandWeapon->ChangeLinkBone( szBoneName );
								}
							}
						}
					}
				}
				break;
		}
	}
	virtual void AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false )
	{
		TDnPlayerCommon<T>::AttachWeapon( hWeapon, nEquipIndex, bDelete );
		if( nEquipIndex == 0 ) RefreshLeftHandWeapon();
	}
	virtual void DetachWeapon( int nEquipIndex = 0 )
	{
		TDnPlayerCommon<T>::DetachWeapon( nEquipIndex );
		if( nEquipIndex == 0 ) RefreshLeftHandWeapon();
	}
	virtual void AttachCashWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false, bool bRefreshWeaponOrder = true )
	{
		TDnPlayerCommon<T>::AttachCashWeapon( hWeapon, nEquipIndex, bDelete, bRefreshWeaponOrder );
		if( nEquipIndex == 0 ) RefreshLeftHandWeapon();
	}
	virtual void DetachCashWeapon( int nEquipIndex = 0 )
	{
		TDnPlayerCommon<T>::DetachCashWeapon( nEquipIndex );
		if( nEquipIndex == 0 ) RefreshLeftHandWeapon();
	}

	virtual void RefreshWeaponViewOrder( int nEquipIndex )
	{
		// RefreshLeftHandWeapon may be called once more due to SetBattleMode inside RefreshWeaponViewOrder, but
		// Duplicate calls are already handled inside RefreshLeftHandWeapon,
		// Made virtual like this so it still works properly if RefreshWeaponViewOrder is later changed not to call SetBattleMode.
		TDnPlayerCommon<T>::RefreshWeaponViewOrder( nEquipIndex );
		if( nEquipIndex == 0 ) RefreshLeftHandWeapon();
	}

	virtual void RefreshLeftHandWeapon()
	{
		// Because there is a lot of code related to Weapon, CashWeapon, WeaponOrder, etc.
		// Handling LeftHandWeapon individually seemed too hard to maintain later.
		// So handle it in one function like this.

		int nWeaponID = 0;
		int nWeaponEnchantLevel = 0;
		if( m_hWeapon[0] && m_hWeapon[0]->IsShow() ) nWeaponID = m_hWeapon[0]->GetClassID();
		if( m_hCashWeapon[0] && m_hCashWeapon[0]->IsShow() && m_bWeaponViewOrder[0] && m_hCashWeapon[0]->GetObjectHandle() ) nWeaponID = m_hCashWeapon[0]->GetClassID();
		if( m_hWeapon[0] ) nWeaponEnchantLevel = m_hWeapon[0]->GetEnchantLevel();

		// Handle cutscene edge case.
		if( CGlobalInfo::GetInstance().IsPlayingCutScene() )
		{
			if( nWeaponID > 0 && GetActiveWeapon( 0 ) && GetActiveWeapon( 0 )->IsShow() )
			{
				if( IsHideWeaponBySignalInCutScene( 0 ) )
					nWeaponID = 0;
			}
		}

		int nCurrentWeaponID = 0;
		if( m_hLeftHandWeapon ) nCurrentWeaponID = m_hLeftHandWeapon->GetClassID();

		// Enhancement is not checked because equipping swaps out the old item first anyway.
		if( nWeaponID != nCurrentWeaponID ) {
			SAFE_RELEASE_SPTR( m_hLeftHandWeapon );
			if( nWeaponID > 0 ) {
				m_hLeftHandWeapon = CDnWeapon::CreateWeapon( nWeaponID, 0, 0, nWeaponEnchantLevel, 0, 0, false, false, false );
				if( m_hLeftHandWeapon ) {
					m_hLeftHandWeapon->CreateObject( this );
					if( m_bRTTRenderMode && m_hLeftHandWeapon->GetObjectHandle() )
					{
						m_hLeftHandWeapon->GetObjectHandle()->SetRenderType( RT_TARGET );
						m_hLeftHandWeapon->SetRTTRenderMode( m_bRTTRenderMode, m_pRTTRenderer );
					}
					m_hLeftHandWeapon->LinkWeapon( GetMySmartPtr(), 1 );

					if( CGlobalInfo::GetInstance().IsPlayingCutScene() == false )
					{
						// Update link according to battle mode
						const char *szBoneName = GetLinkWeaponBoneName( 1, m_bBattleMode );
						m_hLeftHandWeapon->ChangeLinkBone( szBoneName );
					}
					else
					{
						// In cutscenes, STE_ChangeWeaponLink may move weapon bone to back even when BattleMode is true.
						// So because BattleMode cannot be trusted, compute and handle it directly as below.
						if( GetActiveWeapon( 0 ) )
						{
							int nLinkBoneIndex = GetActiveWeapon( 0 )->GetLinkBoneIndex();
							if( nLinkBoneIndex >= 0 )
							{
								std::string szName = GetBoneName( nLinkBoneIndex );
								if( !szName.empty() )
								{
									bool bBattleMode = false;
									if( szName == "~BoxBone01" ) bBattleMode = true;
									else if( szName == "~BoxBone01_1" ) bBattleMode = false;
									const char *szBoneName = GetLinkWeaponBoneName( 1, bBattleMode );
									m_hLeftHandWeapon->ChangeLinkBone( szBoneName );
								}
							}
						}
					}

					// When setting an action on CDnWeapon, usually via CDnWeapon::GetWeaponMatchAction
					// Switches to an action with enhancement or Fantastic Four,
					// For cash items, retrieve and apply the enhancement level of the normal weapon.
					// However, a problem arose:
					// Kali's left-hand weapon is a main weapon due to bone position, but is set and used with the sub-weapon index.
					// Moreover, since CDnWeapon::GetWeaponMatchAction is called internally, it is difficult to pass the index as a separate argument.
					// So having no choice, create a forced setting.
					//
					// To summarize,
					// After CreateObject, set LinkWeapon to index 1 so it goes to the left hand.
					// Also when refreshing by battle mode (calling GetLinkWeaponBoneName), use index 1 to place it at the left hand position,
					// Reset m_nEquipIndex back to 0 at the end so enhancement effect attaches properly.
					m_hLeftHandWeapon->ForceSetEquipIndex( 0 );
					if( m_hLeftHandWeapon->IsExistAction( "Idle" ) ) m_hLeftHandWeapon->SetAction( "Idle", 0.f, 0.f );
				}
			}
		}
	}

	virtual void Show( bool bShow )
	{
		if( IsShow() == bShow ) return;
		TDnPlayerCommon<T>::Show( bShow );
		if( m_hLeftHandWeapon || bShow ) RefreshLeftHandWeapon();
	}

	virtual void ShowWeapon( int nEquipIndex, bool bShow )
	{
		bool bRefresh = false;
		if( nEquipIndex == 0 && m_hWeapon[nEquipIndex] && m_hWeapon[nEquipIndex]->IsShow() != bShow ) bRefresh = true;
		if( bShow == false && IsShow() == false ) bRefresh = false;
		if( IsSwapSingleSkin() ) bRefresh = false;
		TDnPlayerCommon<T>::ShowWeapon( nEquipIndex, bShow );
		if( bRefresh ) {
			RefreshLeftHandWeapon();
		}
	}

	virtual void ShowCashWeapon( int nEquipIndex, bool bShow )
	{
		bool bRefresh = false;
		if( nEquipIndex == 0 && m_hCashWeapon[nEquipIndex] && m_hCashWeapon[nEquipIndex]->IsShow() != bShow ) bRefresh = true;
		TDnPlayerCommon<T>::ShowCashWeapon( nEquipIndex, bShow );
		if( bRefresh ) {
			RefreshLeftHandWeapon();
		}
	}

#ifdef PRE_ADD_37745
	virtual void SetBattleMode( bool bEnable, bool bForce )
	{
		TDnPlayerCommon<T>::SetBattleMode( bEnable, bForce );
#else
	virtual void SetBattleMode( bool bEnable )
	{
		TDnPlayerCommon<T>::SetBattleMode( bEnable );
#endif

		if( m_hLeftHandWeapon )
		{
			const char *szBoneName = GetLinkWeaponBoneName( 1, m_bBattleMode );
			m_hLeftHandWeapon->ChangeLinkBone( szBoneName );
		}
	}
#endif
};

#endif // #ifdef PRE_ADD_KALI