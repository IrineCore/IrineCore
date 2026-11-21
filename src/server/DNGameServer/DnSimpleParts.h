#pragma once

#include "DnUnknownRenderObject.h"
#include "DnRenderBase.h"
#include "DnActionBase.h"
#include "DnItem.h"


// Rotha //  SimpleParts
// Class with only basic animation parts - created to attach rendering. //
// Parts can also be easily attached to Actors such as PET, NPC, or mounts. //

class CDnSimpleParts : public CDnUnknownRenderObject< CDnSimpleParts , MAX_SESSION_COUNT >, 
	public CDnActionBase , 
	public CDnItem ,
	public CDnRenderBase
{
public:
	
	CDnSimpleParts(CMultiRoom *pRoom);
	virtual ~CDnSimpleParts();
	
protected:
	
	DnActorHandle m_hActor;
	EtAniObjectHandle m_hObject;
	DWORD m_dwUniqueID;
	
protected:

public:
	
	virtual bool Initialize( int nPartsTableID , const char *szSkinName , const char *szAniName );
	virtual bool CreateObject( const char *szSkinName , const char *szAniName );

	static DnPartsHandle CreateParts( CMultiRoom *pRoom, int nPartsTableID , const char *szSkinName , const char *szAniName );

	virtual EtAniObjectHandle GetAniObjectHandle() { return m_hObject; }

	void SetUniqueID( DWORD dwUniqueID ) { m_dwUniqueID = dwUniqueID; }
	DWORD GetUniqueID() { return m_dwUniqueID; }
	EtAniObjectHandle GetObjectHandle() { return m_hObject; }	
	
	void SetActor( DnActorHandle hActor ){ m_hActor = hActor; }
	void InitializeItem( int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false);
	bool FreeObject();

};

