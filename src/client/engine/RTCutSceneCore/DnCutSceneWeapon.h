#pragma once
#include "DnCutSceneActionBase.h"
#include "IDnCutSceneRenderObject.h"
#include "EtMatrixEx.h"

class CDnCutSceneActor;

// Weapon to attach to actor.
class CDnCutSceneWeapon : public CDnCutSceneActionBase,
						  public IDnCutSceneRenderObject
{
public:
	enum
	{
		Sword,
		Gauntlet,
		Axe,
		Hammer,
		SmallBow,
		BigBow,
		CrossBow,
		Staff,
		Book,
		Orb,
		Puppet,
		Mace,
		Flail,
		Wand,
		Shield,
		Arrow,
		Cannon,
		BubbleGun,
		Glove,
		Fan,
		Chakram,
		Charm,
	};

private:
	string				m_strSkinName;
	string				m_strAniName;
	tstring				m_strWeaponName;

	EtAniObjectHandle	m_hObject;
		
	MatrixEx		m_matExWorld;

	float				m_fFrame;

	CDnCutSceneActor*	m_pHasActor;
	int					m_iEquipIndex;
	int					m_iLinkBoneIndex;

	boost::shared_ptr<CDnCutSceneWeapon> m_pParentWeapon;

	int					m_iEquipType;

public:
	CDnCutSceneWeapon(void);
	virtual ~CDnCutSceneWeapon(void);

	bool LoadSkin( const char* pSkinName, const char* pAniName );

	void SetName( const wchar_t* pWeaponName );
	const wchar_t* GetName( void ) { return m_strWeaponName.c_str(); };

	void Process( LOCAL_TIME LocalTime, float fDelta );

	void OnSignal( int iSignalType, int iSignalArrayIndex, void* pSignalData, LOCAL_TIME LocalTime, LOCAL_TIME StartTime, LOCAL_TIME EndTime );
	void UpdateSignal( LOCAL_TIME LocalTime, float fDelta );

	void LinkWeapon( CDnCutSceneActor* pActor, int iEquipIndex, const char *szBoneName = NULL );
	void LinkWeapon( CDnCutSceneActor* pActor, boost::shared_ptr<CDnCutSceneWeapon> pWeapon );

	int GetBoneIndex( const char* pBoneName );
	EtAniObjectHandle GetObjectHandle( void ) { return m_hObject; };

	void SetEquipType( int iEquipType ) { m_iEquipType = iEquipType; };
	int GetEquipType( void ) { return m_iEquipType; };

	// from IDnCutSceneRenderObject
	int GetAniIndex( const char* pAniName );
	int GetCachedAniIndex( int iIndex ) { return m_vlAniIndexList.at(iIndex); };

	void Show( bool bShow ); // bintitle.
};
