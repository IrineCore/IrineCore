#include "StdAfx.h"
#include "DnPartsFairy.h"
#include "DnTableDB.h"
#include "DnActor.h"
#include "DnPlayerActor.h"


CDnPartsFairy::CDnPartsFairy()
{
	m_EffectType = FairyEffectType::None;
#ifdef PRE_MOD_CASHFAIRY
	m_nLinkBoneIndexForCalc = -1;
#endif
}

CDnPartsFairy::~CDnPartsFairy()
{
}

bool CDnPartsFairy::Initialize( int nPartsTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound, int nLookItemID )
{
	if( CDnParts::Initialize( nPartsTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound, nLookItemID ) == false ) return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TFAIRY );
	if( !pSox ) return false;

	const DNTableCell* pFairyTypeField = pSox->GetFieldFromLablePtr( nPartsTableID, "_FairyType" );
	const DNTableCell* pWeaponEffectActionField = pSox->GetFieldFromLablePtr( nPartsTableID, "_WeaponEffectAction" );
	if (pFairyTypeField == NULL || pWeaponEffectActionField == NULL)
		return false;

	m_EffectType = (FairyEffectType)pFairyTypeField->GetInteger();
	m_szChangeWeaponActionStr = pWeaponEffectActionField->GetString();

	return true;
}

bool CDnPartsFairy::CreateObject( DnActorHandle hActor, const char *szAniName )
{
	bool bResult = CDnParts::CreateObject( hActor, szAniName );

	switch( m_EffectType ) {
		case FairyEffectType::ChangeWeaponAction:
			// Originally, in CDnWeapon::GetWeaponMatchAction, if Player and Fairy parts exist, get m_szChangeWeaponActionStr and
			// Attaching it would be clean, but for players with parts checks etc., it can get messy and affect lookup performance
			// That did not seem great, so implement it generically by getting and using the AddionalWeaponAction member on CDnActor.
			if( !m_szChangeWeaponActionStr.empty() && hActor ) {
				hActor->SetAdditionalWeaponAction( m_szChangeWeaponActionStr );
				// Must call ReleaseSignalImp on the weapon side to delete remaining effects.
				hActor->ReleaseWeaponSignalImp();
			}
			break;
	}

	return bResult;
}

bool CDnPartsFairy::FreeObject()
{
	switch( m_EffectType ) {
#ifdef PRE_MOD_CASHFAIRY
		case FairyEffectType::None:
			m_nLinkBoneIndexForCalc = -1;
			break;
#endif
		case FairyEffectType::ChangeWeaponAction:
			if( !m_szChangeWeaponActionStr.empty() && m_hActor ) {

				DnActorHandle hActor = m_hActor;
				if( m_hActor->IsVehicleActor() )
				{
					CDnVehicleActor* pVehicle = static_cast<CDnVehicleActor*>(m_hActor.GetPointer());
					if( pVehicle->GetMyPlayerActor() )
						hActor = pVehicle->GetMyPlayerActor()->GetActorHandle();
				}

				hActor->SetAdditionalWeaponAction( std::string("") );
				// Must call ReleaseSignalImp on the weapon side to delete remaining effects.
				hActor->ReleaseWeaponSignalImp();
			}
			break;
	}

	return CDnParts::FreeObject();
}

#ifdef PRE_MOD_CASHFAIRY
void CDnPartsFairy::LinkFairy( DnActorHandle hActor )
{
	switch( m_EffectType ) {
		case FairyEffectType::None:
			{
				DnActorHandle hTempActor = hActor;
				CDnPlayerActor *pPlayerActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
				if( pPlayerActor && pPlayerActor->IsVehicleMode() && pPlayerActor->GetMyVehicleActor() )
					hTempActor = pPlayerActor->GetMyVehicleActor()->GetMySmartPtr();

				// Since we know cash items attach to a dummy anyway, process based on the dummy.
				m_nLinkBoneIndexForCalc = -1;
				MAActorRenderBase *pRender = dynamic_cast<MAActorRenderBase *>(hTempActor.GetPointer());
				if( pRender ) {
					int nBoneIndex = pRender->GetDummyBoneIndex( "#Cash_Spirit" );
					if( nBoneIndex != -1 ) {
						if( m_hObject ) {

							// Exception handling: use legacy handling in RenderAvatar.
							if( m_bRTTRenderMode )
							{
								m_hObject->SetParent( hTempActor->GetObjectHandle(), "#Cash_Spirit" );
								break;
							}

							m_nLinkBoneIndexForCalc = nBoneIndex;
							m_hParentObjectForCalc = hTempActor->GetObjectHandle();
							EtMatrixIdentity( &m_matCalculated );
							EtMatrixIdentity( &m_matCurrent );
							m_matCurrent._42 = -100000.0f;
						}
					}
				}
			}
			break;
	}
}

void CDnPartsFairy::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnParts::Process( LocalTime, fDelta );

	switch( m_EffectType ) {
		case FairyEffectType::None:
			{
				if( m_nLinkBoneIndexForCalc == -1 ) break;
				if( !m_hParentObjectForCalc )
				{
					if( m_hParentObjectForCalc != CEtObject::Identity() )
					{
						if( m_hActor && m_hActor->GetObjectHandle() )
							m_hParentObjectForCalc = m_hActor->GetObjectHandle();
					}
				}
				if( !m_hParentObjectForCalc ) break;

				// Find the position where it originally should be,
				EtMatrix *pBoneMat;
				pBoneMat = m_hParentObjectForCalc->GetDummyTransMat( m_nLinkBoneIndexForCalc );
				if( pBoneMat )
				{
					EtMatrixMultiply( &m_matCalculated, pBoneMat, m_hParentObjectForCalc->GetWorldMat() );
				}

				// Keep the current position as is while always receiving the rotation value,
				memcpy_s( &m_matCurrent, sizeof(float)*12, &m_matCalculated, sizeof(float)*12 );

				// The movement value is
				EtVector3 vCalcuated = *(EtVector3*)&m_matCalculated._41;
				EtVector3 vCurrent = *(EtVector3*)&m_matCurrent._41;

				// If the difference is 1000.0f or more, take it as is
				if( EtVec3Length( &( vCurrent - vCalcuated ) ) > 1000.0f )
				{
					memcpy_s( &m_matCurrent._41, sizeof(float)*4, &m_matCalculated._41, sizeof(float)*4 );
				}
				else
				{
					// If below that, have it follow slowly.
					EtVector3 vLerp;
					EtVec3Lerp( &vLerp, &vCurrent, &vCalcuated, fDelta * 1.75f );
					memcpy_s( &m_matCurrent._41, sizeof(float)*3, &vLerp, sizeof(float)*3 );
				}

				// Force call Update function at the end.
				m_hObject->Update( &m_matCurrent );
			}
			break;
	}
}
#endif