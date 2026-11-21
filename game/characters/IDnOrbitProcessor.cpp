#include "stdafx.h"
#include "IDnOrbitProcessor.h"
#include "DnProjectile.h"
#include "DnLinearOrbit.h"
#include "DnLinearAccellOrbit.h"
#include "DnTerrainLinearOrbit.h"
#include "DnHomingOrbit.h"
#include "DnHomingAccellOrbit.h"
#include "DnGravityOrbit.h"
#include "DnCurveOrbit.h"
#include "DnTerrainHomingOrbit.h"
#include "VelocityFunc.h"

#include "DnPartsMonsterActor.h"

IDnOrbitProcessor* IDnOrbitProcessor::Create( const MatrixEx& Cross, const MatrixEx& OffsetCross, const S_PROJECTILE_PROPERTY* pOrbitProperty )
{
	_ASSERT( pOrbitProperty );

	IDnOrbitProcessor* pResult = NULL;

	// Create and initialize an appropriate trajectory handler object based on trajectory info and return it.
	switch( pOrbitProperty->eOrbitType )
	{
		case CDnProjectile::Linear:
			pResult = new CDnLinearOrbit( pOrbitProperty );
			break;

		case CDnProjectile::Acceleration:
			pResult = new CDnLinearAccellOrbit( pOrbitProperty );
			break;
		
		case CDnProjectile::TerrainLinear:
			pResult = new CDnTerrainLinearOrbit( pOrbitProperty );
			break;

		case CDnProjectile::Homing:
			if( CDnProjectile::Accell == pOrbitProperty->eVelocityType )
			{
				pResult = new CDnHomingAccellOrbit( pOrbitProperty );
			}
			else
			if( CDnProjectile::Constant == pOrbitProperty->eVelocityType )
			{
				pResult = new CDnHomingOrbit( pOrbitProperty );
			}
			break;

		case CDnProjectile::Projectile:
			if( CDnProjectile::Accell == pOrbitProperty->eVelocityType )
			{
				pResult = new CDnGravityOrbit( pOrbitProperty );
			}
			else
			if( CDnProjectile::Constant == pOrbitProperty->eVelocityType )
			{
				pResult = new CDnCurveOrbit( pOrbitProperty );
			}
			break;

		case CDnProjectile::TerrainHoming:
			if( CDnProjectile::Constant == pOrbitProperty->eVelocityType )
			{
				pResult = new CDnTerrainHomingOrbit( pOrbitProperty );
			}
			break;

		default:
			_ASSERT( !"IDnOrbitProcessor::Create() -> Invalid OrbitType!!" );
			break;
	}

	if (pResult)
		pResult->Initialize( Cross, OffsetCross );
	else _ASSERT(0);

	return pResult;
}


void IDnOrbitProcessor::ProcessFallGravityOrbit( MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta, float& fGravity, float fFallGravityAccell )
{
	Cross.m_vPosition.y += CalcMovement( fGravity, fDelta, FLT_MAX, FLT_MIN, fFallGravityAccell);
}

EtVector3 IDnOrbitProcessor::FindTargetPosition(DnActorHandle hActor, int nPartsIndex, int nBoneIndex)
{
	EtVector3 vTargetPosition = EtVector3(0.0f, 0.0f, 0.0f);
	if (!hActor)
		return vTargetPosition;

	if (hActor->IsMonsterActor() && nPartsIndex != -1 && nBoneIndex != -1)
	{
		if( hActor->IsPartsMonsterActor() ) {
			CDnPartsMonsterActor* pPartsMonsterActor = static_cast<CDnPartsMonsterActor*>(hActor.GetPointer());
			MonsterParts* pMonsterParts = pPartsMonsterActor->GetPartsByIndex(nPartsIndex);
			if (pMonsterParts)
			{
				const MonsterParts::_Info &partsInfo = pMonsterParts->GetPartsInfo();
				const MonsterParts::_PartsInfo &partsBoneInfo = partsInfo.vParts[nBoneIndex];
				bool bExistBone = false;
				EtMatrix boneMatrix = pPartsMonsterActor->GetBoneMatrix(partsBoneInfo.szBoneName.c_str());

				vTargetPosition = *(EtVector3*)&boneMatrix._41;
			}
			else
			{
				vTargetPosition = hActor->FindAutoTargetPos();
				vTargetPosition.y += hActor->GetHeight() / 2.f;
			}
		}
	}
	else
	{
		vTargetPosition = hActor->FindAutoTargetPos();
		vTargetPosition.y += hActor->GetHeight() / 2.f;
	}

	return vTargetPosition;
}
