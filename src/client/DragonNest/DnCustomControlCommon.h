#pragma once

// Note: Custom control ID
//		Defined in EtUITool, CustomControlList.txt
//
enum emCustomControlId
{
	idStatusStatic = 1,
	idGaugeFace,
	idQuickSlotButton,
	idItemSlotButton,
	// Note : No longer used. Do not delete due to numbering order.
	//
	idPartyListBox,
	idSkillSlotButton,
	// Note : No longer used. Do not delete due to numbering order.
	//
	idPartyInfoStatic,
	idGaugeExp,
	idComboCount,
	idDamageCount,
	idChainCount,

	idZoneGateButton,
	idZoneNPCButton,
	idWorldVillageButton,
	idWorldGameButton,
	idQuestTree,
	idMenuButton,
	idMessageStatic,
	idSkillUpButton,
	idStageEnterButton,
	idBonusBoxButton,

	idMovieControl,
	idColorButton,
	idCpCount,
	idNpcAnswerHtmlTextBox,

	idMissionIconStatic,
	idCpJudgeControl,
	idRadioMsgButton,
	idGestureButton,
	idJobIconStatic,
	idSkillAcquireButton,
	idFishingProgressBar,

	idFarmAreaButton,
	idLoadingButton,
	idRotateToggleButton,

	idMenuListControl,
};

#define SLOT_ICON_XSIZE		50
#define SLOT_ICON_YSIZE		50

// Note : Unify the size of item, skill, and quickslot icons below...?
//
#define ITEMSLOT_ICON_XSIZE			50
#define ITEMSLOT_ICON_YSIZE			50

#define SKILLSLOT_ICON_XSIZE		50
#define SKILLSLOT_ICON_YSIZE		50
#define SKILLSLOT_ICON_XCOUNT		10

#define BUFFSLOT_ICON_XSIZE			25
#define BUFFSLOT_ICON_YSIZE			25
#define BUFFSLOT_ICON_XCOUNT		10
#define BUFFSLOT_ICON_PARTYMEMBER_XSIZE			25
#define BUFFSLOT_ICON_PARTYMEMBER_YSIZE			25

#define QUICKSLOT_ICON_XSIZE		50
#define QUICKSLOT_ICON_YSIZE		50

#define QUICKSLOT_SHORCUT_ICON_XSIZE		34
#define QUICKSLOT_SHORCUT_ICON_YSIZE		34

#define FACE_ICON_XSIZE			85
#define FACE_ICON_YSIZE			85

#define SKILL_TEXTURE_COUNT			10		// Note : Number of skill icon textures
#define SKILL_TEXTURE_ICON_COUNT	200		// Note: Number of icons in a single texture

#define BUFF_TEXTURE_COUNT			2
#define BUFF_TEXTURE_ICON_COUNT		100

#define ITEM_TEXTURE_COUNT			100		// Allocate generously and use as soon as loaded.

#define ITEM_TEXTURE_ICON_COUNT		200
#define ITEM_TEXTURE_X_COUNT		10

#define SYMBOL_ICON_XSIZE			50
#define SYMBOL_ICON_YSIZE			50
#define SYMBOL_ICON_XCOUNT			10

#define MISSION_TEXTURE_ICON_COUNT	200

#define MAX_BUFF_SLOT				7 // MAX_BUBBLE_ONLY_SLOT + MAX_BUFF_ONLY_SLOT
#define MAX_BUFF_ONLY_SLOT			5
#define MAX_BUBBLE_ONLY_SLOT		2

#define MAX_BUFF_SLOT_MINIGAUGE		5

#define BUBBLE_COOLTIME_TEXTURE_IDX	9

#define RADIOMSG_TEXTURE_COUNT		1
#define RADIOMSG_TEXTURE_ICON_COUNT	200
#define RADIOMSGSLOT_ICON_XSIZE		50
#define RADIOMSGSLOT_ICON_YSIZE		50

#define GESTURE_TEXTURE_COUNT		1
#define SECONDARYSKILL_TEXTURE_COUNT	1
#define GESTURE_TEXTURE_ICON_COUNT	200
#define GESTURESLOT_ICON_XSIZE		50
#define GESTURESLOT_ICON_YSIZE		50

#define JOB_NORMALSIZE_ICON_XSIZE	55
#define JOB_NORMALSIZE_ICON_YSIZE	55

#define JOB_SMALLSIZE_ICON_XSIZE	28
#define JOB_SMALLSIZE_ICON_YSIZE	28

#define GUILDMARK_TEXTURE_COUNT		50
#define GUILDMARK_TEXTURE_ICON_COUNT	64
#define GUILDMARK_ICON_XSIZE		64
#define GUILDMARK_ICON_YSIZE		64

enum GAUGE_FACE_TYPE
{
	FT_NONE = -1,
	FT_PLAYER,
	FT_MINI_PLAYER,
	FT_ENEMY,
	FT_MINI_ENEMY,
	FT_MONSTER_PARTS,
};

enum ITEM_SLOT_TYPE
{
	ST_ITEM_NONE,
	ST_QUICKSLOT,
	ST_INVENTORY,
	ST_INVENTORY_QUEST,
	ST_INVENTORY_CASH,
	ST_INVENTORY_VEHICLE,

	ST_INVENTORY_CASHREMOVE, // PRE_ADD_CASHREMOVE

	// This enum is used to distinguish whether it is an ItemSlot in the character window, etc. (bright highlight indicating equippable)
	ST_CHARSTATUS,
	ST_SKILL,
	ST_QUEST,
	ST_STORAGE_PLAYER,
	ST_STORAGE_GUILD,
	ST_STORAGE_FARM,
	ST_STORE,
	ST_SKILL_STORE,

	// Even without special handling, do not set to NONE if the comparison tooltip should appear.
	// Currently comparison tooltips do not appear even for equipment in the sell confirmation window only.
	ST_EXCHANGE,
	ST_ITEM_DISJOINT,
	ST_PLATE,
	ST_JEWEL,	// Unlike plates, jewels placed in the Crest Jewel synthesis window do not render borders.
	ST_ITEM_UPGRADE,
	ST_ITEM_UPGRADE_JEWEL,	// Material item
	ST_ITEM_COMPOUND,
	ST_ITEM_COMPOUND_JEWEL,	// Material item

	ST_INSPECT,	// Inspect

	ST_RADIOMSG,			// Radio message skill window
	ST_RADIOMSG_QUICKSLOT,	// Radio message quickslot
	ST_GESTURE,				// Gesture inventory window
	ST_SECONDARYSKILL,		// Secondary skill window (Fishing, Cooking)
	ST_LIFESKILL_QUICKSLOT,	// LifeSkill quickslot (skill quickslot activated only in Normal_Stand - includes Gesture and SecondarySkill)
	ST_MARKET_REGIST,		// Unmanned shop registered item
	ST_INVENTORY_CASHSHOP,
	ST_ASKLOOKCHANGE,

	ST_NPCREWARD,			// NPC choice reward slot
	ST_MISSIONREWARD,		// Mission reward slot
	ST_NPC_PRESENT,			// Gift slot received by NPC

	ST_ITEM_COSTUMEMIX_STUFF,
	ST_ITEM_COSTUMEMIX_RESULT,

	ST_INVENTORY_CASHSHOP_REFUND,

	ST_FARM_CONDITION,
	ST_LIFESKILL_QUICKSLOT_EX,	// Item quickslot placed in LifeSkill quickslot (mounts, pets)

	ST_ITEM_COSTUME_DMIX_STUFF,
	ST_ITEM_COSTUME_DMIX_RESULT,

	ST_ITEM_COSTUME_RANDOMMIX_STUFF,
	ST_ITEM_COSTUME_RANDOMMIX_RESULT,

	ST_SET_FONTCOLOR,		// Render font color using m_dwFontColor instead of Element's FontColor.
	ST_CASHINSTANCE_ITEM,	// Cash Shop quick purchase item slot

	ST_ITEM_UPGRADE_EXCHANGE_SOURCE,
	ST_ITEM_UPGRADE_EXCHANGE_TARGET,
	ST_ITEM_UPGRADE_EXCHANGE_STUFF,
	ST_ITEM_NOTIFY_COMPOUND,

	ST_TOTAL_LEVEL_SKILL,
	ST_SPECIALBOX_REWARD,

	ST_STORAGE_WORLDSERVER_NORMAL,
	ST_STORAGE_WORLDSERVER_CASH,


#if defined( PRE_PERIOD_INVENTORY )
	ST_INVENTORY_PERIOD,
	ST_STORAGE_PERIOD,
#endif	// #if defined( PRE_PERIOD_INVENTORY )
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	ST_TALISMAN,
	ST_TALISMAN_CASH,
#endif
};

enum QUICKSLOT_STATE
{
	QUICKSLOT_NORMAL,
	QUICKSLOT_USABLE,
	QUICKSLOT_DELAY,
	QUICKSLOT_COMPLETE,
	QUICKSLOT_END,
	QUICKSLOT_LOWSP
};

enum ITEMSLOT_STATE
{
	ITEMSLOT_ENABLE,
	ITEMSLOT_DISABLE,
	ITEMSLOT_EXPIRE,
};

enum SKILLSLOT_STATE
{
	SKILLSLOT_ENABLE,
	SKILLSLOT_DISABLE,
};

enum COLOR_TYPE
{
	COLOR_NONE = -1,
	COLOR_WHITE = 0,
	COLOR_ORANGE,
	COLOR_RED,
	COLOR_DAHONG,
	COLOR_YELLOW,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_PURPLE,
	COLOR_BLUE1,
	COLOR_RED1,
	COLOR_COUNT,
};

inline void CalcButtonUV( int nIndex, EtTextureHandle hTexture, SUICoord &UVCoord, int nIconWidth, int nIconHeight )
{
	if( !hTexture ) return;

	int nCountX = hTexture->Width() / nIconWidth;
	int nCountY = hTexture->Height() / nIconHeight;
	float fTexWidth = nIconWidth / ( float )hTexture->Width();
	float fTexHeight = nIconHeight / ( float )hTexture->Height();

	UVCoord.SetCoord( (nIndex%nCountX)*fTexWidth, (nIndex/nCountX)*fTexHeight, fTexWidth, fTexHeight );
}

inline void CalcButtonUV( int nIndex, EtTextureHandle hTexture, int &nX, int &nY, int nIconWidth, int nIconHeight )
{
	if( !hTexture ) return;

	int nCountX = hTexture->Width() / nIconWidth;
	int nCountY = hTexture->Height() / nIconHeight;

	nX = (nIndex%nCountX) * nIconWidth;
	nY = (nIndex/nCountX) * nIconWidth;
}


// No better place to put it, so place it here.
// Is there no other control that displays hotkeys besides custom button...
void CalcHotKeyUV( int nVKHotKey, EtTextureHandle hTexture, SUICoord &UVCoord, int nIconWidth, int nIconHeight );