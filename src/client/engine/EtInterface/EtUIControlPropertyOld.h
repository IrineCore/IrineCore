#pragma once

#define _ANIMATION_FRAME_CONTROL

struct SUIControlProperty_01
{
	enum { VERSION = 0x101 };

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[32];
	int nCustomControlID;
};

struct SUIControlProperty_02
{
	enum { VERSION = 0x102 };

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[32];
	int nCustomControlID;
};

struct SUIControlProperty_03
{
	enum { VERSION = 0x103 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
	};
};

struct SUIControlProperty_04
{
	enum { VERSION = 0x104 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
	};
};

struct SUIControlProperty_05
{
	enum { VERSION = 0x105 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};
	struct SUIAnimationProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
		SUIAnimationProperty AnimationProperty;
	};
};

struct SUIControlProperty_06
{
	enum { VERSION = 0x106 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};
	struct SUIAnimationProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
		SUIAnimationProperty AnimationProperty;
	};

	union
	{
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;	// Note : CEtHtmlTextBox inherits from CEtUITextBox.
	};
};

struct SUIControlProperty_07
{
	enum { VERSION = 0x107 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};
	struct SUIAnimationProperty{};
	struct SUILineEditBoxProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
		SUIAnimationProperty AnimationProperty;
		SUILineEditBoxProperty LineEditBoxProperty;
	};

	union
	{
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;	// Note : CEtHtmlTextBox inherits from CEtUITextBox.
	};
};

struct SUIControlProperty_08
{
	enum { VERSION = 0x108 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};
	struct SUIAnimationProperty{};
	struct SUILineEditBoxProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
		SUIAnimationProperty AnimationProperty;
		SUILineEditBoxProperty LineEditBoxProperty;
	};

	union
	{
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;	// Note : CEtHtmlTextBox inherits from CEtUITextBox.
	};
};

struct SUIControlProperty_09
{
	enum { VERSION = 0x109 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUICustomProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};
	struct SUIAnimationProperty{};
	struct SUILineEditBoxProperty{};
	struct SUILineIMEEditBoxProperty{};
	struct SUIListBoxExProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUICustomProperty CustomProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
		SUIAnimationProperty AnimationProperty;
		SUILineEditBoxProperty LineEditBoxProperty;
		SUILineIMEEditBoxProperty LineIMEEditBoxProperty;
		SUIListBoxExProperty ListBoxExProperty;
	};

	union
	{
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;	// Note : CEtHtmlTextBox inherits from CEtUITextBox.
	};
};

struct SUIControlProperty_10
{
	enum { VERSION = 0x110 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUICustomProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};
	struct SUIAnimationProperty{};
	struct SUILineEditBoxProperty{};
	struct SUILineIMEEditBoxProperty{};
	struct SUIListBoxExProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;
	int nInitState;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUICustomProperty CustomProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
		SUIAnimationProperty AnimationProperty;
		SUILineEditBoxProperty LineEditBoxProperty;
		SUILineIMEEditBoxProperty LineIMEEditBoxProperty;
		SUIListBoxExProperty ListBoxExProperty;
	};

	union
	{
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;	// Note : CEtHtmlTextBox inherits from CEtUITextBox.
	};
};

struct SUIControlProperty_11
{
	enum { VERSION = 0x111 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUICustomProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};
	struct SUIAnimationProperty{};
	struct SUILineEditBoxProperty{};
	struct SUILineIMEEditBoxProperty{};
	struct SUIListBoxExProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;
	int nInitState;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUICustomProperty CustomProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
		SUIAnimationProperty AnimationProperty;
		SUILineEditBoxProperty LineEditBoxProperty;
		SUILineIMEEditBoxProperty LineIMEEditBoxProperty;
		SUIListBoxExProperty ListBoxExProperty;
	};

	union
	{
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;	// Note : CEtHtmlTextBox inherits from CEtUITextBox.
	};
};

struct SUIControlProperty_12
{
	enum { VERSION = 0x112 };

	struct SUIStaticProperty{};
	struct SUIButtonProperty{};
	struct SUICheckBoxProperty{};
	struct SUIRadioButtonProperty{};
	struct SUISliderProperty{};
	struct SUIEditBoxProperty{};
	struct SUIIMEEditBoxProperty{};
	struct SUIComboBoxProperty{};
	struct SUIListBoxProperty{};
	struct SUIProgressBarProperty{};
	struct SUICustomProperty{};
	struct SUITextBoxProperty{};
	struct SUIHtmlTextBoxProperty{};
	struct SUITextureControlProperty{};
	struct SUITreeControlProperty{};
	struct SUIScrollBarProperty{};
	struct SUIQuestTreeControlProperty{};
	struct SUIAnimationProperty{};
	struct SUILineEditBoxProperty{};
	struct SUILineIMEEditBoxProperty{};
	struct SUIListBoxExProperty{};

	UI_CONTROL_TYPE UIType;
	int nID;
	int nHotKey;
	int nTemplateIndex;
	BOOL bDefaultControl;
	SUICoord UICoord;
	char szUIName[ 32 ];
	int nCustomControlID;
	int nTooltipStringIndex;
	int nInitState;

	union
	{
		SUIStaticProperty StaticProperty;
		SUIButtonProperty ButtonProperty;
		SUICheckBoxProperty CheckBoxProperty;
		SUIRadioButtonProperty RadioButtonProperty;
		SUISliderProperty SliderProperty;
		SUIEditBoxProperty EditBoxProperty;
		SUIIMEEditBoxProperty IMEEditBoxProperty;
		SUIComboBoxProperty ComboBoxProperty;
		SUIListBoxProperty ListBoxProperty;
		SUIProgressBarProperty ProgressBarProperty;
		SUICustomProperty CustomProperty;
		SUITextBoxProperty TextBoxProperty;
		SUITextureControlProperty TextureControlProperty;
		SUITreeControlProperty TreeControlProperty;
		SUIScrollBarProperty ScrollBarProperty;
		SUIQuestTreeControlProperty QuestTreeControlProperty;
		SUIAnimationProperty AnimationProperty;
		SUILineEditBoxProperty LineEditBoxProperty;
		SUILineIMEEditBoxProperty LineIMEEditBoxProperty;
		SUIListBoxExProperty ListBoxExProperty;
	};

	union
	{
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;	// Note : CEtHtmlTextBox inherits from CEtUITextBox.
	};
};
///////////////////////////////////////////////////////////

struct SUIButtonProperty_01
{
	enum { VERSION = 0x101 };

	int nStringIndex;
	float fMoveOffsetHori;
	float fMoveOffsetVert;
	UIButtonType ButtonType;
};

struct SUIButtonProperty_02
{
	enum { VERSION = 0x102 };

	int nStringIndex;
	float fMoveOffsetHori;
	float fMoveOffsetVert;
	UIButtonType ButtonType;
	char szSoundFileNameClick[_MAX_PATH];
};

struct SUIButtonProperty_03
{
	enum { VERSION = 0x103 };

	int nStringIndex;
	float fMoveOffsetHori;
	float fMoveOffsetVert;
	UIButtonType ButtonType;
	UIButtonSound ButtonSound;
};

struct SUIButtonProperty_04
{
	enum { VERSION = 0x104 };

	int nStringIndex;
	float fMoveOffsetHori;
	float fMoveOffsetVert;
	UIButtonType ButtonType;
	UIButtonSound ButtonSound;
	bool bVariableWidth;
	float fVariableWidthLength;
};

struct SUIButtonProperty_05
{
	enum { VERSION = 0x105 };

	int nStringIndex;
	float fMoveOffsetHori;
	float fMoveOffsetVert;
	UIButtonType ButtonType;
	UIButtonSound ButtonSound;
	bool bVariableWidth;
	float fVariableWidthLength;
	int nButtonID;
};

struct SUIButtonProperty_06
{
	enum { VERSION = 0x106 };

	int nStringIndex;
	float fMoveOffsetHori;
	float fMoveOffsetVert;
	UIButtonType ButtonType;
	bool bVariableWidth;
	float fVariableWidthLength;
	int nButtonID;
	char szSoundFileName[32];
};

struct SUIRadioButtonProperty_01
{
	enum { VERSION = 0x101 };

	int nStringIndex;
	int nButtonGroup;
	float fSpace;
	int nAllign;
	int nTabID;
};

struct SUIRadioButtonProperty_02
{
	enum { VERSION = 0x102 };

	int nStringIndex;
	int nButtonGroup;
	float fSpace;
	int nAllign;
	int nTabID;
	int nIconIndex;
};

struct SUIRadioButtonProperty_03
{
	enum { VERSION = 0x103 };

	int nStringIndex;
	int nButtonGroup;
	float fSpace;
	int nAllign;
	int nTabID;
	int nIconIndex;
	float fFontHoriOffset;
	float fFontVertOffset;
};

struct SUIRadioButtonProperty_04
{
	enum { VERSION = 0x104 };

	int nStringIndex;
	int nButtonGroup;
	float fSpace;
	int nAllign;
	int nTabID;
	int nIconIndex;
};

struct SUIRadioButtonProperty_05
{
	enum { VERSION = 0x105 };

	int nStringIndex;
	int nButtonGroup;
	float fSpace;
	int nAllign;
	int nTabID;
	int nIconIndex;
	char szSoundFileName[32];
};

struct SUIRadioButtonProperty_06
{
	enum { VERSION = 0x106 };

	int nStringIndex;
	int nButtonGroup;
	float fSpace;
	int nAllign;
	int nTabID;
	int nIconIndex;
	char szSoundFileName[32];
	float fMoveOffsetHori;
	float fMoveOffsetVert;
};

struct SUIRadioButtonProperty_07
{
	enum { VERSION = 0x107 };

	int nStringIndex;
	int nButtonGroup;
	float fSpace;
	int nAllign;
	int nTabID;
	int nIconIndex;
	char szSoundFileName[32];
	float fMoveOffsetHori;
	float fMoveOffsetVert;
	float fVariableWidthLength;
};

struct SUIEditBoxProperty_01
{
	enum { VERSION = 0x101 };

	float fSpace;
	float fBorder;
	DWORD dwTextColor;
	DWORD dwSelTextColor;
	DWORD dwSelBackColor;
	DWORD dwCaretColor;
	BOOL bPassword;
};

struct SUIEditBoxProperty_02
{
	enum { VERSION = 0x102 };

	float fSpace;
	float fBorder;
	DWORD dwTextColor;
	DWORD dwSelTextColor;
	DWORD dwSelBackColor;
	DWORD dwCaretColor;
	BOOL bPassword;
	DWORD dwMaxChars;
};

struct SUIIMEEditBoxProperty_01
{
	enum { VERSION = 0x101 };

	float fSpace;
	float fBorder;
	DWORD dwTextColor;
	DWORD dwSelTextColor;
	DWORD dwSelBackColor;
	DWORD dwCaretColor;
	BOOL bPassword;
	float fIndicatorSize;
	DWORD dwCompTextColor;
	DWORD dwCompBackColor;
	DWORD dwCompCaretColor;
	DWORD dwCandiTextColor;
	DWORD dwCandiBackColor;
	DWORD dwCandiSelTextColor;
	DWORD dwCandiSelBackColor;
	DWORD dwIndiTextColor;
	DWORD dwReadingTextColor;
	DWORD dwReadingBackColor;
	DWORD dwReadingSelTextColor;
	DWORD dwReadingSelBackColor;
};

struct SUITextBoxProperty_01
{
	enum { VERSION = 0x101 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bUseMinThumbSize;
};

struct SUITextBoxProperty_02
{
	enum { VERSION = 0x102 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bUseMinThumbSize;
	bool bVariableTextBox;
	float fLineSpace;
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
};

struct SUITextBoxProperty_03
{
	enum { VERSION = 0x103 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bUseMinThumbSize;
	float fLineSpace;
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	UITextBoxVariableType VariableType;
};

struct SUITextBoxProperty_04
{
	enum { VERSION = 0x104 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bUseMinThumbSize;
	float fLineSpace;
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	UITextBoxVariableType VariableType;
	bool bRollOver;
};

struct SUITextBoxProperty_05
{
	enum { VERSION = 0x105 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bFixThumb;
	float fThumbSize;
	float fLineSpace;
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	UITextBoxVariableType VariableType;
	bool bRollOver;
};

struct SUITextBoxProperty_06
{
	enum { VERSION = 0x106 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	float fLineSpace;
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	UITextBoxVariableType VariableType;
	bool bRollOver;
};

struct SUIHtmlTextBoxProperty_01
{
	enum { VERSION = 0x101 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bUseMinThumbSize;
	bool bVariableTextBox;
	float fLineSpace;
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
};

struct SUIHtmlTextBoxProperty_02
{
	enum { VERSION = 0x102 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bFixThumb;
	float fThumbSize;
	bool bVariableTextBox;
	float fLineSpace;
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
};

struct SUIHtmlTextBoxProperty_03
{
	enum { VERSION = 0x103 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bVariableTextBox;
	float fLineSpace;
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
};

struct SUIHtmlTextBoxProperty_04
{
	enum { VERSION = 0x104 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
};

struct SUITreeControlProperty_01
{
	enum { VERSION = 0x101 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bUseMinThumbSize;
	float fLineSpace;
};

struct SUITreeControlProperty_02
{
	enum { VERSION = 0x102 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bUseMinThumbSize;
	float fLineSpace;
	float fMainButtonSize;
	float fSubButtonSize;
	float fIndentSize;
};

struct SUITreeControlProperty_03
{
	enum { VERSION = 0x103 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bUseMinThumbSize;
	float fLineSpace;
	float fMainButtonSize;
	float fSubButtonSize;
	float fIndentSize;
	float fBorderSize;
};

struct SUITreeControlProperty_04
{
	enum { VERSION = 0x104 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bFixThumb;
	float fThumbSize;
	float fLineSpace;
	float fMainButtonSize;
	float fSubButtonSize;
	float fIndentSize;
	float fBorderSize;
};

struct SUITreeControlProperty_05
{
	enum { VERSION = 0x105 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	bool bFixThumb;
	float fThumbSize;
	float fLineSpace;
	float fMainButtonSize;
	float fSubButtonSize;
	float fIndentSize;
	float fBorderSize;
	float fStateSize;
};

struct SUITreeControlProperty_06
{
	enum { VERSION = 0x106 };

	bool bVerticalScrollBar;
	float fScrollBarSize;
	int nScrollBarTemplate;
	float fLineSpace;
	float fMainButtonSize;
	float fSubButtonSize;
	float fIndentSize;
	float fBorderSize;
	float fStateSize;
};

struct SUIScrollBarProperty_01
{
	enum { VERSION = 0x101 };

	float fThumbSize;
	bool bfixableThumb;
};

struct SUIScrollBarProperty_02
{
	enum { VERSION = 0x102 };

	bool bfixableThumb;
};

struct SUIComboBoxProperty_01
{
	enum { VERSION = 0x101 };

	float fMoveOffsetHori;
	float fMoveOffsetVert;
	float fDropdownSize;
	float fScrollBarSize;
	int nScrollBarTemplate;
};

struct SUIComboBoxProperty_02
{
	enum { VERSION = 0x102 };

	float fMoveOffsetHori;
	float fMoveOffsetVert;
	float fDropdownSize;
	float fScrollBarSize;
	int nScrollBarTemplate;
	float fVariableWidthLength;
};

struct SUIListBoxProperty_01
{
	enum { VERSION = 0x101 };

	float fScrollBarSize;
	int nScrollBarTemplate;
	BOOL bMultiSelect;
	float fBorder;
	float fMargin;
};

struct SUIStaticProperty_01
{
	enum { VERSION = 0x101 };

	int nStringIndex;
	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
};

struct SUIStaticProperty_02
{
	enum { VERSION = 0x102 };

	int nStringIndex;
	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
	bool bVariableWidth;
};

struct SUIStaticProperty_03
{
	enum { VERSION = 0x103 };

	int nStringIndex;
	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
	bool bVariableWidth;
	float fVariableWidthLength;
};

struct SUIStaticProperty_04
{
	enum { VERSION = 0x104 };

	int nStringIndex;
	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
};

struct SUIStaticProperty_05
{
	enum { VERSION = 0x105 };

	int nStringIndex;
	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	DWORD dwShadowFontColor;
};

struct SUIStaticProperty_06
{
	enum { VERSION = 0x106 };

	int nStringIndex;
	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	DWORD dwShadowFontColor;
	bool bProcessInput;
};

struct SUIStaticProperty_07
{
	enum { VERSION = 0x107 };

	int nStringIndex;
	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	DWORD dwShadowFontColor;
	bool bProcessInput;
	DWORD dwTextureColor;
};


struct SUIStaticProperty_08
{
	enum { VERSION = 0x108 };

	int nStringIndex;
	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	DWORD dwShadowFontColor;
	bool bProcessInput;
	DWORD dwTextureColor;
	bool bAutoTooltip;
};
struct SUILineEditBoxProperty_01
{
	enum { VERSION = 0x101 };

	DWORD dwTextColor;
	DWORD dwSelTextColor;
	DWORD dwSelBackColor;
	DWORD dwCaretColor;
};

struct SUILineIMEEditBoxProperty_01
{
	enum { VERSION = 0x101 };

	DWORD dwTextColor;
	DWORD dwSelTextColor;
	DWORD dwSelBackColor;
	DWORD dwCaretColor;
	DWORD dwCompTextColor;
	DWORD dwCompBackColor;
	DWORD dwCompCaretColor;
	DWORD dwCandiTextColor;
	DWORD dwCandiBackColor;
	DWORD dwCandiSelTextColor;
	DWORD dwCandiSelBackColor;
	DWORD dwReadingTextColor;
	DWORD dwReadingBackColor;
	DWORD dwReadingSelTextColor;
	DWORD dwReadingSelBackColor;
};

struct SUITextureControlProperty_01
{
	enum { VERSION = 0x101 };

	int nStringIndex;

	DWORD dwFontFormat;
	BOOL bShadowFont;
	DWORD dwFontColor;
};

struct SUIListBoxExProperty_01
{
	enum { VERSION = 0x101 };

	float fScrollBarSize;
	int nScrollBarTemplate;
	BOOL bMultiSelect;
	float fBorder;
	float fMargin;
	float fLineSpace;
	char szListBoxItemUIFileName[32];
};

#ifdef _ANIMATION_FRAME_CONTROL
struct SUIAnimationInfoProperty_01
{
	enum { VERSION = 0x101 };

	bool bLoop;
};
#endif