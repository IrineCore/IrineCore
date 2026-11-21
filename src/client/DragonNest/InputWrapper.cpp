#include "StdAfx.h"
#include "InputWrapper.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

BYTE g_WrappingKeyData[WrappingKeyIndex_Amount] = {
	DIK_W,
	DIK_S,
	DIK_A,
	DIK_D,
	DIK_E,
	DIK_SPACE,
	DIK_F,
	DIK_R,
	DIK_G,
	DIK_LCONTROL,
	DIK_LMENU,
	DIK_LBRACKET,
	DIK_RBRACKET,
	DIK_TAB,
#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	DIK_NUMLOCK,
#endif
};

BYTE g_DefaultWrappingKeyData[WrappingKeyIndex_Amount] = {
	DIK_W,
	DIK_S,
	DIK_A,
	DIK_D,
	DIK_E,
	DIK_SPACE,
	DIK_F,
	DIK_R,
	DIK_G,
	DIK_LCONTROL,
	DIK_LMENU,
	DIK_LBRACKET,
	DIK_RBRACKET,
	DIK_TAB,
#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	DIK_NUMLOCK,
#endif
};

WCHAR *g_szKeyString[] = {
	L"NULL",
	L"ESC",
	L"1",
	L"2",
	L"3",
	L"4",
	L"5",
	L"6",
	L"7",
	L"8",
	L"9",
	L"0",
	L"-",
	L"=",
	L"<-",
	L"Tab",
	L"Q",
	L"W",
	L"E",
	L"R",
	L"T",
	L"Y",
	L"U",
	L"I",
	L"O",
	L"P",
	L"[",
	L"]",
	L"Enter",
	L"Control",
	L"A",
	L"S",
	L"D",
	L"F",
	L"G",
	L"H",
	L"J",
	L"K",
	L"L",
	L";",
	L"'",
	L"~",
	L"LShift",
	L"\\",
	L"Z",
	L"X",
	L"C",
	L"V",
	L"B",
	L"N",
	L"M",
	L",",
	L".",
	L"/",
	L"RShift",
	L"*",
	L"LAlt",
	L"Space",
	L"Capital",
	L"F1",
	L"F2",
	L"F3",
	L"F4",
	L"F5",
	L"F6",
	L"F7",
	L"F8",
	L"F9",
	L"F10",
	L"NumLock",
	L"Scroll",
	L"NumPad7",
	L"NumPad8",
	L"NumPad9",
	L"Num-",
	L"NumPad4",
	L"NumPad5",
	L"NumPad6",
	L"Num+",
	L"NumPad1",
	L"NumPad2",
	L"NumPad3",
	L"NumPad0",
	L"Num.",		// 0x53
	L"NULL",
	L"NULL",
	L"Unknown",		// 0x56
	L"F11",
	L"F12",			// 0x58
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"F13",			// 0x64
	L"F14",
	L"F15",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"Kana",
	L"NULL",
	L"NULL",
	L"ABNT_C1",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"Convert",
	L"NULL",
	L"NoConvert",
	L"NULL",
	L"Yen",
	L"ABNT_C2",		// 0x7e
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"Num=",		// 0x8d
	L"NULL",
	L"NULL",
	L"PrevTrack",	// 0x90
	L"At",
	L"Colon",
	L"Underline",
	L"Kanji",
	L"Stop",
	L"AX",
	L"Unlabenled",
	L"NULL",
	L"NEXTTRACK",
	L"NULL",
	L"NULL",
	L"NUMPADENTER",
	L"RControl",
	L"NULL",
	L"NULL",
	L"MUTE",		// 0xa0
	L"CALCULATOR",
	L"PLAYPAUSE",
	L"NULL",
	L"MEDIASTOP",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"VOLUMEDOWN",	// 0xae
	L"NULL",
	L"VOLUMEUP",
	L"NULL",
	L"WEBHOME",		// 0xb2
	L"NUMPADCOMMA",
	L"NULL",
	L"DIVIDE",		// 0xb5
	L"NULL",
	L"SYSRQ",
	L"RAlt",		// 0xb8
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"Pause",		// 0xc5
	L"NULL",
	L"Home",		// 0xc7
	L"Up",
	L"Prior",
	L"NULL",
	L"Left",		// 0xcb
	L"NULL",
	L"Right",		// 0xcd
	L"NULL",
	L"End",			// 0xcf
	L"Down",
	L"Next",
	L"Insert",
	L"Delete",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"LWin",		// 0xdb
	L"RWin",
	L"APPS",
	L"POWER",
	L"SLEEP",
	L"NULL",
	L"NULL",
	L"NULL",
	L"WAKE",		// 0xe3
	L"NULL",
	L"WEBSEARCH",
	L"WEBFAVORITES",
	L"WEBREFRESH",
	L"WEBSTOP",
	L"WEBFORWARD",
	L"WEBBACK",
	L"MYCOMPUTER",
	L"MAIL",
	L"MEDIASELECT",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
	L"NULL",
};

BYTE g_UIWrappingKeyData[UIWrappingKeyIndex_Amount] = {
	DIK_1,
	DIK_2,
	DIK_3,
	DIK_4,
	DIK_5,
	DIK_6,
	DIK_7,
	DIK_8,
	DIK_9,
	DIK_0,
	DIK_P,
	DIK_O,
	DIK_I,
	DIK_U,
	DIK_K,
	DIK_L,
	DIK_M,
	DIK_H,
	DIK_C,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	DIK_Y,
#endif
	DIK_GRAVE,
	DIK_T,
	DIK_Q,
	DIK_ESCAPE,
#ifdef _USE_VOICECHAT
	DIK_N,
#endif
	DIK_V,
	DIK_J,
	DIK_B,
	DIK_N,
};

// If properly implemented, load default data externally on initial load,
// Should save to default and current key settings before processing, but
// Handled separately like this to process it while keeping the existing structure.
// Hotkeys and default values in external UI data must always match.
BYTE g_DefaultUIWrappingKeyData[UIWrappingKeyIndex_Amount] = {
	DIK_1,
	DIK_2,
	DIK_3,
	DIK_4,
	DIK_5,
	DIK_6,
	DIK_7,
	DIK_8,
	DIK_9,
	DIK_0,
	DIK_P,
	DIK_O,
	DIK_I,
	DIK_U,
	DIK_K,
	DIK_L,
	DIK_M,
	DIK_H,
	DIK_C,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	DIK_Y,
#endif
	DIK_GRAVE,
	DIK_T,
	DIK_Q,
	DIK_ESCAPE,
#ifdef _USE_VOICECHAT
	DIK_N,
#endif
	DIK_V,
	DIK_J,
	DIK_B,
};

//////////////////////////////////////////////////////////////////////////
//
//	*Joypad-related data*
//
//////////////////////////////////////////////////////////////////////////

WCHAR *g_szJoypadString[] = {
	L"LS_UP",
	L"LS_DOWN",
	L"LS_LEFT",
	L"LS_RIGHT",
	L"LS_ZOOMIN",
	L"LS_ZOOMOUT",
	L"RS_UP",
	L"RS_DOWN",
	L"RS_LEFT",
	L"RS_RIGHT",
	L"RS_ZOOMIN",
	L"RS_ZOOMOUT",
	L"MS_UP",
	L"MS_DOWN",
	L"MS_LEFT",
	L"MS_RIGHT",
	L"B0",
	L"B1",
	L"B2",
	L"B3",
	L"B4",
	L"B5",
	L"B6",
	L"B7",
	L"B8",
	L"B9",
	L"B10",
	L"B11",
	L"B12",
	L"B13",
	L"B14",
	L"B15",
	L"B16",
	L"B17",
	L"B18",
	L"B19",
	L"B20",
	L"B21",
	L"B22",
	L"B23",
	L"B24",
	L"B25",
	L"B26",
	L"B27",
	L"B28",
	L"B29",
	L"B30",
	L"B31",
	L"",
};


BYTE * g_WrappingJoypadData = g_WrappingCustomData;

BYTE * g_WrappingJoypadAssistData = g_WrappingCustomAssistData;

// XBOX360 Data
BYTE g_DefaultWrappingXBOX360Data[WrappingKeyIndex_Amount] = {
	CInputJoyPad::LS_UP,
	CInputJoyPad::LS_DOWN,
	CInputJoyPad::LS_LEFT,
	CInputJoyPad::LS_RIGHT,
	CInputJoyPad::B8,			//auto run
	CInputJoyPad::B0,			//jump
	CInputJoyPad::LS_ZOOMOUT,		//pick up
	CInputJoyPad::B7,			//rebirth
	CInputJoyPad::B9,			//turn
	CInputJoyPad::MS_UP,		//toggle mouse
	CInputJoyPad::NULL_VALUE,	//toggle drop item	
	CInputJoyPad::NULL_VALUE,			//degrease mouse
	CInputJoyPad::NULL_VALUE,			//increase mouse
	CInputJoyPad::B6,			//toggle battle
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::B5,			//aim assist
#endif
	CInputJoyPad::B2,			//normal attack
	CInputJoyPad::B3,			//special attack
	CInputJoyPad::B1,			//avoid
	CInputJoyPad::RS_LEFT,		//look left
	CInputJoyPad::RS_RIGHT,		//look right
	CInputJoyPad::RS_UP,		//look up
	CInputJoyPad::RS_DOWN,		//look down
	CInputJoyPad::NULL_VALUE,	//zoom in
	CInputJoyPad::NULL_VALUE	//zoom out
};

BYTE g_DefaultWrappingXBOX360AssistData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE
};

//PlayStation Data
BYTE g_DefaultWrappingPSData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE
};

BYTE g_DefaultWrappingPSAssistData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE
};

//HangamePad Data
BYTE g_DefaultWrappingHanData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::LS_UP,
	CInputJoyPad::LS_DOWN,
	CInputJoyPad::LS_LEFT,
	CInputJoyPad::LS_RIGHT,
	CInputJoyPad::B10,			//auto run
	CInputJoyPad::B0,			//jump
	CInputJoyPad::B7,		//pick up
	CInputJoyPad::B9,			//rebirth
	CInputJoyPad::B11,			//turn
	CInputJoyPad::MS_UP,		//toggle mouse
	CInputJoyPad::NULL_VALUE,	//toggle drop item	
	CInputJoyPad::NULL_VALUE,			//degrease mouse
	CInputJoyPad::NULL_VALUE,			//increase mouse
	CInputJoyPad::B8,			//toggle battle
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::B5,			//aim assist
#endif
	CInputJoyPad::B1,			//normal attack
	CInputJoyPad::B3,			//special attack
	CInputJoyPad::B2,			//avoid
	CInputJoyPad::LS_ZOOMOUT,		//look left
	CInputJoyPad::LS_ZOOMIN,		//look right
	CInputJoyPad::RS_ZOOMOUT,		//look up
	CInputJoyPad::RS_ZOOMIN,		//look down
	CInputJoyPad::NULL_VALUE,	//zoom in
	CInputJoyPad::NULL_VALUE	//zoom out
};

BYTE g_DefaultWrappingHanAssistData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE
};

//GAMMAC Data
BYTE g_DefaultWrappingGAMMACData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::LS_UP,
	CInputJoyPad::LS_DOWN,
	CInputJoyPad::LS_LEFT,
	CInputJoyPad::LS_RIGHT,
	CInputJoyPad::B10,			//auto run
	CInputJoyPad::B1,			//jump
	CInputJoyPad::B7,		//pick up
	CInputJoyPad::B9,			//rebirth
	CInputJoyPad::B11,			//turn
	CInputJoyPad::MS_UP,		//toggle mouse
	CInputJoyPad::NULL_VALUE,	//toggle drop item	
	CInputJoyPad::NULL_VALUE,			//degrease mouse
	CInputJoyPad::NULL_VALUE,			//increase mouse
	CInputJoyPad::B8,			//toggle battle
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::B5,			//aim assist
#endif
	CInputJoyPad::B0,			//normal attack
	CInputJoyPad::B3,			//special attack
	CInputJoyPad::B2,			//avoid
	CInputJoyPad::LS_ZOOMOUT,		//look left
	CInputJoyPad::LS_ZOOMIN,		//look right
	CInputJoyPad::RS_ZOOMOUT,		//look up
	CInputJoyPad::RS_ZOOMIN,		//look down
	CInputJoyPad::NULL_VALUE,	//zoom in
	CInputJoyPad::NULL_VALUE	//zoom out
};

BYTE g_DefaultWrappingGAMMACAssistData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE
};

//Custom Data
BYTE g_WrappingCustomData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE

};

BYTE g_WrappingCustomAssistData[WrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#if defined(PRE_ADD_AIM_ASSIST_BUTTON)
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE

};


BYTE * g_UIWrappingJoypadData = g_UIWrappingCustomData;

BYTE * g_UIWrappingJoypadAssistData = g_UIWrappingCustomAssistData;

//XBOX360 Data
BYTE g_DefaultUIWrappingXBOX360Data[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::B4,			//quick slot1
	CInputJoyPad::B4,			//quick slot2
	CInputJoyPad::B4,			//quick slot3
	CInputJoyPad::B4,			//quick slot4
	CInputJoyPad::B4,			//quick slot5
	CInputJoyPad::LS_ZOOMIN,		//quick slot6
	CInputJoyPad::LS_ZOOMIN,		//quick slot7
	CInputJoyPad::LS_ZOOMIN,		//quick slot8
	CInputJoyPad::LS_ZOOMIN,		//quick slot9
	CInputJoyPad::LS_ZOOMIN,		//quick slot10
	CInputJoyPad::MS_LEFT,	//char
	CInputJoyPad::MS_LEFT,	//community
	CInputJoyPad::MS_LEFT,	//inven
	CInputJoyPad::MS_LEFT,	//quest
	CInputJoyPad::MS_RIGHT,	//skill
	CInputJoyPad::MS_RIGHT,	//mission
	CInputJoyPad::MS_RIGHT,	//map
	CInputJoyPad::MS_RIGHT,	//help
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::MS_DOWN,	//quick slot change
	CInputJoyPad::NULL_VALUE,	//chat reply
	CInputJoyPad::NULL_VALUE,	//pvp tab
	CInputJoyPad::B4,		//System key
#ifdef _USE_VOICECHAT
	CInputJoyPad::NULL_VALUE,	//toggle mic
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

BYTE g_DefaultUIWrappingXBOX360AssistData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::B0,
	CInputJoyPad::B2,
	CInputJoyPad::B1,
	CInputJoyPad::B3,
	CInputJoyPad::B5,
	CInputJoyPad::B0,
	CInputJoyPad::B2,
	CInputJoyPad::B1,
	CInputJoyPad::B3,
	CInputJoyPad::B5,
	CInputJoyPad::B0,
	CInputJoyPad::B2,
	CInputJoyPad::B1,
	CInputJoyPad::B3,
	CInputJoyPad::B0,
	CInputJoyPad::B2,
	CInputJoyPad::B1,
	CInputJoyPad::B3,
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::B7,
#ifdef _USE_VOICECHAT
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

//PlayStation Data
BYTE g_DefaultUIWrappingPSData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef _USE_VOICECHAT
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

BYTE g_DefaultUIWrappingPSAssistData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef _USE_VOICECHAT
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

//HanGamePad Data
BYTE g_DefaultUIWrappingHanData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::B4,			//quick slot1
	CInputJoyPad::B4,			//quick slot2
	CInputJoyPad::B4,			//quick slot3
	CInputJoyPad::B4,			//quick slot4
	CInputJoyPad::B4,			//quick slot5
	CInputJoyPad::B6,		//quick slot6
	CInputJoyPad::B6,		//quick slot7
	CInputJoyPad::B6,		//quick slot8
	CInputJoyPad::B6,		//quick slot9
	CInputJoyPad::B6,		//quick slot10
	CInputJoyPad::MS_LEFT,	//char
	CInputJoyPad::MS_LEFT,	//community
	CInputJoyPad::MS_LEFT,	//inven
	CInputJoyPad::MS_LEFT,	//quest
	CInputJoyPad::MS_RIGHT,	//skill
	CInputJoyPad::MS_RIGHT,	//mission
	CInputJoyPad::MS_RIGHT,	//map
	CInputJoyPad::MS_RIGHT,	//help
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::MS_DOWN,	//quick slot change
	CInputJoyPad::NULL_VALUE,	//chat reply
	CInputJoyPad::NULL_VALUE,	//pvp tab
	CInputJoyPad::B12,		//System key
#ifdef _USE_VOICECHAT
	CInputJoyPad::LS_ZOOMOUT,	//toggle mic
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

BYTE g_DefaultUIWrappingHanAssistData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::B0,
	CInputJoyPad::B1,
	CInputJoyPad::B2,
	CInputJoyPad::B3,
	CInputJoyPad::B7,
	CInputJoyPad::B0,
	CInputJoyPad::B1,
	CInputJoyPad::B2,
	CInputJoyPad::B3,
	CInputJoyPad::B7,
	CInputJoyPad::B0,
	CInputJoyPad::B1,
	CInputJoyPad::B2,
	CInputJoyPad::B3,
	CInputJoyPad::B0,
	CInputJoyPad::B1,
	CInputJoyPad::B2,
	CInputJoyPad::B3,
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef _USE_VOICECHAT
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

//GAMMAC Data
BYTE g_DefaultUIWrappingGAMMACData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::B4,			//quick slot1
	CInputJoyPad::B4,			//quick slot2
	CInputJoyPad::B4,			//quick slot3
	CInputJoyPad::B4,			//quick slot4
	CInputJoyPad::B4,			//quick slot5
	CInputJoyPad::B6,		//quick slot6
	CInputJoyPad::B6,		//quick slot7
	CInputJoyPad::B6,		//quick slot8
	CInputJoyPad::B6,		//quick slot9
	CInputJoyPad::B6,		//quick slot10
	CInputJoyPad::MS_LEFT,	//char
	CInputJoyPad::MS_LEFT,	//community
	CInputJoyPad::MS_LEFT,	//inven
	CInputJoyPad::MS_LEFT,	//quest
	CInputJoyPad::MS_RIGHT,	//skill
	CInputJoyPad::MS_RIGHT,	//mission
	CInputJoyPad::MS_RIGHT,	//map
	CInputJoyPad::MS_RIGHT,	//help
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::MS_DOWN,	//quick slot change
	CInputJoyPad::NULL_VALUE,	//chat reply
	CInputJoyPad::NULL_VALUE,	//pvp tab
	CInputJoyPad::B12,		//System key
#ifdef _USE_VOICECHAT
	CInputJoyPad::LS_ZOOMOUT,	//toggle mic
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

BYTE g_DefaultUIWrappingGAMMACAssistData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::B1,
	CInputJoyPad::B0,
	CInputJoyPad::B2,
	CInputJoyPad::B3,
	CInputJoyPad::B7,
	CInputJoyPad::B1,
	CInputJoyPad::B0,
	CInputJoyPad::B2,
	CInputJoyPad::B3,
	CInputJoyPad::B7,
	CInputJoyPad::B0,
	CInputJoyPad::B1,
	CInputJoyPad::B2,
	CInputJoyPad::B3,
	CInputJoyPad::B0,
	CInputJoyPad::B1,
	CInputJoyPad::B2,
	CInputJoyPad::B3,
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef _USE_VOICECHAT
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

//Custom Data
BYTE g_UIWrappingCustomData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef _USE_VOICECHAT
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};

BYTE g_UIWrappingCustomAssistData[UIWrappingKeyIndex_Amount] = {
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
#ifdef _USE_VOICECHAT
	CInputJoyPad::NULL_VALUE,
#endif
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
	CInputJoyPad::NULL_VALUE,
};


void Initialize_InputWrapper()
{
	memset( g_DefaultWrappingXBOX360Data, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultWrappingXBOX360Data) );
	memset( g_DefaultWrappingXBOX360AssistData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultWrappingXBOX360AssistData) );
	memset( g_DefaultWrappingPSData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultWrappingPSData) );
	memset( g_DefaultWrappingPSAssistData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultWrappingPSAssistData) );
	memset( g_DefaultWrappingHanData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultWrappingHanData) );
	memset( g_DefaultWrappingHanAssistData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultWrappingHanAssistData) );
	memset( g_DefaultWrappingGAMMACData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultWrappingGAMMACData) );
	memset( g_DefaultWrappingGAMMACAssistData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultWrappingGAMMACAssistData) );
	memset( g_WrappingCustomData, CInputJoyPad::NULL_VALUE, sizeof(g_WrappingCustomData) );
	memset( g_WrappingCustomAssistData, CInputJoyPad::NULL_VALUE, sizeof(g_WrappingCustomAssistData) );

	memset( g_DefaultUIWrappingXBOX360Data, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultUIWrappingXBOX360Data) );
	memset( g_DefaultUIWrappingXBOX360AssistData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultUIWrappingXBOX360AssistData) );
	memset( g_DefaultUIWrappingPSData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultUIWrappingPSData) );
	memset( g_DefaultUIWrappingPSAssistData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultUIWrappingPSAssistData) );
	memset( g_DefaultUIWrappingHanData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultUIWrappingHanData) );
	memset( g_DefaultUIWrappingHanAssistData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultUIWrappingHanAssistData) );
	memset( g_DefaultUIWrappingGAMMACData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultUIWrappingGAMMACData) );
	memset( g_DefaultUIWrappingGAMMACAssistData, CInputJoyPad::NULL_VALUE, sizeof(g_DefaultUIWrappingGAMMACAssistData) );
	memset( g_UIWrappingCustomData, CInputJoyPad::NULL_VALUE, sizeof(g_UIWrappingCustomData) );
	memset( g_UIWrappingCustomAssistData, CInputJoyPad::NULL_VALUE, sizeof(g_UIWrappingCustomAssistData) );
}

void _ResetDefault()
{
	// Initialize all
	memcpy_s( g_WrappingKeyData, sizeof(g_WrappingKeyData), g_DefaultWrappingKeyData, sizeof(g_DefaultWrappingKeyData) );
	memcpy_s( g_UIWrappingKeyData, sizeof(g_UIWrappingKeyData), g_DefaultUIWrappingKeyData, sizeof(g_DefaultUIWrappingKeyData) );
}