#pragma once

#include "DnCustomDlg.h"
#include "DnBlindDlg.h"
#include "DnItemTask.h"

class CDnDarklairClearRewardDlg : public CDnCustomDlg, public CDnBlindCallBack
{
public:
	CDnDarklairClearRewardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnDarklairClearRewardDlg();

	struct SBonusBoxButton
	{
		CDnBonusBoxButton *m_pBonusBoxButton;
		CDnItem *m_pItem;
		CDnItemSlotButton *m_pItemSlotButton;
		int m_nBoxType;

		SBonusBoxButton()
			: m_pBonusBoxButton(NULL)
			, m_pItem(NULL)
			, m_pItemSlotButton(NULL)
			, m_nBoxType(0)
		{
		}

		~SBonusBoxButton()
		{
			SAFE_DELETE( m_pItem );
		}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
		void SetBoxType(int nBoxType)
		{
			m_nBoxType = nBoxType;
		}

		void SetBoxItem(TItem &ItemInfo)
		{
			if( ItemInfo.nItemID > 0 )
			{
				SAFE_DELETE( m_pItem );

				m_pItem = GetItemTask().CreateItem( ItemInfo );

				m_pItemSlotButton->SetItem(m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
			}
			else
			{
				m_pItemSlotButton->SetBlankItem();
			}
		}
#else
		void SetBoxInfo(int nBoxType, TItem &ItemInfo)
		{
			m_nBoxType = nBoxType;

			if( ItemInfo.nItemID > 0 )
			{
				SAFE_DELETE( m_pItem );

				m_pItem = GetItemTask().CreateItem( ItemInfo );

				m_pItemSlotButton->SetItem(m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
			}
			else
			{
				m_pItemSlotButton->SetBlankItem();
			}
		}
#endif

		void Clear()
		{
			SAFE_DELETE( m_pItem );
			m_pItemSlotButton->ResetSlot();
		}
	};

	struct SStageClearList
	{
		CDnGaugeFace *m_pFace;
		CEtUIStatic *m_pName;
		CEtUIStatic *m_pBase;
		SBonusBoxButton m_sBonusBoxButton[4];

		int m_nSessionID;
		int m_nBoxCount;

		SStageClearList()
			: m_pFace(NULL)
			, m_pName(NULL)
			, m_pBase(NULL)
			, m_nSessionID(0)
			, m_nBoxCount(0)
		{
		}

		void Show( bool bShow )
		{
			m_pFace->Show( bShow );
			m_pName->Show( bShow );
			m_pBase->Show( bShow );
		}

		void Clear()
		{
			m_nSessionID = 0;
			m_nBoxCount = 0;
		}
	};

	struct SoundType 
	{
		enum 
		{
			RECOMPENSE_WINDOW =0,
			BOX_MOUSEON,			//  Mouse over reward box
			BOX_CLICK,				//  Click on reward box
			BOX_OPEN,				//  When reward box is opened
			MAX
		};
	};

	enum ePhase
	{
		PHASE_NONE,
		PHASE_CLOSED_BOX,
		PHASE_OPEN_BOX
	};

	enum
	{
		REWARDIDENTITY_ITEMID = 1107312640,
	};

protected:
	EtTextureHandle m_hRoundTexture;

	CDnGaugeFace *m_pFace;
	CEtUITextureControl *m_pTextureRound[2];
	CEtUIStatic *m_pStaticAllRound;
	CEtUIStatic *m_pName;
	CEtUIStatic *m_pStaticRemainSelectBox;
	SBonusBoxButton m_sBonusBoxButton[4];
	CDnItemSlotButton *m_pIdentifySlot;
	CEtUIStatic *m_pIdentifySlotBase;
	CEtUIStatic *m_pIdentifyMsg;
	CEtUIStatic *m_pIdentifyBack[2];
	CDnItem *m_pIdentifyItem;
	int m_nBoxCount;
	bool m_bRequestIdentify;

	CEtUIStatic *m_pTime;
	CEtUIStatic *m_pTimeBase;

	std::vector<SStageClearList> m_vecStageClearList;

	float m_fElapsedTime;
	int m_nCount;
	bool m_bTimer;
	int m_nSelectRemainCount;
	int m_nCurPhase;

	int	m_nSoundIndexArray[SoundType::MAX];

	int		m_CurBoxIdxPresented;
	float	m_fPhaseTime;
	ePhase	m_Phase;

	CDnStageClearMoveDlg* m_pStageClearMoveDlg;

protected:
	void SetClearInfo();
	void CheckIdentifyItem();
#if defined ( PRE_ADD_NAMEDITEM_SYSTEM )
	void SetRewardBoxOrRewardItemInfo(bool bBoxInfo);
#else
	void SetRewardItemInfo();
#endif

public:
	// CDnCustomDLg
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void Render( float fElapsedTime );

	virtual void OnBlindClose();
	virtual void OnBlindClosed();

	void SetAllQuestionBox();
	void SetAllBonusBox();
	void SetAllOpenBonusBox();

	void SetClosedBonusBox(int idx);
	void SetOpenBonusBox(int idx);
	void SetAllPartyBonusBox();
	void SetAllPartyOpenBonusBox();

	void SelectBonusBox( int nSessionID, char cSelectIndex );
	void OpenBonusBox();
	void OpenBonusBox( char cSlotIndex, char cBoxType );
	void OpenBonusItem();
	void OpenDungeonMoveDlg();
	void SetMyPortraitTexture( EtTextureHandle hTexture );

	void ResetCount() { m_nCount = 0; }

#if defined ( PRE_ADD_NAMEDITEM_SYSTEM )
	void ShowBonusBox();
#endif
};
