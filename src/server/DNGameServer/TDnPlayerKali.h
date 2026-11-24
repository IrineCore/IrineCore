#pragma once

#include "DnWeapon.h"
#include "TDnPlayerCommon.h"

#ifdef PRE_ADD_KALI

template < class T >

class TDnPlayerKali : public TDnPlayerCommon<T>
{
public:
	TDnPlayerKali( CMultiRoom *pRoom, int nClassID )
		: TDnPlayerCommon<T>(pRoom, nClassID)
	{
		memset( m_nKickActionIndex, -1, sizeof(m_nKickActionIndex) );
		m_fPitchAngle = 0.f;
	}

	virtual ~TDnPlayerKali()
	{
		for( int i=0; i<4; i++ ) SAFE_RELEASE_SPTR( m_hKick[i] );
	}

	struct RotateBoneStruct {
		char *szBoneName;
		int nBoneIndex;
		float fWeight;
	};

protected:
	int m_nKickActionIndex[4];
	DnWeaponHandle m_hKick[4];
	float m_fPitchAngle;

	static RotateBoneStruct s_RotateBoneDefine[];

public:
	// Actor Message
	virtual bool Initialize()
	{
		TDnPlayerCommon<T>::Initialize();

		m_hKick[0] = CDnWeapon::CreateWeapon( this->GetRoom(), 5036, 0 );
		m_hKick[1] = CDnWeapon::CreateWeapon( this->GetRoom(), 5037, 0 );
		m_hKick[2] = CDnWeapon::CreateWeapon( this->GetRoom(), 5038, 0 );
		m_hKick[3] = CDnWeapon::CreateWeapon( this->GetRoom(), 5036, 0 );
		for( int i=0; i<4; i++ ) m_hKick[i]->CreateObject();

		m_nKickActionIndex[0] = this->GetElementIndex( "Skill_SlightTurn" );
		m_nKickActionIndex[1] = this->GetElementIndex( "Skill_ReturnSpin" );
		m_nKickActionIndex[2] = this->GetElementIndex( "Skill_SoulKiss" );
		m_nKickActionIndex[3] = this->GetElementIndex( "Skill_Ghostkick" );

		for( int i=0; ; i++ ) {
			if( s_RotateBoneDefine[i].szBoneName == NULL ) break;
			s_RotateBoneDefine[i].nBoneIndex = this->GetBoneIndex( s_RotateBoneDefine[i].szBoneName );
		}
		return true;
	}

	virtual DnWeaponHandle GetWeapon( int nEquipIndex = 0 )
	{
		if( nEquipIndex == 0 ) {
			for( int i=0; i<4; i++ ) {
				if( this->m_nActionIndex == m_nKickActionIndex[i] ) return m_hKick[i];
			}
		}

		return TDnPlayerCommon<T>::GetWeapon( nEquipIndex );
	}
	virtual DnWeaponHandle GetActiveWeapon( int nEquipIndex )
	{
		if( nEquipIndex == 0 ) {
			for( int i=0; i<4; i++ ) {
				if( this->m_nActionIndex == m_nKickActionIndex[i] ) return m_hKick[i];
			}
		}

		return TDnPlayerCommon<T>::GetActiveWeapon( nEquipIndex );
	}
};

#endif // #ifdef PRE_ADD_KALI