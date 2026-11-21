#pragma once

class CEtTriggerElement;
void ClearScriptLoadResource();

void DefAllAPIFunc( lua_State *pLuaState );

void api_trigger_FadeBGM( float fTargetVolume, float fDelta );
void api_trigger_PlaySound( int nFileIndex );
void api_trigger_OpenTextureDialog( int nFileIndex, float fX, float fY, int nTime, int nDialogIndex );
void api_trigger_OpenTextureDialogEx( int nFileIndex, float fX, float fY, int nPos, int nTime, int nDialogIndex );
void api_trigger_CloseTextureDialog( bool bIsDialogIndex, int nIndex );
bool api_trigger_Delay( void *pRoom, CEtTriggerElement *pElement, int nDelay );
void api_trigger_CloseAllTextureDialog();
void api_trigger_OpenAlarmDialog( int nPanelIndex, float fX, float fY, float fWidth, int nTime, int nUIStringIndex, int nDialogIndex );
void api_trigger_OpenAlarmDialogEx( int nPanelIndex, float fX, float fY, int nPos, int nFormat, int nTime, int nUIStringIndex, int nDialogIndex );
void api_trigger_OpenCountDownAlarmDialogEx( int nPanelIndex, float fX, float fY, int nPos, int nFormat, int nTime, int nCountDown, int nDialogIndex );
void api_trigger_CloseAlarmDialog( bool bIsDialogIndex, int nIndex );
void api_trigger_CloseAllAlarmDialog();
void api_trigger_BeginCountDown( int nSec, int nUIStringIndex );
void api_trigger_BeginCountDownEx( int nSec, int nUIStringIndex, int type );
void api_trigger_EndCountDown();
void api_trigger_EnableTriggerElement( void *pRoom, CEtTriggerElement *pElement, bool bEnable );
void api_trigger_PlayCutSceneByTrigger( void *pRoom, int nCutSceneTableID, bool bFadeIn );
void api_trigger_ShowNotifyArrowByEventArea( int nEventAreaHandle, int nDelay );
void api_trigger_ShowNotifyArrowByProp( int nPropHandle, int nDelay );
void api_trigger_HideNotifyArrow();
void api_trigger_ChangeBGM( int nFileIndex );
void api_trigger_ChangeEnviVolume( float fVolume );
void api_trigger_CameraWork1( int nStartPropHandle, int nEndPropHandle, int nDelay, bool bStartVel, bool bEndVel );
void api_trigger_CameraWork2( int nWorkPropHandle, int nTargetPropHandle, int nTotalAngle, int nDelay, bool bStartVel, bool bEndVel );
void api_trigger_CameraWork3( int nTargetPropHandle, int nDelay, bool bStartVel, bool bEndVel );
void api_trigger_AddCameraEffect_Quake( int nDelay, float fBeginRatio, float fEndRatio, float fValue );
void api_trigger_AddCameraEffect_DepthOfField( int nDelay, float fFocusDist, float fNearStart, float fNearEnd, float fFarStart, float fFarEnd, float fNearBlur, float fFarBlur );
void api_trigger_AddCameraEffect_RadialBlur( int nDelay, float fBlurCenterX, float fBlurCenterY, float fBlurSize, float fBeginRatio, float fEndRatio );
void api_trigger_DLChangeRound( void *pRoom, bool bBoss );
int api_trigger_DLGetRound( void *pRoom );
int api_trigger_DLGetTotalRound( void *pRoom );
void api_trigger_OpenBlind();
void api_trigger_CloseBlind();
void api_trigger_ShowBlindCaption( int nUIStringIndex, int nDelay );
#ifdef PRE_ADD_FADE_TRIGGER
void api_trigger_FadeScreen( int nSourceAlpha, int nSourceRed, int nSourceGreen, int nSourceBlue,
							int nDestAlpha, int nDestRed, int nDestGreen, int nDestBlue, float fDelta );
void api_trigger_ShowFadeScreenCaption( int nUIStringIndex, int nDelay );
#else // PRE_ADD_FADE_TRIGGER
void api_trigger_FadeScreen( DWORD dwTargetColor, float fDelta );
#endif // PRE_ADD_FADE_TRIGGER
void api_trigger_ChangeEnvironment( const char *szEnviFileName );
void api_trigger_InitializeMODDialog( int nMODTableID );
void api_trigger_LinkMODValue( CEtTriggerElement *pElement, int nIndex, int nDefineValueIndex );
void api_trigger_ShowMODDialog( bool bShow );
int api_trigger_GetMonsterAreaLiveCount( void *pRoom, int nEventAreaHandle );
int api_trigger_GetMonsterAreaTotalCount( void *pRoom, int nEventAreaHandle );
int api_trigger_GetMonsterActorFromEventArea( void *pRoom, int nEventAreaHandle, int nIndex );
void api_trigger_ShowBossAlertDialog( bool bShow, int nUIStringIndex = 0, int nFileIndex = 0 );
int api_trigger_GetPropDurability( void *pRoom, int nPropHandle );
bool api_trigger_MyObseverInEventArea( int nSrcEventAreaHandle );
void api_trigger_MyObseverEventAreaMove( int nDestEventAreaHandle );
void api_trigger_BattleBGMToggle( bool bToggle );
int  api_trigger_TriggerVariableTable( void * pRoom, int nItemID );
void api_trigger_HurryUpTime( int nTime );
void api_trigger_DungeonClear_AddReward( int nPropHandle );
bool IsPlayingTriggerCamera();
#ifdef PRE_ADD_MONSTER_PARTS_UI_TRIGGER
void api_trigger_LinkMODPartsValue( CEtTriggerElement *pElement, int nIndex, int nDefineValueIndex, int nGaugeType, int nPartsIndex );
int api_trigger_GetMonsterParts( void *pRoom, int nEventAreaHandle, int nIndex, int nPartsID );
#endif // PRE_ADD_MONSTER_PARTS_UI_TRIGGER
#ifdef PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER
void api_trigger_OpenImageBlind();
void api_trigger_CloseImageBlind();
void api_trigger_ShowCationImageBlind(int nUIStringIndex, int nDelay);
void api_trigger_ShowNPCImageBlind(bool bShow, int nUIStringIndex = 0, int nFileIndex = 0, int nImageType = 0);
#endif // PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER
void api_trigger_DungeonSynchroDialog( bool bShow, bool bSynchro );
#ifdef PRE_ADD_PART_SWAP_RESTRICT_TRIGGER
void api_trigger_PartSwapRestrict(bool bSwitch);
#endif
void api_trigger_ShowAlteaBoard();
void api_trigger_ShowAlteaClear( int nClear );
void api_trigger_DisableReturnVillage();
void api_trigger_Camera_Look( int nType, int nUniqueID, int nDuration );
void api_trigger_ShowPVPVillageAccessDlg( bool bShow );