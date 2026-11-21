#pragma once

#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"
#include "DnInterface.h"

class CDnItem;
class CDnWorldServerStorageConfirmDlg : public CDnCustomDlg
{
public:
	CDnWorldServerStorageConfirmDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL);
	virtual ~CDnWorldServerStorageConfirmDlg(void);

	virtual void Initialize(bool bShow);
	virtual void InitialUpdate();
	virtual void InitCustomControl(CEtUIControl *pControl);
	virtual void Show(bool bShow);
	virtual void ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg);

	virtual void EnableButtons(bool bEnable);
	virtual void Reset();
	virtual int GetSlotItemCount() const;
	int GetRealItemCount() const;

	void SetItemWithSlotButton(CDnSlotButton* pFromSlot, bool bRenderCountMinusOne);
	virtual bool SetItem(CDnItem* pItem, int slotCount);

	CDnItem* GetItem();
	void UpdateFee();

	bool IsFromCashInventory() const;
	bool IsFromCashStorage() const;
	bool IsRenderCountMinusOne() const { return m_bRenderCountMinusOne; }

protected:
	CDnItemSlotButton* m_pItemSlotButton;
	CDnSlotButton* m_pFromSlotBtnCache;

	CEtUIStatic* m_pItemName;

	CEtUIStatic* m_pGold;
	CEtUIStatic* m_pSilver;
	CEtUIStatic* m_pCopper;

	CEtUIButton* m_pOKButton;
	CEtUIButton* m_pCancelButton;

	DWORD m_dwColorGold;
	DWORD m_dwColorSilver;
	DWORD m_dwColorCopper;

	bool m_bRenderCountMinusOne;

	// Smart move
	CDnSmartMoveCursor m_SmartMove;
};

//////////////////////////////////////////////////////////////////////////

class CDnWorldServerStorageConfirmExDlg : public CDnWorldServerStorageConfirmDlg
{
public:
	CDnWorldServerStorageConfirmExDlg(UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL);
	virtual ~CDnWorldServerStorageConfirmExDlg()  {}

	virtual void Initialize(bool bShow);
	virtual void InitialUpdate();

	virtual bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg);

	virtual void EnableButtons(bool bEnable);
	virtual void Reset();

	virtual bool SetItem(CDnItem* pItem, int slotCount);
	virtual void Show(bool bShow);

	void SetItemCountForce(int count);

	void UpdateAmount();
	int GetSlotItemCount() const;

private:
	CEtUIButton* m_pItemUp;
	CEtUIButton* m_pItemDown;

	CEtUIButton* m_pButtonMin;
	CEtUIButton* m_pButtonMax;

	CEtUIEditBox* m_pEditBoxItemCount;
};