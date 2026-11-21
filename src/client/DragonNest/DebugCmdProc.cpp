#include "Stdafx.h"
#include "DebugCmdProc.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnInterface.h"
#include "DnDataManager.h"
#include "DnCEDof.h"
#include "PerfCheck.h"
#include "EtActionCoreMng.h"
#include "LogWnd.h"
#include "SundriesFunc.h"
#include "VillageSendPacket.h"
#include "PartySendPacket.h"
#include "DnLocalPlayerActor.h"
#include "DnTableDB.h"
#include <mmsystem.h>
#include "GlobalValue.h"
#include "DnQuestTask.h"
#include "DnPartsHair.h"
#include "EtBenchMark.h"
#include "DnCutSceneData.h"
#include "DnCommonTask.h"
#include "DnSkillTask.h"
#include "DnActor.h"
#include "DnPlayerCamera.h"
#include "FarmSendPacket.h"

#include "DnActorClassDefine.h"
#include "DnNPCActor.h"
#include "DnVehicleActor.h" 
#include "MAEnvironmentEffect.h"

#include "DnChatTabDlg.h"
#include "DnChatOption.h"

#include "CountryByCountryKeyboard.h"
#include "InputWrapper.h"

extern bool g_bNaviDraw;
extern bool g_bRenderBlur;
extern bool g_bNaviTest;

#ifdef PRE_ADD_JAPAN_AUTO_AIMING
extern int g_iAutoAdmingRangeDelta;
#endif // PRE_ADD_JAPAN_AUTO_AIMING
#ifdef PRE_ADD_SHOW_MONACTION
extern bool g_bShowMonsterAction;
extern int g_nShowMonsterActionId;
extern DWORD g_dwShowMonsterUniqueId;
#endif

#include "DnHomingAccellOrbit.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

enum CmdType
{
	HELP = 0,
	HELP1,
	SET_WIREMODE,
	SET_DEBUG_DRAW,
	TALK_FILE_LIST,
	TALK_FILE_RELOAD,
	DOF,
	DOF_NEAR,
	DOF_FAR,
	DOF_FOCUS,
	DOF_SIZE,
	OUTPUT,
	PROFILE,
	MEM,
	CHANGE_ENVI,
	SHOW_COLL,
	RELOAD_ACTION,
	ADD_SKILL,
	REMOVE_SKILL,
	ADD_SKILL_LEVEL,
	GATE,
	MOVE_SPEED,
	SHOW_NAVI,
	NAVI_PICK,
	CHANGE_MAP,
	NPC_UNLOCK,
	MAPNAME,
	CURRENTMAP,
	IGNORECOOLTIME,
	SHOWFPS,
	ENABLESHADOW,
	DUMP_QUEST_INFO,
	SETACTION,
	SETACTIONT,
	SHOW_ATTR,
	BENCHMARK,
	IGNORE_PVPCONDITION,
	SHOW_RESPAWNINFO,	
	SET_HAIRCOLOR,
	SET_SKINCOLOR,
	SET_EYECOLOR,
	GOTO_MAP,
	SET_HP,
	SET_MP,
	GAINMISSION,
	ACHIEVEMISSION,
	FORCERANK,
	GAINDAILYMISSION,
	ACHIEVEDAILYMISSION,
	SET_ENCHANT,
	LUCKY,
	SETCP,
	SET_RIDE,         
	SET_VEHICLE_COLOR, 
	SET_VEHICLE_PARTS,
	SHOW_DPS,   
	SHOW_SKILLDPS,
	SHOW_CURRENT_ACTION,
	SWAP_DUAL_SKILLTREE,
#if !defined(SW_ADD_CUTSCENECHEAT_20091106_jhk8211)
	CUTSCENE,
#endif
	SETSP,
	BLUR,
	ATTACHCAMERA,
	DETACHCAMERA,
	OCCLUSION,
	CAMERAFAR,
	MAKELAG,
	PLAYERCAMERA,
#ifdef PRE_ADD_JAPAN_AUTO_AIMING
	ADJUST_AUTOAMING_RANGE,
#endif // PRE_ADD_JAPAN_AUTO_AIMING
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	SHOWREPUTE,
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	SHOWVERSION,
	SWAPSKIN,
	SHOWHIT,
	SHOWSHUTDOWN,
	SPECTATORMODE,
	SHOW_HOMING_TARGET,	//For displaying Guided Shot target position
#ifdef PRE_ADD_SHOW_MONACTION
	SHOW_MONSTER_ACTION,
#endif
	SHOW_PLAYER_EFFECT,
#if defined (PRE_PARTY_DB) && defined (_WORK)
	ADD_DUMMY_PARTY,
	SET_PARTYLIST_REFRESHTIME,
#endif
#ifdef PRE_SOURCEITEM_TIMECHEAT
	SOURCEITEM_TIME,
#endif


#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	SET_TOTAL_LEVEL,
	ADD_TOTAL_LEVEL_SKILL,
	REMOVE_TOTAL_LEVEL_SKILL,
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	SET_HEADSCALE,
	SET_KEYBOARD_COUNTRY,
	SHOW_CP,
	SHOW_DAMAGE,
	RUNACTION,
	RELINKVEHICLE,
	CMD_END,
	
};

struct __CmdLineDefineStruct 
{
	int nIndex;
	TCHAR* szString;
	TCHAR* szComment;
};

__CmdLineDefineStruct g_CmdList[] = 
{
	{ HELP, _T("/help"), _T("Help") },
	{ HELP1, _T("/?"), _T("Help") },
	{ SET_WIREMODE, _T("/wireframe"), _T("Switch to wireframe mode")},
	{ SET_DEBUG_DRAW, _T("/dd"), _T("Enable debug drawing")},

	{ TALK_FILE_LIST, _T("/talk_file_list"), _T("View loaded dialogue file list.")},
#if !defined(SW_MODIFY_CHEAT_COMMAND_20091029_b4nfter)
	{ TALK_FILE_RELOAD, _T("/reloadscript"), _T("Reload dialogue files")},
#else	// #if !defined(SW_MODIFY_CHEAT_COMMAND_20091029_b4nfter)
	{ TALK_FILE_RELOAD, _T("/talk_file_reload"), _T("Reload dialogue files")},
#endif	// #if !defined(SW_MODIFY_CHEAT_COMMAND_20091029_b4nfter)
	{ DOF, _T("/dof"), _T("Enable DOF")},
	{ DOF_NEAR, _T("/dofnear"), _T("DOF near")},
	{ DOF_FAR, _T("/doffar"), _T("DOF far")},
	{ DOF_FOCUS, _T("/doffocus"), _T("DOF focus distance")},
	{ DOF_SIZE, _T("/dofsize"), _T("DOF size")},

	{ OUTPUT, _T("/output"), _T("(0/1), (on/off), Hide debug info") },
	{ PROFILE, _T("/profile"), _T("Print accumulated profile values") },
	{ MEM, _T("/mem"), _T("Print accumulated memory check values") },
	{ CHANGE_ENVI, _T("/ce"), _T("(envi name) Change environment") },
	{ SHOW_COLL, _T("/showcollision"), _T("(0:Hide/1:Show), Show/Hide collision box") }, 
	{ RELOAD_ACTION, _T("/reloadaction"), _T("Reload actions") },
	{ ADD_SKILL, _T("/addskill"), _T("Create skill [SkillIndex]") },
	{ REMOVE_SKILL, _T("/removeskill"), _T("Delete skill [SkillIndex]") },
	{ ADD_SKILL_LEVEL, _T("/addskilllevel"), _T("Create skill [SkillIndex][SkillLevel]") },
	{ GATE, _T("/gate"), _T("Exit through gate [GateIndex]") },
	{ MOVE_SPEED, _T("/move_speed"), _T("Set movement speed [Value]") },
	{ SHOW_NAVI, _T("/show_navi"), _T("View navigation mesh.") },
	{ NAVI_PICK, _T("/navi_pick"), _T("For navigation testing") },
	{ CHANGE_MAP, _T("/changemap"), _T("Move map") },
	{ NPC_UNLOCK, _T("/npc_unlock"), _T("Escape when grabbed by NPC") },
	{ MAPNAME, _T("/mapname"), _T("List map names.") },
	{ CURRENTMAP, _T("/curmap"), _T("Current map info") },
	{ IGNORECOOLTIME, _T("/ignorect"), _T("Ignore cooldown (client only)" ) },
	{ SHOWFPS, _T("/showfps"), _T("(0:Hide/1:Show), Show FPS" ) },
	{ ENABLESHADOW, _T("/enableshadow"), _T("Shadow test On/Off") },
	{ DUMP_QUEST_INFO, _T("/dump_quest"), _T("Print quest status info") },
	{ SETACTION, _T("/setaction"), _T("Perform player action") },
	{ SETACTIONT, _T("/setactiont"), _T("Perform crosshair target action") },
	{ SHOW_ATTR, _T("/showattr"), _T("View floor attribute") },
	{ BENCHMARK, _T("/benchmark"), _T("GPU benchmark") },
	//blondy
	{ IGNORE_PVPCONDITION, _T("/pvpcondition"), _T("Ignore PVP game start conditions") },
	{ SHOW_RESPAWNINFO, _T("/respawnlogic"), _T("Display respawn point info on entire map during PVP game") },	
	//blondy end
	{ SET_HAIRCOLOR, _T("/haircolor"), _T("Change hair color.") },
	{ SET_SKINCOLOR, _T("/skincolor"), _T("Change skin color.") },
	{ SET_EYECOLOR, _T("/eyecolor"), _T("Change eye color.") },
	{ GOTO_MAP, _T("/go"), _T("Move map") },
	{ SET_HP, _T("/hp"), _T("Change HP") },
	{ SET_MP, _T("/mp"), _T("Change MP") },
	{ GAINMISSION, _T("/missiongain"), _T("Acquire mission") },
	{ ACHIEVEMISSION, _T("/missionachieve"), _T("Complete mission") },
	{ FORCERANK, _T("/forcerank"), _T("Force set dungeon clear score") },
	{ GAINDAILYMISSION, _T("/dmissiongain"), _T("Acquire daily mission") },
	{ ACHIEVEDAILYMISSION, _T("/dmissionachieve"), _T("Complete daily mission") },
	{ SET_ENCHANT, _T("/setenchant"), _T("Enhance item [SlotIndex][EnhanceLevel][PotentialArrayIndex]") },
	{ LUCKY, _T("/lucky"), _T("Set enhance/combine/craft rate to 100%") },
	{ SET_RIDE , _T("/cmride"), _T("Ride mount")},          
	{ SET_VEHICLE_COLOR , _T("/cmvehiclecolor"), _T("Change mount color")},           
	{ SET_VEHICLE_PARTS , _T("/cmvehicleparts"), _T("Change mount item")},         
	{ SHOW_DPS , _T("/showdps"), _T("Damage per second <Test>")},
	{ SHOW_SKILLDPS , _T("/showskilldps"), _T("Skill damage per second <Test>")},
	{ SHOW_CURRENT_ACTION , _T("/showcurrentaction"), _T("Show current/previous action")}, 
	{ SWAP_DUAL_SKILLTREE , _T("/swapdualskill"), _T("Swap dual skill")},
	{ SETCP, _T("/cp"), _T("Set CP value.") },
	{ CUTSCENE, _T("/cutscene"), _T("/cutscene (CutsceneTableID)") },
	{ SETSP, _T("/setsp"), _T("Usage(/setsp SkillPoints), Desc(Set current character's skill points to the entered value.") },
	{ BLUR, _T("/blur"), _T("/blur Skill effect blur On/Off") },
	{ ATTACHCAMERA, _T("/attachcam"), _T("/attachcam CharacterName") },
	{ DETACHCAMERA, _T("/detachcam"), _T("/detachcam") },
	{ OCCLUSION, _T("/occlusion"), _T("Hardware occlusion On/Off") },
	{ CAMERAFAR, _T("/camerafar"), _T("Set camera Far value") },
	{ MAKELAG, _T("/makelag"), _T("Induce lag (for low-spec testing)") },
	{ PLAYERCAMERA, _T("/pcam"), _T("New player camera") },
#ifdef PRE_ADD_JAPAN_AUTO_AIMING
	{ ADJUST_AUTOAMING_RANGE, _T("/aar"), _T("Max distance (range) +/- offset based on weapon range for auto-targeting check") },
#endif // PRE_ADD_JAPAN_AUTO_AIMING
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	{ SHOWREPUTE, _T("/showrepute"), _T("Print favor/dislike values of the target NPC.") },
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	{ SHOWVERSION, _T("/showver"), _T("Show version") },
	{ SWAPSKIN, _T("/swapskin"), _T("Change player skin") },
	{ SHOWHIT, _T("/showhit"), _T("Show Hit signal") },
	{ SHOWSHUTDOWN, _T("/showshutdown"), _T("Check Korea shutdown system display") },
	{ SPECTATORMODE, _T("/spectatormode"), _T("Observer mode") },
	{ SHOW_HOMING_TARGET, _T("/showhoming"), _T("Show Guided Shot target")	},
#ifdef PRE_ADD_SHOW_MONACTION
	{ SHOW_MONSTER_ACTION, _T("/monaction"), _T("Show monster action")},
#endif
	{ SHOW_PLAYER_EFFECT , _T("/showplayereffect"), _T("For checking player effects")},
#if defined (PRE_PARTY_DB) && defined (_WORK)
#if defined( PRE_WORLDCOMBINE_PARTY )
	{ ADD_DUMMY_PARTY, _T("/addparty"), _T("Create dummy party Usage(/addparty [\"PartyName\"] [TargetMapIndex] [Difficulty(0:EASY ~ 4:ABYSS)] [PartyType(0:Normal 1:Raid 2:Unified]")},
#else
	{ ADD_DUMMY_PARTY, _T("/addparty"), _T("Create dummy party Usage(/addparty [\"PartyName\"] [TargetMapIndex] [Difficulty(0:EASY ~ 4:ABYSS)]")},
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	{ SET_PARTYLIST_REFRESHTIME, _T("/partyrefresh"), _T("Set party list refresh time Usage(/partyrefresh [Time(sec)])")},
#endif
#ifdef PRE_SOURCEITEM_TIMECHEAT
	{ SOURCEITEM_TIME, _T("/srctime"), _T("For checking origin item tooltip duration") },
#endif


#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	{ SET_TOTAL_LEVEL, _T("/settotallevel"), _T("Unified Level") },
	{ ADD_TOTAL_LEVEL_SKILL, _T("/addtotallevelskill"), _T("Unified Level Skill (slotIndex, skillID") },
	{ REMOVE_TOTAL_LEVEL_SKILL, _T("/removetotallevelskill"), _T("Unified Level Skill (slotIndex") },
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	{ SET_HEADSCALE, _T("/headscale"), _T("Change head size [Scale:1.0f]") },
	{ SET_KEYBOARD_COUNTRY, _T("/keyboardcountry"), _T("Change multilingual country keyboard (countryID)") },
	{ SHOW_CP, _T("/showcp"), _T("Print current CP info)") },
	{ SHOW_DAMAGE, _T("/showdamage") , _T("Show damage message") },
	{ RUNACTION, _T("/act") , _T("Run action") },
	{ RELINKVEHICLE, _T("/vehlink") , _T("Vehicle Link Bone") },

};

void PrintCurMapName();
void PrintMapName();
int FindMap(const char* szToolMapName);
/*
When splitting a string by whitespace.. roughly write it like this.
std::vector<std::string> tokens;
std::string str("-IP 127.0.0.1 -Port 5000", );
Tokenize( str, tokens, " ");
*/

void Tokenize( const tstring& str, std::vector<tstring>& tokens, const tstring& delimiters = _T(" ") )
{
	// Ignore if the very first character is a delimiter
	tstring::size_type lastPos = str.find_first_not_of( delimiters, 0 );
	// Find the first character that is not a delimiter
	tstring::size_type pos = str.find_first_of( delimiters, lastPos );

	while( tstring::npos != pos || tstring::npos != lastPos )
	{
		// Token found, so add it to vector
		tokens.push_back( str.substr( lastPos, pos - lastPos ) );
		// Skip delimiters. Note "not_of"
		lastPos = str.find_first_not_of( delimiters, pos );
		// Find the next non-delimiter character
		pos = str.find_first_of( delimiters, lastPos );
	}
}



void OnCmd(int nMsg, tstring& szOriginal, std::vector<tstring>& tokens)
{
	switch( nMsg )
	{
	case HELP:
	case HELP1:
		{
			for ( int i = 0 ; i < CMD_END ; i++ )
			{
				LogWnd::Log(1,_T("[%s] - %s"), g_CmdList[i].szString, g_CmdList[i].szComment);
			}

			SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );
		}
		break;
	case SET_WIREMODE:
		{
			static bool s_bWire = false;
			s_bWire = !s_bWire;
			GetEtDevice()->SetRenderState( D3DRS_FILLMODE, s_bWire ? D3DFILL_WIREFRAME : D3DFILL_SOLID );
		}
		break;
	case SET_DEBUG_DRAW:
		{
			static bool s_bDebugDrawing = false;
			s_bDebugDrawing = !s_bDebugDrawing;
			CDnActor::SetAllMonsterDebugDrawing(s_bDebugDrawing, s_bDebugDrawing);
		}
		break;

	case TALK_FILE_LIST:
		{
			std::vector<std::wstring> _list = g_DataManager.GetTalkFileList();
			for ( int i = 0 ; i < (int)_list.size() ; i++ )
			{
				LogWnd::Log(1,_list[i].c_str());
			}
		}
		break;
	case TALK_FILE_RELOAD:
		{
			g_DataManager.ClearAll();
			g_DataManager.LoadData();

#if !defined(SW_MODIFY_CHEAT_COMMAND_20091029_b4nfter)
			SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );
#endif	// #if !defined(SW_MODIFY_CHEAT_COMMAND_20091029_b4nfter)
		}
		break;
	case DOF:
	case DOF_NEAR:
	case DOF_FAR:
	case DOF_FOCUS:
	case DOF_SIZE:
		{
			DnCameraHandle hCamera;
			hCamera = CDnCamera::GetActiveCamera();
			if( !hCamera ) break;

			static int s_nIndex = -1;
			static EtVector2 vNear( 1000.f, 1500.f );
			static EtVector2 vFar( 2500.f, 4000.f );
			static float fDist = 2000.f;
			static float fNearBlurSize = 5.f;
			static float fFarBlurSize = 5.f;

			if( nMsg == DOF ) {
				if( tokens.size() < 2 ) break;
				int nValue = _ttoi( tokens[1].c_str() );

				if( nValue == 0 ) {
					if( s_nIndex != -1 ) {
						hCamera->RemoveCameraEffect( s_nIndex );
						s_nIndex = -1;
					}
				}
				else {
					if( s_nIndex != -1 ) {
						hCamera->RemoveCameraEffect( s_nIndex );
						s_nIndex = -1;
					}
					if( tokens.size() >= 9 ) {
						vNear.x = (float)_tstof( tokens[2].c_str() );
						vNear.y = (float)_tstof( tokens[3].c_str() );
						vFar.x = (float)_tstof( tokens[4].c_str() );
						vFar.y = (float)_tstof( tokens[5].c_str() );
						fDist = (float)_tstof( tokens[6].c_str() );
						fNearBlurSize = (float)_tstof( tokens[7].c_str() );
						fFarBlurSize = (float)_tstof( tokens[8].c_str() );
					}
					s_nIndex = hCamera->DepthOfField( -1, vNear.x, vNear.y, vFar.x, vFar.y, fDist, fNearBlurSize, fFarBlurSize );
				}
			}
			else {
				if( s_nIndex == -1 )
					s_nIndex = CDnWorld::GetInstance().GetEnvironment()->GetDOFEffectIndex();
				if( s_nIndex == -1 ) break;
				CDnCEDof *pEffect = (CDnCEDof *)hCamera->FindCameraEffect( s_nIndex );
				if( pEffect == NULL ) break;

				switch( nMsg ) {
	case DOF_NEAR:
		if( tokens.size() < 3 ) break;
		pEffect->SetNear( (float)_tstof( tokens[1].c_str() ), (float)_tstof( tokens[2].c_str() ) );
		break;
	case DOF_FAR:
		if( tokens.size() < 3 ) break;
		pEffect->SetFar( (float)_tstof( tokens[1].c_str() ), (float)_tstof( tokens[2].c_str() ) );
		break;
	case DOF_FOCUS:
		if( tokens.size() < 2 ) break;
		pEffect->SetFocusDistance( (float)_tstof( tokens[1].c_str() ) );
		break;
	case DOF_SIZE:
		if( tokens.size() < 3 ) break;
		pEffect->SetBlurSize( (float)_tstof( tokens[1].c_str() ), (float)_tstof( tokens[2].c_str() ) );
		break;
				}
			}
		}
	case OUTPUT:
		{
			if( !_tcscmp( tokens[1].c_str(), _T("on") ) || !_tcscmp( tokens[1].c_str(), _T("1") ) ) 
				SetOutputDebugFuncPtr( LogWnd::CmdLog );
			else if( !_tcscmp( tokens[1].c_str(), _T("off") ) || !_tcscmp( tokens[1].c_str(), _T("0") ) ) 
				SetOutputDebugFuncPtr( _OutputDebug );
		}

		break;
	case PROFILE:
		ProfilePrint( LogWnd::CmdLog );
		break;
	case MEM:
		ProfileMemPrint( LogWnd::CmdLog );
		break;
	case CHANGE_ENVI:
		{
			TCHAR  szEnviName[128];

			_stprintf( szEnviName, _T("Envi\\%s.env"), tokens[1].c_str() );
			//WideCharToMultiByte( CP_ACP, 0, m_szParam, -1, szTemp, 128, NULL, NULL );

			std::string szEnvi;
			ToMultiString(szEnviName, szEnvi);

			bool bFind;
			CFileNameString szFullName = CEtResourceMng::GetInstance().GetFullName( szEnvi.c_str(), &bFind );
			if( bFind )
				CDnWorld::GetInstance().InitializeEnviroment( szFullName.c_str(), CDnCamera::GetActiveCamera() );
			else {
				LogWnd::Log(1, _T("Can't find environment file. [ %s ]"), tokens[1].c_str() );
			}
		}
		break;

	case SHOW_COLL:
		{
			bool bShow = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bShow = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bShow = false;
			}

			CDnActor::SetAllMonsterDebugDrawing(bShow, bShow);
			DNVector(DnPropHandle) hVecList;
			CDnWorld::GetInstance().ScanProp( EtVector3( 0.f, 0.f, 0.f ), FLT_MAX, (PropTypeEnum)-1, hVecList );
			for( DWORD i=0; i<hVecList.size(); i++ ) {
				if( !hVecList[i] ) continue;
				if( !hVecList[i]->GetObjectHandle() ) continue;
				if( hVecList[i]->IsIgnoreBuildColMesh() ) continue;
				hVecList[i]->GetObjectHandle()->ShowCollisionPrimitive( bShow );
				hVecList[i]->GetObjectHandle()->Update( hVecList[i]->GetObjectHandle()->GetWorldMat() );
			}
#ifdef RENDER_PRESS_hayannal2009
			CDnActor::SetRenderPress(bShow);
#endif
		}
		break;
	case SET_RIDE:
		{
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
			bool bShow = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bShow = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bShow = false;
			}
			else
				break;
			GetInterface().ShowPVPVillageAccessDlg(bShow);
#endif
		}
		break;

	case SET_VEHICLE_COLOR:
		{

			if( tokens.size() > 4 )
			{
				if( CDnActor::s_hLocalActor ) {
					CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
					if( pPlayer && pPlayer->IsVehicleMode() ) 
					{
						DWORD color = 0;

						float fHairColor[3]={(float)_tstoi(tokens[2].c_str()),(float)_tstoi(tokens[3].c_str()),(float)_tstoi(tokens[4].c_str())};
						CDnParts::ConvertFloatToR10G10B10( &color , fHairColor );
						pPlayer->GetMyVehicleActor()->ChangeHairColor(color);
					}
				}
			}
			else if( tokens.size() == 2)
			{
				if( CDnActor::s_hLocalActor ) {
					CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

				if( (DWORD)_tstoi(tokens[1].c_str()) != -1 )
				{
					float fHairColor[3]={0,};
					CDnParts::ConvertR10G10B10ToFloat(fHairColor, (DWORD)_tstoi(tokens[1].c_str()));
					EtColor Color( fHairColor[0], fHairColor[1], fHairColor[2], 1.0f);
					pPlayer->GetMyVehicleActor()->ChangeHairColor((DWORD)_tstoi(tokens[1].c_str())); 
				}
				else
				{
					pPlayer->GetMyVehicleActor()->SetDefaultHairColor();
				}
				}

			}
			
		}
		break;

	case SET_VEHICLE_PARTS:
		{

			if( CDnActor::s_hLocalActor ) {
				CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
				if( pPlayer && pPlayer->IsVehicleMode() ) 
				{
					// Default parts only have information about item ID. //
					TVehicleItem tItem;
					memset(&tItem,0,sizeof(tItem));
					tItem.nItemID = (int)_tstoi(tokens[1].c_str());
					pPlayer->GetMyVehicleActor()->EquipItem(tItem);
				}
			}
		}
		break;

	case SHOW_DPS: // Rotha: DPS measurement.
		{
			bool bShow = !CGlobalValue::GetInstance().m_bShowDPS;

			CGlobalValue::GetInstance().m_bShowDPS = bShow;
		
			if( bShow ) {
				CGlobalValue::GetInstance().m_nSumDPS = 0; // Accumulated damage
				CGlobalValue::GetInstance().m_fDPSTime = 0.f;
			}
			else {
				WCHAR wszStr[512];
				swprintf_s( wszStr, L"Total Damage %d \n DPS %f \n Elapsed Time(sec) %f", CGlobalValue::GetInstance().m_nSumDPS ,  (float)(CGlobalValue::GetInstance().m_nSumDPS / (int)CGlobalValue::GetInstance().m_fDPSTime) , CGlobalValue::GetInstance().m_fDPSTime);
				CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption2, wszStr, textcolor::RED );
				CGlobalValue::GetInstance().m_nSumDPS = 0;
				CGlobalValue::GetInstance().m_fDPSTime = 0.f;
			}
		}
		break;

	case SHOW_SKILLDPS:
		{
			bool bShow = !CGlobalValue::GetInstance().m_bShowSkillDPS;

			CGlobalValue::GetInstance().m_bShowSkillDPS = bShow;
			if(bShow == false)
				CGlobalValue::GetInstance().m_bShowDPS = false;
		}
		break;

	case SHOW_CURRENT_ACTION:
		{
			bool bShow = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bShow = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bShow = false;
			}

			CGlobalValue::GetInstance().m_bShowCurrentAction = bShow;
		}
		break;

	case SWAP_DUAL_SKILLTREE:
		{
			bool bShow = true;
			if( tokens.size() > 1 ) 
			{
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) )
					GetSkillTask().SendChangeSkillPage(DualSkill::Type::Primary);
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) 
					GetSkillTask().SendChangeSkillPage(DualSkill::Type::Secondary);
			}
		}
		break;

	case RELOAD_ACTION:
		{
			//			CEtActionCoreMng::GetInstance().Reload();
			//			SendChatMsg( CHATTYPE_NORMAL, _T("/reloadaction") );
		}
		break;
	case ADD_SKILL:
		{
			if( tokens.size() !=2 ) break;
			tstring szTemp = tokens[0] + L" " + tokens[1];
			SendChatMsg( CHATTYPE_NORMAL, szTemp.c_str() );
		}
		break;
	case REMOVE_SKILL:
		{
			if( tokens.size() !=2 ) break;
			tstring szTemp = tokens[0] + L" "+tokens[1];
			SendChatMsg( CHATTYPE_NORMAL, szTemp.c_str() );
		}
		break;
	case ADD_SKILL_LEVEL:
		{
			if( tokens.size() !=3 ) break;
			tstring szTemp = tokens[0] + L" " + tokens[1] + L" " + tokens[2];
			SendChatMsg( CHATTYPE_NORMAL, szTemp.c_str() );
		}
		break;
	case GATE:
		{
			if ( tokens.size() != 2 )
				return ;

			int nGateNum = _wtoi(tokens[1].c_str());

#ifdef PRE_PARTY_DB
			SendStartStage(Dungeon::Difficulty::Easy, -1, false);
#else
			SendStartStage(0, -1, false);
#endif

		}
		break;
	case MOVE_SPEED:
		{
			int nGateNum = _wtoi(tokens[1].c_str());

			CDnActor::s_hLocalActor->SetMoveSpeed(nGateNum);

		}
		break;
	case SHOW_NAVI:
		{
			g_bNaviDraw = !g_bNaviDraw;
		}
		break;
	case NAVI_PICK:
		{
			g_bNaviTest = !g_bNaviTest;
		}
		break;
	case CHANGE_MAP:
	case GOTO_MAP:
		{
			SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );

			// #39851 When map splatting texture index is specified.
#ifndef _FINAL_BUILD
			if( 6 == tokens.size() )
			{
				int iLayerTexture1 = _wtoi( tokens.at( 4 ).c_str() );
				if( 0 < iLayerTexture1 && iLayerTexture1 <= 4 )
					CGlobalValue::GetInstance().m_aiSelectedMapLayerTexture[ 0 ] = iLayerTexture1;

				int iLayerTexture2 = _wtoi( tokens.at( 5 ).c_str() );
				if( 0 < iLayerTexture2 && iLayerTexture2 <= 4 )
					CGlobalValue::GetInstance().m_aiSelectedMapLayerTexture[ 1 ] = iLayerTexture2;
			}
#endif // #ifndef _FINAL_BUILD
		}
		break;
	case NPC_UNLOCK:
		{
			CDnLocalPlayerActor::LockInput(false);
		}
		break;
	case MAPNAME:
		{
			PrintMapName();
		}
		break;
	case CURRENTMAP:
		{
			PrintCurMapName();
		}
		break;
	case IGNORECOOLTIME:
		{
			bool bResetOnly = false;
			if( tokens.size() >= 2 ) {
				bResetOnly = ( _wtoi( tokens[1].c_str() ) > 0 ) ? true : false;
			}
			CGlobalValue::GetInstance().m_bIgnoreCoolTime = !bResetOnly;
			CDnActor::s_hLocalActor->ResetSkillCoolTime();
			SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );
		}
		break;
		case RUNACTION:
		{
			if (CDnActor::s_hLocalActor->IsExistAction(FormatA("%ws", tokens[1].c_str()).c_str()))
			{
				CDnActor::s_hLocalActor->SetActionQueue(FormatA("%ws", tokens[1].c_str()).c_str());
				SendChatMsg(CHATTYPE_NORMAL, L"Execute"); //
			}
			
			CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if (pPlayer && pPlayer->IsVehicleMode())
			{
			//	pPlayer->SetActionQueue(FormatA("%ws", tokens[1].c_str()).c_str());
				pPlayer->GetMyVehicleActor()->SetActionQueue(FormatA("%ws", tokens[1].c_str()).c_str());
				SendChatMsg(CHATTYPE_NORMAL, L"Execute Vehicle"); //
			}

		}
		break;
		case RELINKVEHICLE:
		{
			CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if (pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor())
			{
				pPlayer->GetMyVehicleActor()->GetObjectHandle()->SetParent(CDnActor::s_hLocalActor->GetObjectHandle(), FormatA("%ws", tokens[1].c_str()).c_str());
				SendChatMsg(CHATTYPE_NORMAL, L"RELINK Vehicle"); //
			}

		}
		break;
		case SHOWFPS:
		{
			bool bShow = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bShow = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bShow = false;
			}

			CGlobalValue::GetInstance().m_bShowFPS = bShow;
			if( bShow ) {
				CGlobalValue::GetInstance().m_dwBeginFPSTime = timeGetTime();
				CGlobalValue::GetInstance().m_dwFPSCount = 0;
				CGlobalValue::GetInstance().m_fAverageFPS = 0.f;
				CGlobalValue::GetInstance().m_fMaxFPS = FLT_MIN;
				CGlobalValue::GetInstance().m_fMinFPS = FLT_MAX;
			}
			else {
				CGlobalValue::GetInstance().m_dwEndFPSTime = timeGetTime();
				DWORD dwTime = CGlobalValue::GetInstance().m_dwEndFPSTime - CGlobalValue::GetInstance().m_dwBeginFPSTime;
				float fAveFrame = ( CGlobalValue::GetInstance().m_dwFPSCount == 0 ) ? 0.f : CGlobalValue::GetInstance().m_fAverageFPS / (float)CGlobalValue::GetInstance().m_dwFPSCount;
				WCHAR wszStr[512];
				swprintf_s( wszStr, L"Time : %d min %d sec, Frame Ave : %.2f, Min : %.2f, Max : %.2f\n", ( dwTime / 1000 ) / 60, ( dwTime / 1000 ) % 60, fAveFrame, CGlobalValue::GetInstance().m_fMinFPS, CGlobalValue::GetInstance().m_fMaxFPS );

				CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption2, wszStr, textcolor::RED );

				CGlobalValue::GetInstance().m_dwFPSCount = 0;
				CGlobalValue::GetInstance().m_fAverageFPS = 0.f;
				CGlobalValue::GetInstance().m_fMaxFPS = 0.f;
				CGlobalValue::GetInstance().m_fMinFPS = 0.f;

			}
		}
		break;
	case ENABLESHADOW:
#ifdef _SHADOW_TEST
		{
			bool bShow = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bShow = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bShow = false;
			}
			CGlobalValue::GetInstance().m_bEnableShadow = bShow;
			SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );
		}
#endif
		break;
	case DUMP_QUEST_INFO:
		{
			const TQuestGroup* pGroup = GetQuestTask().GetQuestGroup();
			if ( pGroup )
			{
				for ( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
				{
					TQuest quest = pGroup->Quest[i];
					if ( quest.nQuestID == 0 )
						continue;

					std::wstring wszString;
					wszString = FormatW( L"Slot[%d]QuestID:%d Type:%d Step:%d Journal:%d", i,
						(int)quest.nQuestID, g_DataManager.GetQuestType(quest.nQuestID), (int)quest.nQuestStep, (int)quest.cQuestJournal);

					GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"ClientQ", wszString.c_str());
				}

			}

			SendChatMsg( CHATTYPE_NORMAL, L"/show_quest_info" );

		}
		break;
	case SETACTION:
		{
			if( tokens.size() < 2 ) break;
			std::string szAction;
			int nLoopCount = 0;
			if( tokens.size() == 3 ) nLoopCount = _wtoi(tokens[2].c_str());
			ToMultiString( tokens[1], szAction );

			if( !CDnActor::s_hLocalActor ) break;
			if( !CDnActor::s_hLocalActor->IsExistAction( szAction.c_str() ) ) break;
			CDnActor::s_hLocalActor->CmdAction( szAction.c_str(), nLoopCount );
		}
		break;
	case SETACTIONT:
		{
			if( tokens.size() < 2 ) break;
			std::string szAction;
			int nLoopCount = 0;
			if( tokens.size() == 3 ) nLoopCount = _wtoi(tokens[2].c_str());
			ToMultiString( tokens[1], szAction );

			if( !CDnActor::s_hLocalActor ) break;
			CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
			DnActorHandle hActor = pPlayer->GetLastAimActor();
			if( !hActor ) break;
			if( !hActor->IsExistAction( szAction.c_str() ) ) break;
			hActor->SetActionQueue( szAction.c_str(), nLoopCount );

		}
		break;
	case SHOW_ATTR:
		{
			bool bShow = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bShow = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bShow = false;
				if( CDnActor::s_hLocalActor ) {
					CDnLocalPlayerActor *pLocalPlayer = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
					if( pLocalPlayer ) {
						pLocalPlayer->EnableDebugDraw( bShow );
					}
				}
			}		
		}
		break;
	case BENCHMARK:
		CEtBenchMark::GetInstance().Run();
		break;
	case IGNORE_PVPCONDITION:
		CGlobalValue::GetInstance().m_bPVPGameIgnoreCondition  = !(CGlobalValue::GetInstance().m_bPVPGameIgnoreCondition);
		if( CGlobalValue::GetInstance().m_bPVPGameIgnoreCondition )
			SendChatMsg( CHATTYPE_NORMAL, L" PVP Condition Enable!" );
		else
			SendChatMsg( CHATTYPE_NORMAL, L" PVP Condition disable!" );
		break;
	case SHOW_RESPAWNINFO:
		CGlobalValue::GetInstance().m_bEnableRespawnInfo  = !(CGlobalValue::GetInstance().m_bEnableRespawnInfo);
		if( CGlobalValue::GetInstance().m_bEnableRespawnInfo )
			SendChatMsg( CHATTYPE_NORMAL, L" Respawn Info Enable!" );
		else
			SendChatMsg( CHATTYPE_NORMAL, L" Respawn Info disable!" );

		SendChatMsg( CHATTYPE_NORMAL, L"/respawnlogic" );
		break;
	case SET_HAIRCOLOR:
		if( tokens.size() > 3 ) {
			int nColor[3];
			swscanf(tokens[1].c_str(), L"%d", &nColor[0]);
			swscanf(tokens[2].c_str(), L"%d", &nColor[1]);
			swscanf(tokens[3].c_str(), L"%d", &nColor[2]);
			if( CDnActor::s_hLocalActor ) {
				MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(CDnActor::s_hLocalActor.GetPointer());
				DWORD dwColor = 0;
				CDnParts::ConvertInt3ToR10G10B10( &dwColor, nColor );
				pPartsBody->SetPartsColor( MAPartsBody::HairColor, dwColor );
			}
		}
		break;
	case SET_SKINCOLOR:
		if( tokens.size() > 4 ) {
			float fR=0.5f, fG=0.5f, fB=0.5f, fA=1.0f;
			swscanf( tokens[1].c_str(), L"%f", &fR);
			swscanf( tokens[2].c_str(), L"%f", &fG);
			swscanf( tokens[3].c_str(), L"%f", &fB);
			swscanf( tokens[4].c_str(), L"%f", &fA);
			fA = EtClamp(fA, 0.0f, 1.0f);
			fR = EtClamp(fR, 0.0f, 1.0f);
			fG = EtClamp(fG, 0.0f, 1.0f);
			fB = EtClamp(fB, 0.0f, 1.0f);
			if( CDnActor::s_hLocalActor ) {
				MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(CDnActor::s_hLocalActor.GetPointer());
				if( pPartsBody ) {
					pPartsBody->SetPartsColor( MAPartsBody::SkinColor, D3DCOLOR_ARGB( (int)(fA*255), (int)(fR*255), (int)(fG*255), (int)(fB*255) ) );
				}
			}
		}
		break;
	case SET_EYECOLOR:
		if( tokens.size() > 3 ) {
			int nColor[3];
			swscanf(tokens[1].c_str(), L"%d", &nColor[0]);
			swscanf(tokens[2].c_str(), L"%d", &nColor[1]);
			swscanf(tokens[3].c_str(), L"%d", &nColor[2]);
			if( CDnActor::s_hLocalActor ) {
				MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(CDnActor::s_hLocalActor.GetPointer());
				DWORD dwColor=0;
				CDnParts::ConvertInt3ToR10G10B10(&dwColor, nColor );
				pPartsBody->SetPartsColor( MAPartsBody::EyeColor, dwColor );
			}
		}
		break;
	case SET_HP:
	case SET_MP:
	case GAINMISSION:
	case ACHIEVEMISSION:
	case FORCERANK:
	case GAINDAILYMISSION:
	case ACHIEVEDAILYMISSION:
	case SET_ENCHANT:
	case LUCKY:
		SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );
		break;
	case SETCP:
		if( tokens.size() < 2 ) break;
		if( CDnActor::s_hLocalActor ) {
			CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
			if( pPlayer ) {
				int nCP = _wtoi( tokens[1].c_str() );
				pPlayer->SetAccumulationCP( nCP );
				CDnInterface::GetInstance().ResetCp();
				CDnInterface::GetInstance().SetCpUp(nCP);
				SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );
			}
		}
		break;
#if !defined(SW_ADD_CUTSCENECHEAT_20091106_jhk8211)
	case CUTSCENE:
		{
#ifndef _FINAL_BUILD
			if( (int)tokens.size() == 2 )
			{
				if( tokens.at( 0 ) == L"/cutscene" )
				{
					int iCutSceneTableID = _ttoi( tokens[ 1 ].c_str() );

					DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TCUTSCENE );
					bool bExist = pSox->IsExistItem( iCutSceneTableID );
					if( bExist )
					{
						CFileNameString strFileName = pSox->GetFieldFromLablePtr( iCutSceneTableID, "_FileName" )->GetString();
						if( !strFileName.empty() )
						{
							// Get the current map number.
							int iCurMapID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;

							// If different from the current map, print the map number to the chat window.


							CStream* pStream = CEtResourceMng::GetInstance().GetStream( strFileName );
							_ASSERT( pStream && "Failed to load cutscene file stream!" );
							if( NULL == pStream )
								return;

							int iSize = pStream->Size();
							char* pBuffer = new char[iSize];
							ZeroMemory( pBuffer, iSize );
							pStream->ReadBuffer( (void*)pBuffer, iSize );

							// After calling ReadBuffer, extra trailing memory was attached, so remove it.
							int i = 0;
							for( i = iSize-1; i > 0; --i )
							{
								if( '}' == pBuffer[ i ] && i < iSize-2 )
								{
									pBuffer[ i+1 ] = NULL;
									break;
								}
							}

							IDnCutSceneDataReader* pCutSceneData = new CDnCutSceneData;
							bool bSuccess = pCutSceneData->LoadFromStringBuffer( pBuffer );
							delete [] pBuffer;
							SAFE_DELETE( pStream );

							if( false == bSuccess )
								return;

							int iCutSceneMapID = 0;
							int iNumRegRes = pCutSceneData->GetNumRegResource();
							for( int iRes = 0; iRes < iNumRegRes; ++iRes )
							{
								int iKind = pCutSceneData->GetRegResKindByIndex( iRes );
								if( CDnCutSceneData::RT_RES_MAP == iKind )
								{
									const char* pToolMapName1 = pCutSceneData->GetRegResNameByIndex( iRes );
									iCutSceneMapID = FindMap( pToolMapName1 );
									break;
								}
							}

							if( iCutSceneMapID == iCurMapID )
							{							
								// Construct packet manually and invoke.
								CTaskManager* pTaskManager = CTaskManager::GetInstancePtr();
								CDnCommonTask* pCommonTask = static_cast<CDnCommonTask*>(pTaskManager->GetTask( "CommonTask" ));
								SCPlayCutScene MockCutScenePlayPacket;
								MockCutScenePlayPacket.nCutSceneTableID = iCutSceneTableID;

								pCommonTask->OnRecvQuestPlayCutScene( &MockCutScenePlayPacket );
								pCommonTask->UsedCutSceneCheat();
							}
							else
							{
								//std::wstring sz = FormatW( L"Please move to MapID:[%d] and use the cutscene cheat key.", iCutSceneMapID );
								std::wstring sz = FormatW( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 102044), iCutSceneMapID );
								GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"", sz.c_str());
							}

							SAFE_DELETE(pCutSceneData);
						}
					}
				}
			}
#endif
		}
		break;
#endif

	case SETSP:
		{
			// Modified so server sends a separate packet.
			//if ( tokens.size() < 2 ) 
			//	break;

			//int iSkillPoint = _wtoi( tokens[1].c_str() );
			//GetSkillTask().SetSkillPoint( iSkillPoint );
			//GetInterface().RefreshSkillDialog();

			SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );
		}
		break;
	case BLUR:
		g_bRenderBlur = !g_bRenderBlur;
		break;
	case ATTACHCAMERA:
		{
			if ( tokens.size() < 2 ) break;

			DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
			if( !hCamera ) break;
			if( hCamera->GetCameraType() != CDnCamera::PlayerCamera ) break;
			DnActorHandle hActor = CDnActor::FindActorFromName( (WCHAR*)tokens[1].c_str() );
			if( !hActor ) break;

			((CDnPlayerCamera*)hCamera.GetPointer())->DetachActor();
			((CDnPlayerCamera*)hCamera.GetPointer())->AttachActor( hActor );
		}
		break;
	case DETACHCAMERA:
		{
			DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
			if( !hCamera ) break;
			if( hCamera->GetCameraType() != CDnCamera::PlayerCamera ) break;
			if( !CDnActor::s_hLocalActor ) break;

			((CDnPlayerCamera*)hCamera.GetPointer())->DetachActor();
			((CDnPlayerCamera*)hCamera.GetPointer())->AttachActor( CDnActor::s_hLocalActor );
		}
		break;
	case OCCLUSION:
		{
			bool bOcclusion = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bOcclusion = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bOcclusion = false;
			}
			CEtRenderStack::EnableOcclusion( bOcclusion );
			SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );
		}
		break;
	case CAMERAFAR:
		if( tokens.size() > 1 ) 
		{
			EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
			if( !hCamera ) break;
			//if( hCamera->GetCameraType() != CDnCamera::PlayerCamera ) break;
			//if( !CDnActor::s_hLocalActor ) break;
			int nFar;
			swscanf(tokens[1].c_str(), L"%d", &nFar);
			hCamera->SetCameraFar( (float)nFar );
			hCamera->Reset();
		}
		break;
	case MAKELAG:
		if( tokens.size() > 1 ) 
		{
			if( CTaskManager::IsActive()	) {
				int nLag;
				swscanf(tokens[1].c_str(), L"%d", &nLag);
				CTaskManager::GetInstance().DebugTestLag( nLag );
			}
		}
		break;
	case PLAYERCAMERA:
		if( tokens.size() > 1 ) 
		{
			int nOn=0;
			swscanf(tokens[1].c_str(), L"%d", &nOn);		
			DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
			if( !hCamera ) break;
			if( hCamera->GetCameraType() != CDnCamera::PlayerCamera ) break;
			((CDnPlayerCamera*)hCamera.GetPointer())->EnablePitchCamera( nOn != 0 );		
		}
		break;

#ifdef PRE_ADD_JAPAN_AUTO_AIMING
	case ADJUST_AUTOAMING_RANGE:
		{
			if( tokens.size() > 1 )
			{
				int iRangeDelta = 0;
				swscanf( tokens[ 1 ].c_str(), L"%d", &iRangeDelta );
				g_iAutoAdmingRangeDelta = iRangeDelta;
			}
		}
		break;
#endif // PRE_ADD_JAPAN_AUTO_AIMING
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	case SHOWREPUTE:
		{
			if( tokens.size() > 1 )
			{

				CReputationSystemRepository* pReputationRepos = GetQuestTask().GetReputationRepository();

				// TODO: Need to handle cases where NPC representative ID exists.
				int iNpcID = 0;
				swscanf_s( tokens[1].c_str(), L"%d", &iNpcID );

				DNTableFileFormat* pReputationTable = GetDNTable( CDnTableDB::TREPUTE );
				vector<int> m_vlResultNpcID;
				pReputationTable->GetItemIDListFromField( "_NpcID", iNpcID, m_vlResultNpcID );
				if( false == m_vlResultNpcID.empty() )
				{
					int iFavor = (int)pReputationRepos->GetNpcReputation( iNpcID, IReputationSystem::NpcFavor );
					int iFavorMax = (int)pReputationRepos->GetNpcReputationMax( iNpcID, IReputationSystem::NpcFavor );
					int iMalice = (int)pReputationRepos->GetNpcReputation( iNpcID, IReputationSystem::NpcMalice );
					int iMaliceMax = (int)pReputationRepos->GetNpcReputationMax( iNpcID, IReputationSystem::NpcMalice );

					std::wstring sz = FormatW( L"Favor[%d] FavorMax[%d] | Malice[%d] MaliceMax[%d]", iFavor, iFavorMax, iMalice, iMaliceMax );
					GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"", sz.c_str() );
				}
				else
				{
					GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", L"That Npc does not exist in the affinity system." );
				}
			}
		}
		break;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	case SWAPSKIN:
		{
			if( !CDnActor::s_hLocalActor ) break;
			int nActorTableID = -1;
			if( tokens.size() > 1 ) nActorTableID = _wtoi( tokens[1].c_str() );

			CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if( nActorTableID > 0 ) {
				
				pPlayer->SwapSingleSkin( nActorTableID );
				pPlayer->ResetCustomAction();
				pPlayer->ResetMixedAnimation( false );
				pPlayer->SetAction( "Stand", 0.f, 0.f );
			}
			else {
				nActorTableID = -1;
				pPlayer->SwapSingleSkin( nActorTableID );
				pPlayer->SetAction( "Stand", 0.f, 0.f );
			}

			DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
			if( !hCamera ) return;
			CDnPlayerCamera *pPlayerCamera = dynamic_cast<CDnPlayerCamera*>(hCamera.GetPointer());
			if( !pPlayerCamera ) return;
			if( pPlayerCamera->GetAttachActor() != CDnActor::s_hLocalActor ) {
				pPlayerCamera->AttachActor( CDnActor::s_hLocalActor );
				pPlayerCamera->ResetCamera();
			}
		}
		break;
#ifndef _FINAL_BUILD
	case SHOWHIT:
		{
			bool bShow = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bShow = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bShow = false;
			}

			CGlobalValue::GetInstance().m_bDrawHitSignal = bShow;
			if( tokens.size() > 2 ) {
				CGlobalValue::GetInstance().m_fDrawHitSignalMinDelta = (float)_wtof( tokens[2].c_str() );
			}
		}
		break;

	case SPECTATORMODE:
		{
			if( !CDnActor::s_hLocalActor ) break;

			bool bEnable = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bEnable = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bEnable = false;
			}

			CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if( pPlayer )
				pPlayer->OnApplySpectator(bEnable);
		}
		break;
#if defined(PRE_FIX_HOMINGTARGET)
	case SHOW_HOMING_TARGET:
		{
			bool bEnable = true;
			if( tokens.size() > 1 ) {
				if( !_tcscmp( tokens[1].c_str(), _T("1") ) ) bEnable = true;
				else if( !_tcscmp( tokens[1].c_str(), _T("0") ) ) bEnable = false;
			}

			CDnHomingAccellOrbit::ms_ShowHomingTarget = bEnable;
		}
		break;
#endif // PRE_FIX_HOMINGTARGET

#ifdef PRE_ADD_SHOW_MONACTION
	case SHOW_MONSTER_ACTION:
		{
			g_bShowMonsterAction = !g_bShowMonsterAction;

			int nMonsterId = -1;
			DWORD dwMonsterUniqueId = 0;
			if( tokens.size() > 1 )
			{
				nMonsterId = _wtoi(tokens[1].c_str());
				g_bShowMonsterAction = true;
			}
			if (tokens.size() > 2)
			{
				dwMonsterUniqueId = _wtoi(tokens[2].c_str());
				g_bShowMonsterAction = true;
			}

			g_nShowMonsterActionId = nMonsterId;
			g_dwShowMonsterUniqueId = dwMonsterUniqueId;
		}
		break;
#endif

	case SHOW_PLAYER_EFFECT:
		{
			if(CDnActor::s_hLocalActor)
			{
				if( tokens.size() > 1 )
				{
					CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
					std::string szString;
					ToMultiString(tokens[1] , szString);

					pPlayer->ToggleLinkedPlayerEffect( true , szString.c_str() );
				}
			}
		}
		break;
#endif
#if defined (PRE_PARTY_DB) && defined (_WORK)
	case ADD_DUMMY_PARTY:
		{
#if defined( PRE_WORLDCOMBINE_PARTY )
			if (tokens.size() < 5)
#else
			if (tokens.size() < 4)
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
			{
#if defined( PRE_WORLDCOMBINE_PARTY )
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", L"Dummy party creation usage (/addparty [\"PartyName\"] [TargetMapIndex] [Difficulty(0:EASY ~ 4:ABYSS)] [PartyType(0:Normal 1:Raid 2:Unified]");
#else
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", L"Dummy party creation usage (/addparty [\"PartyName\"] [TargetMapIndex] [Difficulty(0:EASY ~ 4:ABYSS)]");
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
				return;
			}

			size_t cmdFound = szOriginal.find(g_CmdList[nMsg].szString);
			if (cmdFound != tstring::npos)
			{
				SPartyCreateParam param;

				size_t cmdLen = lstrlenW(g_CmdList[nMsg].szString);
				tstring arguments = szOriginal.substr(cmdLen + 1);
				if (arguments.empty() == false)
				{
					size_t partyNameSize = arguments.find_last_of(L"\"");
					if (partyNameSize < 1)
					{
#if defined( PRE_WORLDCOMBINE_PARTY )
						GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", L"Dummy party creation usage (/addparty [\"PartyName\"] [TargetMapIndex] [Difficulty(0:EASY ~ 4:ABYSS)] [PartyType(0:Normal 1:Raid 2:Unified]");
#else
						GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", L"Dummy party creation usage (/addparty [\"PartyName\"] [TargetMapIndex] [Difficulty(0:EASY ~ 4:ABYSS)]");
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
						return;
					}
					param.wszPartyName = arguments.substr(1, partyNameSize - 1);

					tstring argTemp = arguments.substr(partyNameSize + 2);
#if defined( PRE_WORLDCOMBINE_PARTY )
					swscanf(argTemp.c_str(), L"%d %d %d", &(param.nMapIdx), &(param.Difficulty), &(param.PartyType) );
					param.bAllowWorldZonePartyList = true;
#else
					swscanf(argTemp.c_str(), L"%d %d", &(param.nMapIdx), &(param.Difficulty) );
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
					param.bAddDummyParty = true;

					CDnPartyTask::GetInstance().ReqCreateParty(param);
				}
			}
		}
		break;

	case SET_PARTYLIST_REFRESHTIME:
		{
			if (tokens.size() <= 1)
			{
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", L"Party list refresh interval usage (/partyrefresh [Time(sec)])");
				return;
			}

			if (CDnPartyTask::IsActive())
			{
				int nSec;
				swscanf(tokens[1].c_str(), L"%d", &nSec);

				CDnPartyTask::GetInstance().SetPartyListRefreshTime((float)nSec);
			}
		}
		break;
#endif
#ifdef PRE_SOURCEITEM_TIMECHEAT
	case SOURCEITEM_TIME:
		{
			if( CDnActor::s_hLocalActor )
			{
				CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
				pPlayer->m_bSourceItemTimeCheat = !pPlayer->m_bSourceItemTimeCheat;
			}
		}
		break;
#endif


#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	case SET_TOTAL_LEVEL:
		{
			if (tokens.size() <= 1)
			{
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", L"Set total level (/settotallevel level");
				return;
			}
			int nLevel = 0;
			swscanf( tokens[1].c_str(), L"%d", &nLevel );

			if (nLevel <= 0)
				nLevel = 0;

			
			if( CDnActor::s_hLocalActor )
			{
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
				pPlayer->UpdateTotalLevel(nLevel);
			}
		}
		break;
	case ADD_TOTAL_LEVEL_SKILL:
		{
			if (tokens.size() <= 2)
			{
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", L"Add total level skill (/addtotallevelskill slotIndex skillID");
				return;
			}
			int nSlotIndex = -1;
			int nSkillID = 0;

			swscanf( tokens[1].c_str(), L"%d", &nSlotIndex );
			swscanf( tokens[2].c_str(), L"%d", &nSkillID );


			if( CDnActor::s_hLocalActor )
			{
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
				pPlayer->AddTotalLevelSkill(nSlotIndex, nSkillID);
			}
		}
		break;
	case REMOVE_TOTAL_LEVEL_SKILL:
		{
			if (tokens.size() <= 1)
			{
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", L"Remove total level skill (/removetotallevelskill slotIndex");
				return;
			}
			int nSlotIndex = -1;
			swscanf( tokens[1].c_str(), L"%d", &nSlotIndex );

			
			if( CDnActor::s_hLocalActor )
			{
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
				pPlayer->RemoveTotalLevelSkill(nSlotIndex);
			}
		}
		break;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
	case SET_HEADSCALE:
		{
			if( !CDnActor::s_hLocalActor ) break;
			if( tokens.size() <= 1 ) break;
			float fScale = (float)_wtof(tokens[1].c_str());
			CDnActor::s_hLocalActor->SetHeadScale( fScale );
		}
		break;

	case SET_KEYBOARD_COUNTRY:
		{
		}
		break;

	case SHOW_CP:
		{
			if( tokens.size() <= 1 ) break;
			int nShow = _wtoi( tokens[1].c_str() );
			CGlobalValue::GetInstance().m_bShowCP = nShow == 1 ? true : false;
		}
		break;

	case SHOW_DAMAGE:
		{
			if( tokens.size() <= 1 ) break;
			int nShow = _wtoi( tokens[1].c_str() );
			CGlobalValue::GetInstance().m_bShowDamage = nShow == 1 ? true : false;
		}
		break;

	default:
		{
			//LogWnd::Log(1,_T("Unknown command."));
			//			SendChatMsg( CHATTYPE_NORMAL, szOriginal.c_str() );

		}

		return;
	}

	LogWnd::Log(1,_T("Executed %s."), g_CmdList[nMsg].szString);

}

bool CALLBACK DebugCmdProc(const TCHAR* szCmd)
{
#ifdef _FINAL_BUILD
	tstring szMsg;
	szMsg = szCmd;

	// Convert to lowercase
	//	std::transform(szMsg.begin(), szMsg.end(), szMsg.begin(), towlower); 

	bool bFound = false;
	std::vector<tstring> tokens;
	Tokenize(szMsg, tokens, _T(" "));
	if( tokens.empty() ) return false;
	std::transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), towlower); 

	tstring szOriginal;
	szOriginal = szCmd;
	for ( int i = 0 ; i < CMD_END ; i++ )
	{
		//char Str[2048] = { 0, };		WideCharToMultiByte( CP_ACP, 0, g_CmdList[i].szString, -1, Str , 2048, NULL, NULL );

		if( _tcscmp( tokens[0].c_str(), g_CmdList[i].szString ) == NULL )
		{
			OnCmd(g_CmdList[i].nIndex, szOriginal, tokens);
			bFound = true;
		}
	}

	if ( bFound == false )
	{
		if (  szOriginal.find(_T("/")) <= szOriginal.size() )
			OnCmd(0xffffffff, szOriginal, tokens);
		return false;
	}
	return true;
#else
	if( CDnActor::s_hLocalActor && static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer())->IsDeveloperAccountLevel() )
	{
		tstring szMsg;
		szMsg = szCmd;

		// Convert to lowercase
		//	std::transform(szMsg.begin(), szMsg.end(), szMsg.begin(), towlower); 

		bool bFound = false;
		std::vector<tstring> tokens;
		Tokenize(szMsg, tokens, _T(" "));
		if( tokens.empty() ) return false;
		std::transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), towlower); 

		tstring szOriginal;
		szOriginal = szCmd;

		if( _tcscmp( tokens[0].c_str(), g_CmdList[CUTSCENE].szString ) == NULL )
		{
			if( (int)tokens.size() == 2 )
			{
				if( tokens.at( 0 ) == L"/cutscene" )
				{
					int iCutSceneTableID = _ttoi( tokens[ 1 ].c_str() );

					DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TCUTSCENE );
					bool bExist = pSox->IsExistItem( iCutSceneTableID );
					if( bExist )
					{
						CFileNameString strFileName = pSox->GetFieldFromLablePtr( iCutSceneTableID, "_FileName" )->GetString();
						if( !strFileName.empty() )
						{
							// Get the current map number.
							int iCurMapID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;

							// If different from the current map, print the map number to the chat window.
							CStream* pStream = CEtResourceMng::GetInstance().GetStream( strFileName );
							_ASSERT( pStream && "Failed to load cutscene file stream!" );
							if( NULL == pStream )
								return false;

							int iSize = pStream->Size();
							char* pBuffer = new char[iSize];
							ZeroMemory( pBuffer, iSize );
							pStream->ReadBuffer( (void*)pBuffer, iSize );

							// After calling ReadBuffer, extra trailing memory was attached, so remove it.
							int i = 0;
							for( i = iSize-1; i > 0; --i )
							{
								if( '}' == pBuffer[ i ] && i < iSize-2 )
								{
									pBuffer[ i+1 ] = NULL;
									break;
								}
							}

							IDnCutSceneDataReader* pCutSceneData = new CDnCutSceneData;
							bool bSuccess = pCutSceneData->LoadFromStringBuffer( pBuffer );
							delete [] pBuffer;
							SAFE_DELETE( pStream );

							if( false == bSuccess )
								return false;

							int iCutSceneMapID = 0;
							int iNumRegRes = pCutSceneData->GetNumRegResource();
							for( int iRes = 0; iRes < iNumRegRes; ++iRes )
							{
								int iKind = pCutSceneData->GetRegResKindByIndex( iRes );
								if( CDnCutSceneData::RT_RES_MAP == iKind )
								{
									const char* pToolMapName1 = pCutSceneData->GetRegResNameByIndex( iRes );
									iCutSceneMapID = FindMap( pToolMapName1 );
									break;
								}
							}

							if( iCutSceneMapID == iCurMapID )
							{							
								// Construct packet manually and invoke.
								CTaskManager* pTaskManager = CTaskManager::GetInstancePtr();
								CDnCommonTask* pCommonTask = static_cast<CDnCommonTask*>(pTaskManager->GetTask( "CommonTask" ));
								SCPlayCutScene MockCutScenePlayPacket;
								MockCutScenePlayPacket.nCutSceneTableID = iCutSceneTableID;

								pCommonTask->OnRecvQuestPlayCutScene( &MockCutScenePlayPacket );
							}
							else
							{
								//std::wstring sz = FormatW( L"Please move to MapID:[%d] and use the cutscene cheat key.", iCutSceneMapID );
								std::wstring sz = FormatW( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 102044), iCutSceneMapID );
								GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"", sz.c_str());
							}

							SAFE_DELETE(pCutSceneData);

							return true;
						}
					}
				}
			}
		}
		else if( _tcscmp( tokens[0].c_str(), g_CmdList[SHOWVERSION].szString ) == NULL ) {
			if( (int)tokens.size() >= 1 )
			{
				bool bShow = true;
				if( (int)tokens.size() >= 2 ) bShow = ( _ttoi( tokens[ 1 ].c_str() ) == 0 ) ? false : true;
				CGlobalInfo::GetInstance().m_bShowVersion = bShow;
			}
		}
	}
#endif
	return false;
}

void PrintMapName()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
	if ( !pSox ) return;

	for( int i=0; i<pSox->GetItemCount(); i++ ) 
	{
		if ( pSox->GetFieldFromLablePtr( i, "_ToolName1" ) == NULL  )
			continue;
		std::string szToolName = pSox->GetFieldFromLablePtr( i, "_ToolName1" )->GetString();
		std::wstring wszToolName;
		ToWideString(szToolName, wszToolName);
		int nStringTableID = pSox->GetFieldFromLablePtr( i, "_MapNameID" )->GetInteger();
		std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );

		std::wstring sz = FormatW( L"MapID:[%d] ToolName:[%s] MapName:[%s] " , i, wszToolName.c_str(), wszName.c_str()  );
		if ( szToolName.empty() )
			continue;
		GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"", sz.c_str());

	}
}

void PrintCurMapName()
{
	int nCurMap = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
	if ( !pSox ) return;

	std::string szToolName = pSox->GetFieldFromLablePtr( nCurMap, "_ToolName1" )->GetString();
	std::wstring wszToolName;
	ToWideString(szToolName, wszToolName);
	int nStringTableID = pSox->GetFieldFromLablePtr( nCurMap, "_MapNameID" )->GetInteger();
	std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );

	std::wstring sz = FormatW( L"Current MapID:[%d] ToolName:[%s] MapName:[%s] " ,  nCurMap, wszToolName.c_str(), wszName.c_str()  );
	GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"", sz.c_str());

}

int FindMap(const char* szToolMapName)
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
	if ( !pSox ) return -1;

	std::string _szToolMapName;
	_szToolMapName = szToolMapName;
	ToLowerA(_szToolMapName);
	for( int i=0; i<pSox->GetItemCount(); i++ ) 
	{
		int iItemID = pSox->GetItemID( i );
		std::string szToolName = pSox->GetFieldFromLablePtr( iItemID, "_ToolName1" )->GetString();
		ToLowerA(szToolName);

		if ( szToolName == _szToolMapName )
			return iItemID;
	}

	return -1;

}