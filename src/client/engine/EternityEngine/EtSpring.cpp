#include "stdafx.h"
#include "EtSpring.h"
#include "EtSaveMat.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

extern float g_fElapsedTime;

void CEtMassPoint::Simulate()
{
	const float maxVel = 50.0f;
	float sq = D3DXVec3LengthSq(&Vel);
	float remainSq = sq-maxVel*maxVel;
	if( remainSq > 0) {
		float addVel = powf(remainSq*10.f,0.3f)*0.15f;
		Vel = (maxVel + addVel) * Vel / sqrtf(sq);
	}
	Pos += Vel * 0.15f;
	PrevVel = Vel;	
}

//// CEtSpring //////////////////////////////////////////////////////////////////////
CEtSpring::CEtSpring()
{
	m_bEnable = false;
	m_fStep = 0;
}

void CEtSpring::Create( const char *szFileName, const std::vector< EtMatrix > &InvWorldMatList, std::map<std::string, int> &boneIndexMap  )
{
	m_bEnable = true;
	int nBoneCount = (int)InvWorldMatList.size();
	m_Masses.resize( nBoneCount );

	m_fStep = g_fElapsedTime;

	for( int i = 0; i < nBoneCount; i++) {
		EtMatrix TransMat;
		EtMatrixInverse(&TransMat, NULL, &InvWorldMatList[i]);
		m_Masses[i].Pos = *(EtVector3*)&TransMat._41;
		m_Masses[i].bForceLimit = true;
	}

	CResMngStream Stream( szFileName );
	Stream.Seek(0, SEEK_END);
	int nFileSize = Stream.Tell();
	Stream.Seek(0, SEEK_SET);
	char *pBuffer = new char[nFileSize+1];
	Stream.Read(pBuffer, nFileSize);
	pBuffer[ nFileSize ] = '\0';

	char *str = strtok(pBuffer, "\n");
	while( str != NULL ) {
		if( strncmp(str, "MASS", 4) == 0 ) {
			char szBoneName[255];
			EtVector3 vecPos;
			sscanf(str+4, "%s %f %f %f", szBoneName, &vecPos.x, &vecPos.y, &vecPos.z);
			int nBoneIndex = boneIndexMap[szBoneName];
			if( nBoneIndex < (int)m_Masses.size() ) {
				if( EtVec3Length(&vecPos) == 0.f ) {
					m_Masses[nBoneIndex].SetLocked( true );
				}
				else {
					m_Masses[nBoneIndex].SetGravity(vecPos);
				}
				m_Masses[nBoneIndex].Vel = m_Masses[nBoneIndex].PrevVel = EtVector3(0,0,0);
				m_Masses[nBoneIndex].nSpringCount = 0;
			}
			else {
				// Bone count mismatch. It will not link properly; likely exported incorrectly or matched mismatched msh and txt files.
				ASSERT( 0 && "Invalid Spring Info!" );
			}
		}
		else if( strncmp(str, "SPRING", 6) == 0 ) {
			char szBoneName1[255];
			char szBoneName2[255];
			float fSpringK;
			sscanf(str+6, "%s %s %f", szBoneName1, szBoneName2, &fSpringK);
			int nBoneIndex1 = boneIndexMap[szBoneName1];
			int nBoneIndex2 = boneIndexMap[szBoneName2];
			if( nBoneIndex1 < (int)m_Masses.size() && nBoneIndex2 < (int)m_Masses.size() ) {
				m_Masses[nBoneIndex1].AddSpringBoth(&m_Masses[nBoneIndex2], fSpringK);
			}
			else {
				// Bone count mismatch. It will not link properly; likely exported incorrectly or matched mismatched msh and txt files.
				ASSERT( 0 && "Invalid Spring Info!" );
			}
		}
		str = strtok(NULL, "\n");
	}
	delete [] pBuffer;
}

void CEtSpring::InputKeyframe( EtMatrix WorldMat, int nSaveMatIndex )
{
	EtMatrix *pBoneMatrix = GetEtSaveMat()->GetMatrix( nSaveMatIndex );
	int nMassCount = (int)m_Masses.size();
	for ( int i = 0; i < nMassCount; i++) {		
		EtMatrix WorldBoneMat;
		EtMatrixMultiply(&WorldBoneMat, &pBoneMatrix[i], &WorldMat);
		m_Masses[i].Pos = *(EtVector3*)&WorldBoneMat._41;
		m_Masses[i].Vel = EtVector3(0,0,0);
		m_Masses[i].PrevVel = EtVector3(0,0,0);
	}
}

void CEtSpring::Simulate( EtMatrix WorldMat, const std::vector< EtMatrix > &InvWorldMatList )
{
	int i;
	int nMassCount = (int)m_Masses.size();

	int nPrevStep = (int)m_fStep;
	float fNewStep = m_fStep + g_fElapsedTime * 200;
	int nMaxStep = (int)fNewStep - nPrevStep;
	m_fStep = fNewStep;
	nMaxStep = __min(20, nMaxStep);
	
	for( int nStep = 0; nStep < nMaxStep; nStep++) {
		EtVector3 d, force, additionalFriction;
		for( i = 0; i < nMassCount; i++) {
			CEtMassPoint *mp = &m_Masses[i];
			mp->Vel -= m_Masses[i].gravity;
			if (!mp->bLocked) {
				for (int j = 0; j < mp->nSpringCount; j++) {
					d.x = mp->nodes[j]->Pos.x - mp->Pos.x;
					d.y = mp->nodes[j]->Pos.y - mp->Pos.y;
					d.z = mp->nodes[j]->Pos.z - mp->Pos.z;
					float len = D3DXVec3Length(&d);
					float mulValue = (len - mp->NaturalLength[j]) * mp->fPower[j] / len;
					float fLengthRatio = len / mp->NaturalLength[j];
					if( fLengthRatio > 15.f ) {
						mp->bForceLimit = true;
					}
					force.x = d.x * mulValue;
					force.y = d.y * mulValue;
					force.z = d.z * mulValue;
					float fFriction = 0.3f;
					additionalFriction.x = (mp->nodes[j]->PrevVel.x - mp->PrevVel.x) * fFriction;
					additionalFriction.y = (mp->nodes[j]->PrevVel.y - mp->PrevVel.y) * fFriction;
					additionalFriction.z = (mp->nodes[j]->PrevVel.z - mp->PrevVel.z) * fFriction;
					force.x += additionalFriction.x;
					force.y += additionalFriction.y; 
					force.z += additionalFriction.z;
					mp->Vel.x += force.x * 0.1f;
					mp->Vel.y += force.y * 0.1f;
					mp->Vel.z += force.z * 0.1f;
					mp->Vel *= 0.985f;
				}
			}
		}
		for (i = 0; i < nMassCount; i++) {
			if (!m_Masses[i].bLocked && !m_Masses[i].bForceLimit ) {
				m_Masses[i].Simulate();
			}
			else {
				m_Masses[i].bForceLimit = false;
				EtMatrix TransMat;
				EtMatrixInverse(&TransMat, NULL, &InvWorldMatList[i]);
				EtMatrix WorldBoneMat;
				EtMatrixMultiply(&WorldBoneMat, &TransMat, &WorldMat);
				m_Masses[i].Pos = *(EtVector3*)&WorldBoneMat._41;
				
			}
		}
	}
}

EtMatrix CEtSpring::GetMatrix( int nIndex, int nChildOffset)
{
	EtMatrix TransMat;
	EtMatrixIdentity( &TransMat );
	//EternityEngine::DrawLine3D( *GetPosition(nIndex) , *GetPosition(nIndex+nChildOffset), 0xffffffff );
	//EternityEngine::DrawPoint3D( *GetPosition(nIndex), 0xffffffff );
	EtVector3 vAxis;
	if( nChildOffset < 0 )
		vAxis = *GetPosition(nIndex) - *GetPosition(nIndex+nChildOffset);
	else
		vAxis = *GetPosition(nIndex+nChildOffset) - *GetPosition(nIndex);
	EtVec3Normalize(&vAxis, &vAxis);
	*(EtVector3*)&TransMat._11 = vAxis;
	*(EtVector3*)&TransMat._21 = EtVector3(0,1,0);
	EtVec3Cross((EtVector3*)&TransMat._31, (EtVector3*)&TransMat._11, (EtVector3*)&TransMat._21);
	EtVec3Normalize((EtVector3*)&TransMat._31, (EtVector3*)&TransMat._31);
	EtVec3Cross((EtVector3*)&TransMat._21, (EtVector3*)&TransMat._31, (EtVector3*)&TransMat._11);
	EtVec3Normalize((EtVector3*)&TransMat._21, (EtVector3*)&TransMat._21);
	*(EtVector3*)&TransMat._41 = *GetPosition(nIndex);

	return TransMat;
}

EtVector3* CEtSpring::GetPosition( int nIndex ) 
{
	static EtVector3 vNullPos(0,0,0);
	if( nIndex >= 0 && nIndex < (int)m_Masses.size() ) {
		return &m_Masses[nIndex].Pos;
	}
	return &vNullPos;
}