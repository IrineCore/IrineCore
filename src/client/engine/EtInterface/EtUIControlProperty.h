#pragma once
#include "DebugSet.h"
#include "EtUIControlPropertyOld.h"

#define _ANIMATION_FRAME_CONTROL


struct SUIControlProperty
{
	enum { VERSION = 0x113 };

	struct SUIStaticProperty
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

		void Load_01( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> dwFontFormat;
			stream >> bShadowFont;
			stream >> dwFontColor;
		}

		void Load_02( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> dwFontFormat;
			stream >> bShadowFont;
			stream >> dwFontColor;
			stream.SeekCur<bool>();
		}

		void Load_03( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> dwFontFormat;
			stream >> bShadowFont;
			stream >> dwFontColor;
			stream.SeekCur<bool>();
			stream.SeekCur<float>();
		}

		void Load_04( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> dwFontFormat;
			stream >> bShadowFont;
			stream >> dwFontColor;
			stream >> nFrameLeft;
			stream >> nFrameTop;
			stream >> nFrameRight;
			stream >> nFrameBottom;
		}

		void Load_05( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> dwFontFormat;
			stream >> bShadowFont;
			stream >> dwFontColor;
			stream >> nFrameLeft;
			stream >> nFrameTop;
			stream >> nFrameRight;
			stream >> nFrameBottom;
			stream >> dwShadowFontColor;
		}

		void Load_06( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> dwFontFormat;
			stream >> bShadowFont;
			stream >> dwFontColor;
			stream >> nFrameLeft;
			stream >> nFrameTop;
			stream >> nFrameRight;
			stream >> nFrameBottom;
			stream >> dwShadowFontColor;
			stream >> bProcessInput;
			// Newly added texture color value; default is 0xFFFFFFFF, so set it here.
			// Strictly speaking this should be done for the above versions too, but skipping since they are expected to be rare.
			dwTextureColor = 0xFFFFFFFF;
		}

		void Load_07( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> dwFontFormat;
			stream >> bShadowFont;
			stream >> dwFontColor;
			stream >> nFrameLeft;
			stream >> nFrameTop;
			stream >> nFrameRight;
			stream >> nFrameBottom;
			stream >> dwShadowFontColor;
			stream >> bProcessInput;
			stream >> dwTextureColor;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIStaticProperty_01::VERSION: Load_01(stream); break;
			case SUIStaticProperty_02::VERSION: Load_02(stream); break;
			case SUIStaticProperty_03::VERSION: Load_03(stream); break;
			case SUIStaticProperty_04::VERSION: Load_04(stream); break;
			case SUIStaticProperty_05::VERSION: Load_05(stream); break;
			case SUIStaticProperty_06::VERSION: Load_06(stream); break;
			case SUIStaticProperty_07::VERSION: Load_07(stream); break;
			case VERSION:
				{
					stream >> nStringIndex;
					stream >> dwFontFormat;
					stream >> bShadowFont;
					stream >> dwFontColor;
					stream >> nFrameLeft;
					stream >> nFrameTop;
					stream >> nFrameRight;
					stream >> nFrameBottom;
					stream >> dwShadowFontColor;
					stream >> bProcessInput;
					stream >> dwTextureColor;
					stream >> bAutoTooltip;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIStaticProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << nStringIndex;
			stream << dwFontFormat;
			stream << bShadowFont;
			stream << dwFontColor;
			stream << nFrameLeft;
			stream << nFrameTop;
			stream << nFrameRight;
			stream << nFrameBottom;
			stream << dwShadowFontColor;
			stream << bProcessInput;
			stream << dwTextureColor;
			stream << bAutoTooltip;
		}
	};

	struct SUIButtonProperty
	{
		enum { VERSION = 0x107 };

		int nStringIndex;
		float fMoveOffsetHori;
		float fMoveOffsetVert;
		UIButtonType ButtonType;
		//UIButtonSound ButtonSound;
		bool bVariableWidth;
		float fVariableWidthLength;
		int nButtonID;
		char szSoundFileName[32];
		bool bAutoTooltip;

		void Load_01( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> ButtonType;
		}

		void Load_02( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> ButtonType;
			//stream.Seek( sizeof(char)*_MAX_PATH, SEEK_CUR );
			stream.ReadBuffer( szSoundFileName, _countof(szSoundFileName) );
		}

		void Load_03( CStream &stream )
		{
			UIButtonSound _ButtonSound;

			stream >> nStringIndex;
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> ButtonType;
			stream >> _ButtonSound;

			// Changed from enum-based to filename-based, so convert and store it like this.
			SoundIndexToFileName( _ButtonSound );
		}

		void Load_04( CStream &stream )
		{
			UIButtonSound _ButtonSound;

			stream >> nStringIndex;
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> ButtonType;
			stream >> _ButtonSound;
			stream >> bVariableWidth;
			stream >> fVariableWidthLength;

			SoundIndexToFileName( _ButtonSound );
		}

		void Load_05( CStream &stream )
		{
			UIButtonSound _ButtonSound;

			stream >> nStringIndex;
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> ButtonType;
			stream >> _ButtonSound;
			stream >> bVariableWidth;
			stream >> fVariableWidthLength;
			stream >> nButtonID;

			SoundIndexToFileName( _ButtonSound );
		}

		void Load_06( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> ButtonType;
			stream >> bVariableWidth;
			stream >> fVariableWidthLength;
			stream >> nButtonID;
			stream.ReadBuffer( szSoundFileName, _countof(szSoundFileName) );
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIButtonProperty_01::VERSION: Load_01( stream ); break;
			case SUIButtonProperty_02::VERSION: Load_02( stream ); break;
			case SUIButtonProperty_03::VERSION: Load_03( stream ); break;
			case SUIButtonProperty_04::VERSION: Load_04( stream ); break;
			case SUIButtonProperty_05::VERSION: Load_05( stream ); break;
			case SUIButtonProperty_06::VERSION: Load_06( stream ); break;
			case VERSION:
				{
					stream >> nStringIndex;
					stream >> fMoveOffsetHori;
					stream >> fMoveOffsetVert;
					stream >> ButtonType;
					stream >> bVariableWidth;
					stream >> fVariableWidthLength;
					stream >> nButtonID;
					stream.ReadBuffer( szSoundFileName, _countof(szSoundFileName) );
					stream >> bAutoTooltip;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIButtonProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << nStringIndex;
			stream << fMoveOffsetHori;
			stream << fMoveOffsetVert;
			stream << ButtonType;
			stream << bVariableWidth;
			stream << fVariableWidthLength;
			stream << nButtonID;
			stream.WriteBuffer( szSoundFileName, _countof(szSoundFileName) );
			stream << bAutoTooltip;
		}

		void SoundIndexToFileName( UIButtonSound emSound )
		{
			switch( emSound )
			{
			case UI_BUTTON_NONE:	sprintf_s( szSoundFileName, _countof(szSoundFileName) , "" ); break;
			case UI_BUTTON_OK:		sprintf_s( szSoundFileName, _countof(szSoundFileName) , "UI_button_ok.wav" ); break;
			case UI_BUTTON_CANCEL:	sprintf_s( szSoundFileName, _countof(szSoundFileName) , "UI_button_cancel.wav" ); break;
			case UI_BUTTON_PAGE:	sprintf_s( szSoundFileName, _countof(szSoundFileName) , "UI_page_a.wav" ); break;
			}
		}
	};

	struct SUICheckBoxProperty
	{
		enum { VERSION = 0x101 };

		int nStringIndex;

		float fSpace;
		int nAllign;

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case VERSION:
				{
					stream >> nStringIndex;
					stream >> fSpace;
					stream >> nAllign;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUICheckBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << nStringIndex;
			stream << fSpace;
			stream << nAllign;
		}
	};

	struct SUIRadioButtonProperty
	{
		enum { VERSION = 0x108 };

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
		bool bAutoTooltip;

		void Load_01( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> nButtonGroup;
			stream >> fSpace;
			stream >> nAllign;
			stream >> nTabID;
		}

		void Load_02( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> nButtonGroup;
			stream >> fSpace;
			stream >> nAllign;
			stream >> nTabID;
			stream >> nIconIndex;
		}

		void Load_03( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> nButtonGroup;
			stream >> fSpace;
			stream >> nAllign;
			stream >> nTabID;
			stream >> nIconIndex;
			stream.SeekCur<float>();
			stream.SeekCur<float>();
		}

		void Load_04( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> nButtonGroup;
			stream >> fSpace;
			stream >> nAllign;
			stream >> nTabID;
			stream >> nIconIndex;
		}

		void Load_05( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> nButtonGroup;
			stream >> fSpace;
			stream >> nAllign;
			stream >> nTabID;
			stream >> nIconIndex;
			stream.ReadBuffer( szSoundFileName, _countof(szSoundFileName) );
		}

		void Load_06( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> nButtonGroup;
			stream >> fSpace;
			stream >> nAllign;
			stream >> nTabID;
			stream >> nIconIndex;
			stream.ReadBuffer( szSoundFileName, _countof(szSoundFileName) );
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
		}

		void Load_07( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> nButtonGroup;
			stream >> fSpace;
			stream >> nAllign;
			stream >> nTabID;
			stream >> nIconIndex;
			stream.ReadBuffer( szSoundFileName, _countof(szSoundFileName) );
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> fVariableWidthLength;
		}


		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIRadioButtonProperty_01::VERSION: Load_01(stream); break;
			case SUIRadioButtonProperty_02::VERSION: Load_02(stream); break;
			case SUIRadioButtonProperty_03::VERSION: Load_03(stream); break;
			case SUIRadioButtonProperty_04::VERSION: Load_04(stream); break;
			case SUIRadioButtonProperty_05::VERSION: Load_05(stream); break;
			case SUIRadioButtonProperty_06::VERSION: Load_06(stream); break;
			case SUIRadioButtonProperty_07::VERSION: Load_07(stream); break;
			case VERSION:
				{
					stream >> nStringIndex;
					stream >> nButtonGroup;
					stream >> fSpace;
					stream >> nAllign;
					stream >> nTabID;
					stream >> nIconIndex;
					stream.ReadBuffer( szSoundFileName, _countof(szSoundFileName) );
					stream >> fMoveOffsetHori;
					stream >> fMoveOffsetVert;
					stream >> fVariableWidthLength;
					stream >> bAutoTooltip;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIRadioButtonProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << nStringIndex;
			stream << nButtonGroup;
			stream << fSpace;
			stream << nAllign;
			stream << nTabID;
			stream << nIconIndex;
			stream.WriteBuffer( szSoundFileName, _countof(szSoundFileName) );
			stream << fMoveOffsetHori;
			stream << fMoveOffsetVert;
			stream << fVariableWidthLength;
			stream << bAutoTooltip;
		}
	};

	struct SUISliderProperty
	{
		enum { VERSION = 0x101 };

		int nMinValue;
		int nMaxValue;

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case VERSION:
				{
					stream >> nMinValue;
					stream >> nMaxValue;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUISliderProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << nMinValue;
			stream << nMaxValue;
		}
	};

	struct SUIEditBoxProperty
	{
		enum { VERSION = 0x103 };

		float fSpace;
		float fBorder;
		DWORD dwTextColor;
		DWORD dwSelTextColor;
		DWORD dwSelBackColor;
		DWORD dwCaretColor;
		BOOL bPassword;
		DWORD dwMaxChars;
		BOOL bOnlyDigit;

		void Load_01( CStream &stream )
		{
			stream >> fSpace;
			stream >> fBorder;
			stream >> dwTextColor;
			stream >> dwSelTextColor;
			stream >> dwSelBackColor;
			stream >> dwCaretColor;
			stream >> bPassword;
		}

		void Load_02( CStream &stream )
		{
			stream >> fSpace;
			stream >> fBorder;
			stream >> dwTextColor;
			stream >> dwSelTextColor;
			stream >> dwSelBackColor;
			stream >> dwCaretColor;
			stream >> bPassword;
			stream >> dwMaxChars;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIEditBoxProperty_01::VERSION: Load_01(stream); break;
			case SUIEditBoxProperty_02::VERSION: Load_02(stream); break;
			case VERSION:
				{
					stream >> fSpace;
					stream >> fBorder;
					stream >> dwTextColor;
					stream >> dwSelTextColor;
					stream >> dwSelBackColor;
					stream >> dwCaretColor;
					stream >> bPassword;
					stream >> dwMaxChars;
					stream >> bOnlyDigit;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIEditBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << fSpace;
			stream << fBorder;
			stream << dwTextColor;
			stream << dwSelTextColor;
			stream << dwSelBackColor;
			stream << dwCaretColor;
			stream << bPassword;
			stream << dwMaxChars;
			stream << bOnlyDigit;
		}
	};

	struct SUIIMEEditBoxProperty
	{
		enum { VERSION = 0x102 };

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
		DWORD dwMaxChars;

		void Load_01( CStream &stream )
		{
			stream >> fSpace;
			stream >> fBorder;
			stream >> dwTextColor;
			stream >> dwSelTextColor;
			stream >> dwSelBackColor;
			stream >> dwCaretColor;
			stream >> bPassword;
			stream >> fIndicatorSize;
			stream >> dwCompTextColor;
			stream >> dwCompBackColor;
			stream >> dwCompCaretColor;
			stream >> dwCandiTextColor;
			stream >> dwCandiBackColor;
			stream >> dwCandiSelTextColor;
			stream >> dwCandiSelBackColor;
			stream >> dwIndiTextColor;
			stream >> dwReadingTextColor;
			stream >> dwReadingBackColor;
			stream >> dwReadingSelTextColor;
			stream >> dwReadingSelBackColor;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIIMEEditBoxProperty_01::VERSION: Load_01(stream); break;
			case VERSION:
				{
					stream >> fSpace;
					stream >> fBorder;
					stream >> dwTextColor;
					stream >> dwSelTextColor;
					stream >> dwSelBackColor;
					stream >> dwCaretColor;
					stream >> bPassword;
					stream >> fIndicatorSize;
					stream >> dwCompTextColor;
					stream >> dwCompBackColor;
					stream >> dwCompCaretColor;
					stream >> dwCandiTextColor;
					stream >> dwCandiBackColor;
					stream >> dwCandiSelTextColor;
					stream >> dwCandiSelBackColor;
					stream >> dwIndiTextColor;
					stream >> dwReadingTextColor;
					stream >> dwReadingBackColor;
					stream >> dwReadingSelTextColor;
					stream >> dwReadingSelBackColor;
					stream >> dwMaxChars;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIIMEEditBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << fSpace;
			stream << fBorder;
			stream << dwTextColor;
			stream << dwSelTextColor;
			stream << dwSelBackColor;
			stream << dwCaretColor;
			stream << bPassword;
			stream << fIndicatorSize;
			stream << dwCompTextColor;
			stream << dwCompBackColor;
			stream << dwCompCaretColor;
			stream << dwCandiTextColor;
			stream << dwCandiBackColor;
			stream << dwCandiSelTextColor;
			stream << dwCandiSelBackColor;
			stream << dwIndiTextColor;
			stream << dwReadingTextColor;
			stream << dwReadingBackColor;
			stream << dwReadingSelTextColor;
			stream << dwReadingSelBackColor;
			stream << dwMaxChars;
		}
	};

	struct SUIComboBoxProperty
	{
		enum { VERSION = 0x103 };

		float fMoveOffsetHori;
		float fMoveOffsetVert;
		float fDropdownSize;
		float fScrollBarSize;
		int nScrollBarTemplate;
		float fVariableWidthLength;
		float fLineSpace;

		void Load_01( CStream &stream )
		{
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> fDropdownSize;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
		}

		void Load_02( CStream &stream )
		{
			stream >> fMoveOffsetHori;
			stream >> fMoveOffsetVert;
			stream >> fDropdownSize;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream >> fVariableWidthLength;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIComboBoxProperty_01::VERSION: Load_01(stream); break;
			case SUIComboBoxProperty_02::VERSION: Load_02(stream); break;
			case VERSION:
				{
					stream >> fMoveOffsetHori;
					stream >> fMoveOffsetVert;
					stream >> fDropdownSize;
					stream >> fScrollBarSize;
					stream >> nScrollBarTemplate;
					stream >> fVariableWidthLength;
					stream >> fLineSpace;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIComboBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << fMoveOffsetHori;
			stream << fMoveOffsetVert;
			stream << fDropdownSize;
			stream << fScrollBarSize;
			stream << nScrollBarTemplate;
			stream << fVariableWidthLength;
			stream << fLineSpace;
		}
	};

	struct SUIListBoxProperty
	{
		enum { VERSION = 0x102 };

		float fScrollBarSize;
		int nScrollBarTemplate;
		BOOL bMultiSelect;
		float fBorder;
		float fMargin;
		float fLineSpace;

		void Load_01( CStream &stream )
		{
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream >> bMultiSelect;
			stream >> fBorder;
			stream >> fMargin;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIListBoxProperty_01::VERSION: Load_01(stream); break;
			case VERSION:
				{
					stream >> fScrollBarSize;
					stream >> nScrollBarTemplate;
					stream >> bMultiSelect;
					stream >> fBorder;
					stream >> fMargin;
					stream >> fLineSpace;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIListBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << fScrollBarSize;
			stream << nScrollBarTemplate;
			stream << bMultiSelect;
			stream << fBorder;
			stream << fMargin;
			stream << fLineSpace;
		}
	};

	struct SUIProgressBarProperty
	{
		enum { VERSION = 0x101 };

		float fHoriBorder;
		float fVertBorder;

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case VERSION:
				{
					stream >> fHoriBorder;
					stream >> fVertBorder;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIProgressBarProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << fHoriBorder;
			stream << fVertBorder;
		}
	};

	struct SUICustomProperty
	{
		enum { VERSION = 0x101 };

		DWORD dwColor;	// Color to blend

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case VERSION:
				{
					stream >> dwColor;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUICustomProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << dwColor;
		}
	};

	struct SUITextBoxProperty
	{
		enum { VERSION = 0x107 };

		bool bVerticalScrollBar;
		bool bLeftScrollBar;
		float fScrollBarSize;
		int nScrollBarTemplate;
		float fLineSpace;
		UIAllignHoriType AllignHori;
		UIAllignVertType AllignVert;
		UITextBoxVariableType VariableType;
		bool bRollOver;

		void Load_01( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
		}

		void Load_02( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream.SeekCur<bool>();
			stream >> fLineSpace;
			stream >> AllignHori;
			stream >> AllignVert;
		}

		void Load_03( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream >> fLineSpace;
			stream >> AllignHori;
			stream >> AllignVert;
			stream >> VariableType;
		}

		void Load_04( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream >> fLineSpace;
			stream >> AllignHori;
			stream >> AllignVert;
			stream >> VariableType;
			stream >> bRollOver;
		}

		void Load_05( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream.SeekCur<float>();
			stream >> fLineSpace;
			stream >> AllignHori;
			stream >> AllignVert;
			stream >> VariableType;
			stream >> bRollOver;
		}

		void Load_06( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream >> fLineSpace;
			stream >> AllignHori;
			stream >> AllignVert;
			stream >> VariableType;
			stream >> bRollOver;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUITextBoxProperty_01::VERSION: Load_01(stream); break;
			case SUITextBoxProperty_02::VERSION: Load_02(stream); break;
			case SUITextBoxProperty_03::VERSION: Load_03(stream); break;
			case SUITextBoxProperty_04::VERSION: Load_04(stream); break;
			case SUITextBoxProperty_05::VERSION: Load_05(stream); break;
			case SUITextBoxProperty_06::VERSION: Load_06(stream); break;
			case VERSION:
				{
					stream >> bVerticalScrollBar;
					stream >> bLeftScrollBar;
					stream >> fScrollBarSize;
					stream >> nScrollBarTemplate;
					stream >> fLineSpace;
					stream >> AllignHori;
					stream >> AllignVert;
					stream >> VariableType;
					stream >> bRollOver;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUITextBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << bVerticalScrollBar;
			stream << bLeftScrollBar;
			stream << fScrollBarSize;
			stream << nScrollBarTemplate;
			stream << fLineSpace;
			stream << AllignHori;
			stream << AllignVert;
			stream << VariableType;
			stream << bRollOver;
		}
	};

	struct SUIHtmlTextBoxProperty
	{
		enum { VERSION = 0x105 };

		bool bVerticalScrollBar;
		float fScrollBarSize;
		int nScrollBarTemplate;
		bool bLeftScrollBar;
		float fLineSpace;
		bool bRollOver;

		void Load_01( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream.SeekCur<bool>();
			stream.SeekCur<float>();
			stream.SeekCur<UIAllignHoriType>();
			stream.SeekCur<UIAllignVertType>();
		}

		void Load_02( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream.SeekCur<float>();
			stream.SeekCur<bool>();
			stream.SeekCur<float>();
			stream.SeekCur<UIAllignHoriType>();
			stream.SeekCur<UIAllignVertType>();
		}

		void Load_03( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream.SeekCur<float>();
			stream.SeekCur<UIAllignHoriType>();
			stream.SeekCur<UIAllignVertType>();
		}

		void Load_04( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIHtmlTextBoxProperty_01::VERSION: Load_01(stream); break;
			case SUIHtmlTextBoxProperty_02::VERSION: Load_02(stream); break;
			case SUIHtmlTextBoxProperty_03::VERSION: Load_03(stream); break;
			case SUIHtmlTextBoxProperty_04::VERSION: Load_04(stream); break;
			case VERSION:
				{
					stream >> bVerticalScrollBar;
					stream >> fScrollBarSize;
					stream >> nScrollBarTemplate;
					stream >> bLeftScrollBar;
					stream >> fLineSpace;
					stream >> bRollOver;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIHtmlTextBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << bVerticalScrollBar;
			stream << fScrollBarSize;
			stream << nScrollBarTemplate;
			stream << bLeftScrollBar;
			stream << fLineSpace;
			stream << bRollOver;
		}
	};

	struct SUITextureControlProperty
	{
		enum { VERSION = 0x102 };

		int nStringIndex;

		DWORD dwFontFormat;
		BOOL bShadowFont;
		DWORD dwFontColor;
		char szTextureFileName[32];
		// std::string cannot be used because a union is used.

		void Load_01( CStream &stream )
		{
			stream >> nStringIndex;
			stream >> dwFontFormat;
			stream >> bShadowFont;
			stream >> dwFontColor;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUITextureControlProperty_01::VERSION: Load_01(stream); break;
			case VERSION:
				{
					stream >> nStringIndex;
					stream >> dwFontFormat;
					stream >> bShadowFont;
					stream >> dwFontColor;
					stream.ReadBuffer( szTextureFileName, _countof(szTextureFileName) );
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUITextureControlProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << nStringIndex;
			stream << dwFontFormat;
			stream << bShadowFont;
			stream << dwFontColor;
			stream.WriteBuffer( szTextureFileName, _countof(szTextureFileName) );
		}
	};

	struct SUITreeControlProperty
	{
		enum { VERSION = 0x107 };

		bool bVerticalScrollBar;
		float fScrollBarSize;
		int nScrollBarTemplate;
		float fLineSpace;
		float fIndentSize;

		void Load_01( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream >> fLineSpace;
		}

		void Load_02( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream >> fLineSpace;
			stream.SeekCur<float>();
			stream.SeekCur<float>();
			stream >> fIndentSize;
		}

		void Load_03( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream >> fLineSpace;
			stream.SeekCur<float>();
			stream.SeekCur<float>();
			stream >> fIndentSize;
			stream.SeekCur<float>();
		}

		void Load_04( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream.SeekCur<float>();
			stream >> fLineSpace;
			stream.SeekCur<float>();
			stream.SeekCur<float>();
			stream >> fIndentSize;
			stream.SeekCur<float>();
		}

		void Load_05( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream.SeekCur<bool>();
			stream.SeekCur<float>();
			stream >> fLineSpace;
			stream.SeekCur<float>();
			stream.SeekCur<float>();
			stream >> fIndentSize;
			stream.SeekCur<float>();
			stream.SeekCur<float>();
		}

		void Load_06( CStream &stream )
		{
			stream >> bVerticalScrollBar;
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream >> fLineSpace;
			stream.SeekCur<float>();
			stream.SeekCur<float>();
			stream >> fIndentSize;
			stream.SeekCur<float>();
			stream.SeekCur<float>();
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUITreeControlProperty_01::VERSION: Load_01(stream); break;
			case SUITreeControlProperty_02::VERSION: Load_02(stream); break;
			case SUITreeControlProperty_03::VERSION: Load_03(stream); break;
			case SUITreeControlProperty_04::VERSION: Load_04(stream); break;
			case SUITreeControlProperty_05::VERSION: Load_05(stream); break;
			case SUITreeControlProperty_06::VERSION: Load_06(stream); break;
			case VERSION:
				{
					stream >> bVerticalScrollBar;
					stream >> fScrollBarSize;
					stream >> nScrollBarTemplate;
					stream >> fLineSpace;
					stream >> fIndentSize;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUITreeControlProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << bVerticalScrollBar;
			stream << fScrollBarSize;
			stream << nScrollBarTemplate;
			stream << fLineSpace;
			stream << fIndentSize;
		}
	};

	struct SUIScrollBarProperty
	{
		enum { VERSION = 0x103 };

		float fThumbVariableLenth;

		void Load_01( CStream &stream )
		{
			stream.SeekCur<float>();
			stream.SeekCur<bool>();
		}

		void Load_02( CStream &stream )
		{
			stream.SeekCur<bool>();
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIScrollBarProperty_01::VERSION: Load_01(stream); break;
			case SUIScrollBarProperty_02::VERSION: Load_02(stream); break;
			case VERSION:
				{
					stream >> fThumbVariableLenth;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIScrollBarProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << fThumbVariableLenth;
		}
	};

	struct SUIQuestTreeControlProperty
	{
		enum { VERSION = 0x101 };

		bool bVerticalScrollBar;
		float fScrollBarSize;
		int nScrollBarTemplate;
		float fLineSpace;
		float fIndentSize;

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case VERSION:
				{
					stream >> bVerticalScrollBar;
					stream >> fScrollBarSize;
					stream >> nScrollBarTemplate;
					stream >> fLineSpace;
					stream >> fIndentSize;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIQuestTreeControlProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << bVerticalScrollBar;
			stream << fScrollBarSize;
			stream << nScrollBarTemplate;
			stream << fLineSpace;
			stream << fIndentSize;
		}
	};

	struct SUIAnimationProperty
	{
#ifdef _ANIMATION_FRAME_CONTROL
		enum { VERSION = 0x102 };

		bool bLoop;
		UIAnimationPlayTimeType PlayTimeType;
		int timePerAction;

		void Load_01( CStream &stream )
		{
			stream >> bLoop;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIAnimationInfoProperty_01::VERSION: Load_01(stream); break;
			case VERSION:
				{
					stream >> bLoop;
					stream >> PlayTimeType;
					stream >> timePerAction;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIAnimationProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << bLoop;
			stream << PlayTimeType;
			stream << timePerAction;
		}
#else
		enum { VERSION = 0x101 };

		bool bLoop;

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case VERSION:
				{
					stream >> bLoop;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIAnimationProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << bLoop;
		}
#endif // _ANIMATION_FRAME_CONTROL
	};

	struct SUILineEditBoxProperty
	{
		enum { VERSION = 0x102 };

		DWORD dwTextColor;
		DWORD dwSelTextColor;
		DWORD dwSelBackColor;
		DWORD dwCaretColor;
		float fLineSpace;

		void Load_01( CStream &stream )
		{
			stream >> dwTextColor;
			stream >> dwSelTextColor;
			stream >> dwSelBackColor;
			stream >> dwCaretColor;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUILineEditBoxProperty_01::VERSION: Load_01(stream); break;
			case VERSION:
				{
					stream >> dwTextColor;
					stream >> dwSelTextColor;
					stream >> dwSelBackColor;
					stream >> dwCaretColor;
					stream >> fLineSpace;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUILineEditBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << dwTextColor;
			stream << dwSelTextColor;
			stream << dwSelBackColor;
			stream << dwCaretColor;
			stream << fLineSpace;
		}
	};

	struct SUILineIMEEditBoxProperty
	{
		enum { VERSION = 0x102 };

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
		float fLineSpace;

		void Load_01( CStream &stream )
		{
			stream >> dwTextColor;
			stream >> dwSelTextColor;
			stream >> dwSelBackColor;
			stream >> dwCaretColor;
			stream >> dwCompTextColor;
			stream >> dwCompBackColor;
			stream >> dwCompCaretColor;
			stream >> dwCandiTextColor;
			stream >> dwCandiBackColor;
			stream >> dwCandiSelTextColor;
			stream >> dwCandiSelBackColor;
			stream >> dwReadingTextColor;
			stream >> dwReadingBackColor;
			stream >> dwReadingSelTextColor;
			stream >> dwReadingSelBackColor;
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUILineIMEEditBoxProperty_01::VERSION: Load_01(stream); break;
			case VERSION:
				{
					stream >> dwTextColor;
					stream >> dwSelTextColor;
					stream >> dwSelBackColor;
					stream >> dwCaretColor;
					stream >> dwCompTextColor;
					stream >> dwCompBackColor;
					stream >> dwCompCaretColor;
					stream >> dwCandiTextColor;
					stream >> dwCandiBackColor;
					stream >> dwCandiSelTextColor;
					stream >> dwCandiSelBackColor;
					stream >> dwReadingTextColor;
					stream >> dwReadingBackColor;
					stream >> dwReadingSelTextColor;
					stream >> dwReadingSelBackColor;
					stream >> fLineSpace;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUILineIMEEditBoxProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;
			stream << dwTextColor;
			stream << dwSelTextColor;
			stream << dwSelBackColor;
			stream << dwCaretColor;
			stream << dwCompTextColor;
			stream << dwCompBackColor;
			stream << dwCompCaretColor;
			stream << dwCandiTextColor;
			stream << dwCandiBackColor;
			stream << dwCandiSelTextColor;
			stream << dwCandiSelBackColor;
			stream << dwReadingTextColor;
			stream << dwReadingBackColor;
			stream << dwReadingSelTextColor;
			stream << dwReadingSelBackColor;
			stream << fLineSpace;
		}
	};

	struct SUIListBoxExProperty
	{
		enum { VERSION = 0x102 };

		float fScrollBarSize;
		int nScrollBarTemplate;
		BOOL bMultiSelect;
		float fBorder;
		float fMargin;
		float fLineSpace;
		char szListBoxItemUIFileName[32];
		int nSelectFrameLeft;
		int nSelectFrameTop;
		int nSelectFrameRight;
		int nSelectFrameBottom;

		void Load_01( CStream &stream )
		{
			stream >> fScrollBarSize;
			stream >> nScrollBarTemplate;
			stream >> bMultiSelect;
			stream >> fBorder;
			stream >> fMargin;
			stream >> fLineSpace;
			stream.ReadBuffer( szListBoxItemUIFileName, _countof(szListBoxItemUIFileName) );
		}

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case SUIListBoxExProperty_01::VERSION: Load_01( stream ); break;
			case VERSION:
				{
					stream >> fScrollBarSize;
					stream >> nScrollBarTemplate;
					stream >> bMultiSelect;
					stream >> fBorder;
					stream >> fMargin;
					stream >> fLineSpace;
					stream.ReadBuffer( szListBoxItemUIFileName, _countof(szListBoxItemUIFileName) );
					stream >> nSelectFrameLeft;
					stream >> nSelectFrameTop;
					stream >> nSelectFrameRight;
					stream >> nSelectFrameBottom;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIListBoxExProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << fScrollBarSize;
			stream << nScrollBarTemplate;
			stream << bMultiSelect;
			stream << fBorder;
			stream << fMargin;
			stream << fLineSpace;
			stream.WriteBuffer( szListBoxItemUIFileName, _countof(szListBoxItemUIFileName) );
			stream << nSelectFrameLeft;
			stream << nSelectFrameTop;
			stream << nSelectFrameRight;
			stream << nSelectFrameBottom;
		}
	};

	struct SUIMovieControlProperty
	{
		enum { VERSION = 0x101 };

		float fAlphaBorderHori;
		float fAlphaBorderVert;

		void Load( CStream &stream )
		{
			DWORD dwVersion;
			stream >> dwVersion;

			switch( dwVersion )
			{
			case VERSION:
				{
					stream >> fAlphaBorderHori;
					stream >> fAlphaBorderVert;
				}
				break;
			default:
				CDebugSet::ToLogFile( "SUIMovieControlProperty::Load, Invalid Control Version(%d)!", dwVersion );
				break;
			}
		}

		void Save( CStream &stream )
		{
			stream << VERSION;

			stream << fAlphaBorderHori;
			stream << fAlphaBorderVert;
		}
	};

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
		SUIMovieControlProperty MovieControlPropery;
	};

	union
	{
		SUIHtmlTextBoxProperty HtmlTextBoxProperty;	// Note : CEtHtmlTextBox inherits from CEtUITextBox.
	};

	SUIControlProperty()
	{
		ZeroMemory( this, sizeof(SUIControlProperty) );
	}

public:
	bool Load_01( CStream &stream );
	bool Load_02( CStream &stream );
	bool Load_03( CStream &stream );
	bool Load_04( CStream &stream );
	bool Load_05( CStream &stream );
	bool Load_06( CStream &stream );
	bool Load_07( CStream &stream );
	bool Load_08( CStream &stream );
	bool Load_09( CStream &stream );
	bool Load_10( CStream &stream );
	bool Load_11( CStream &stream );
	bool Load_12( CStream &stream );

	bool Load( CStream &stream );
	bool Save( CStream &stream );

public:
	void Assign( SUIControlProperty *pProperty );
};