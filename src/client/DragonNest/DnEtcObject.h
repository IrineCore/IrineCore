#pragma once

#include "DnActionRenderBase.h"
#include "DnUnknownRenderObject.h"
#include "DnActionSignalImp.h"

class CDnEtcObject : public CDnActionRenderBase, 
					 public CDnActionSignalImp,
					 public CDnUnknownRenderObject<CDnEtcObject>
{
public:
	CDnEtcObject( bool bProcess = true );
	virtual ~CDnEtcObject();

	// EtcObject used by the state effect
	bool m_bUseAsStateEffect;

protected:
	DnActorHandle m_hParentActor;
	bool m_bBillboard;
	bool m_bYBillboard;
	bool m_bAttachParentPosition;
	EtObjectHandle m_hApplyLightObject;

#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
	DnActorHandle m_hPlayerActor;
#endif 

protected:
	virtual EtLightHandle CreateLight( SLightInfo *pInfo );

public:

	// CDnActionRenderBase
	virtual bool Initialize( const char *szSkinName, const char *szAniName, const char *szActName );

	// CDnUnknownRenderBase
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	// CDnActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );

	// CDnActionSignalImp
	virtual MatrixEx *GetObjectCross() override;
	virtual EtAniObjectHandle GetAniObjectHandle() { return m_hObject; }
	virtual CDnRenderBase *GetRenderBase() {return this;}

	static void ProcessClass( LOCAL_TIME LocalTime, float fDelta );
	static bool InitializeClass();
	static void ReleaseClass();

	void Show( bool bShow );

	void AsStateEffect( void ) { m_bUseAsStateEffect = true; };
	bool IsUsingStateEffect( void ) { return m_bUseAsStateEffect; };

	void SetParentActor( DnActorHandle hActor ) { m_hParentActor = hActor; }
	EtMatrix GetBoneMatrix( const char *szBoneName, bool *bExistBone = NULL ) override;

#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
	void SetPlayerActor( DnActorHandle hPlayerActor ) { m_hPlayerActor = hPlayerActor; }
#endif 

	void SetApplyLightObject( EtObjectHandle hObject ) { m_hApplyLightObject = hObject; }
};
