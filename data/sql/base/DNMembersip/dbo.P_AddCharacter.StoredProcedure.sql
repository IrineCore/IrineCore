USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCharacter]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddCharacter]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddCharacter] AS' 
END
GO
/**
version : 32
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-06
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101114 = ??? ?? ??? ???????. ??? ???? ? ?? ??? ? ????.
101115 = ??? ???? ????.
101116 = ?? ???? ??? ?????.
101119 = ?? ??? ? ?? ???? ?? ???????.
101142 = ? ??? ? ?? ???? ?? ? ????.
101192 = ??? ??? ??? ??? ???? ????.
101193 = 7? ?? ?? ?????? ??? ???? ??? ? ????.
history :
25 = (48784) CBT ??? ??? ???? ??? OBT ??? ??? ? ?? ?? ?? by ??? at 2012-01-06
26 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
27 = ?? ?? ?? ??? ?? ?? ???? ??? ? ??? ?? by ??? at 2012-05-09
28 = input param. @intDefaultMaxCharacterCountPerAccount ?? by ??? at 2012-05-10
29 = output param. @dt2CreateDate ?? by ??? at 2012-10-17
30 ~ 32 = ??? ??? ??? ?? ?? ?? by ??? at 2013-05-10
**/

ALTER PROCEDURE [dbo].[P_AddCharacter]
	  @intAccountID int --// ??? ?? ID
	, @nvcCharacterName nvarchar(30) --// ??? ??
	, @intWorldID int --// ???? ??? ?? ID
	, @intDeleteWaitingTime int --// ??? ?? ?? ??. ?? : minute
	, @inbCharacterID bigint = NULL OUTPUT --// ??? ID
	, @bitMaxCharacterCountByAccountFlag bit = 0 --// ?? ??? ? ?? ???? ?? ????? ??. 0=?? ???, 1=?? ???
	, @intDefaultMaxCharacterCountPerAccount int = 0 --// ?? ??? ? ?? ???? ?? ???? ??, ?? ? ?? Max ??? ?
	, @dt2CreateDate datetime2(0) = NULL OUTPUT --// ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @nvcAccountName nvarchar(50)
	, @intActiveCharacterCount int = 0
	, @intDeleteWaitingCharacterCount int = 0
	, @intCharacterMaxCount int = 0
	, @dt2Now datetime2(0) = GETDATE()
	, @intItemCount int;

DECLARE @tblCharacterStarterItems table (
	Seq int NOT NULL IDENTITY(1,1),
	WorldID int NOT NULL, 
	GiftSenderName nvarchar(50) NOT NULL,
	GiftMemo nvarchar(100) NOT NULL,
	GiftLifeSpan int NOT NULL,
	ProductID int NOT NULL,
	ItemIDs varchar(max) NOT NULL
);

DECLARE @Temp_SelectedItemID TABLE (ItemID int, ItemOption tinyint);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dt2CreateDate = GETDATE();

	/**_# ??? ??? ?? ?? ???? 101142? ?????.*/
	IF EXISTS (
		SELECT * FROM dbo.WorldConfig WITH (NOLOCK)
		WHERE WorldID = @intWorldID AND WorldConfigCode = 1 AND WorldConfigValue = 1
	) OR @intWorldID = 0
	BEGIN
		SET @intReturnValue = 101142;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ???? 101192? ?????.*/
	IF EXISTS (
		SELECT *
		FROM dbo.ForbiddenCharacterNames WITH (NOLOCK)
		WHERE CharacterName = @nvcCharacterName
	)
	BEGIN
		SET @intReturnValue = 101192;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ?? ??? ???? 101193? ?????.*/
	IF EXISTS (
		SELECT *
		FROM dbo.CharacterNameChangeLogs WITH (UPDLOCK)
		WHERE CharacterNameBefore = @nvcCharacterName
			AND ChangeDate > DATEADD(DAY, -7, GETDATE())
	)
	BEGIN
		SET @intReturnValue = 101193;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ?? ? ???? ??? ??? ????? ???.*/
	IF EXISTS (
		SELECT *
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.DeletedCharacters DC WITH (NOLOCK) ON CH.CharacterID = DC.CharacterID
		WHERE CH.CharacterName = @nvcCharacterName AND DC.DeleteDate < DATEADD(MINUTE, -@intDeleteWaitingTime, @dt2CreateDate)
	)
		UPDATE dbo.Characters
		SET CharacterName = NULL
		WHERE CharacterName = @nvcCharacterName;

	/**_# ?? ????? ??? ? ?? ??? ???? ?????.*/
	SELECT @nvcAccountName = AccountName
	FROM dbo.ReservedCharacterNames WITH (NOLOCK, INDEX(PK_ReservedCharacterNames), FORCESEEK)
	WHERE CharacterName = @nvcCharacterName
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT > 0 AND NOT EXISTS (
		SELECT *
		FROM dbo.Accounts WITH (NOLOCK, INDEX(PK_Accounts), FORCESEEK)
		WHERE AccountID = @intAccountID AND AccountName = @nvcAccountName
	)
	BEGIN
		SET @intReturnValue = 101116;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;

	/**_# [P_DecreaseCharacterCreateLimit]? ???? ??? ?? ??? ??? ?? 1?? ?????.*/
	EXEC @intReturnValue = dbo.P_DecreaseCharacterCreateLimit
		  @intAccountID = @intAccountID;

	/**_# [Characters] ???? INSERT???.*/
	/**_## ??? ???? ??? 101115? ?????.*/
	/**_## ?? ???? ??? ???? 101116? ?????.*/
	INSERT dbo.Characters (AccountID, CharacterName, WorldID, CreateDate, DeleteFlag)
	VALUES (@intAccountID, @nvcCharacterName, @intWorldID, @dt2CreateDate, 0);
	SET @inbCharacterID = SCOPE_IDENTITY();

	/**_# ?? ?? ??? ?? ???? 101119? ???? ?????.*/
	IF @bitMaxCharacterCountByAccountFlag = 0
	BEGIN
		SELECT @intActiveCharacterCount = CharacterCount
		FROM dbo.VIX_CharacterCount WITH (NOEXPAND, NOLOCK)
		WHERE AccountID = @intAccountID AND WorldID = @intWorldID;

		SELECT @intDeleteWaitingCharacterCount = COUNT(*)
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.DeletedCharacters DCH WITH (NOLOCK) ON CH.CharacterID = DCH.CharacterID
		WHERE CH.AccountID = @intAccountID
			AND CH.CharacterName IS NOT NULL
			AND DCH.DeleteDate > DATEADD(MINUTE, -@intDeleteWaitingTime, @dt2CreateDate)
			AND CH.WorldID = @intWorldID
			AND CH.DeleteByAdminFlag = CAST(0 AS bit)

		EXEC @intReturnValue = dbo.P_GetCharacterSlotCount
			  @inbCharacterID = @inbCharacterID
			, @inyCharacterSlotCount = @intCharacterMaxCount OUTPUT;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	ELSE
	BEGIN
		SELECT @intActiveCharacterCount = SUM(CharacterCount)
		FROM dbo.VIX_CharacterCount WITH (NOEXPAND, NOLOCK)
		WHERE AccountID = @intAccountID;

		SELECT @intDeleteWaitingCharacterCount = COUNT(*)
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.DeletedCharacters DCH WITH (NOLOCK) ON CH.CharacterID = DCH.CharacterID
		WHERE CH.AccountID = @intAccountID
			AND CH.CharacterName IS NOT NULL
			AND DCH.DeleteDate > DATEADD(MINUTE, -@intDeleteWaitingTime, @dt2CreateDate)
			AND CH.DeleteByAdminFlag = CAST(0 AS bit)

		EXEC @intReturnValue = dbo.P_GetCharacterSlotCount
			  @inbCharacterID = NULL
			, @inyCharacterSlotCount = @intCharacterMaxCount OUTPUT
			, @intAccountID = @intAccountID
			, @intWorldID = NULL;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END
	IF (@intActiveCharacterCount + @intDeleteWaitingCharacterCount) > (@intCharacterMaxCount + @intDefaultMaxCharacterCountPerAccount)
	BEGIN
		SET @intReturnValue = 101119;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	/**_# CharacterStarterItem? ?? ?? ????? ?????.*/
	INSERT @tblCharacterStarterItems (WorldID, GiftSenderName, GiftMemo, GiftLifeSpan, ProductID, ItemIDs)
	SELECT WorldID, GiftSenderName, GiftMemo, GiftLifeSpan, ProductID, ItemIDs
	FROM dbo.CharacterStarterItems WITH (NOLOCK)
	WHERE (ProvideStartDate IS NULL AND ProvideEndDate IS NULL) 
		OR (ProvideStartDate <= @dt2Now AND ProvideEndDate >= @dt2Now)
		OR (ProvideStartDate IS NULL AND ProvideEndDate >= @dt2Now)
		OR (ProvideEndDate IS NULL AND ProvideStartDate <= @dt2Now);

	SET @intItemCount = @@ROWCOUNT;
	
	IF @intItemCount > 0
	BEGIN
		DECLARE @i int = 1;		
		
		WHILE @i <= @intItemCount
		BEGIN
			DELETE @Temp_SelectedItemID;

			DECLARE @nvcGiftSenderName nvarchar(50)
				  , @nvcGiftMemo nvarchar(100)
				  , @intGiftLifeSpan int
				  , @intProductID int
				  , @vchItemIDs varchar(max)
				  , @inbPurchaseOrderID bigint
				  , @inbPurchaseOrderDetailID bigint
				  , @intProvideWorldID int;			

			SELECT @nvcGiftSenderName = GiftSenderName
				 , @nvcGiftMemo = GiftMemo
				 , @intGiftLifeSpan = GiftLifeSpan
				 , @intProductID = ProductID
				 , @vchItemIDs = ItemIDs
				 , @intProvideWorldID = WorldID
			FROM @tblCharacterStarterItems
			WHERE Seq = @i;

			IF @intProvideWorldID = @intWorldID
			BEGIN
				INSERT @Temp_SelectedItemID (ItemID)
				SELECT SUBSTRING(',' + @vchItemIDs + ',', n + 1, CHARINDEX(',', ',' + @vchItemIDs + ',', n + 1) - n - 1)
				  FROM dbo.Tally WITH (NOLOCK)
				 WHERE n < DATALENGTH(',' + @vchItemIDs + ',')
				   AND SUBSTRING(',' + @vchItemIDs + ',', n, 1) = ','
				   AND SUBSTRING(',' + @vchItemIDs + ',', n + 1, CHARINDEX(',', ',' + @vchItemIDs + ',', n + 1) - n - 1) > 0

				INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, OrderResult, OrderRollbackResult, GiftReceiveFlag, GiftMemo, GiftExpireDate, GiftSenderName, CouponKey, GiftReceiveDate, ContextKey, GiftRejectFlag, GiftRejectDate, NewFlag, PaidCashAmount)
				SELECT NULL, @inbCharacterID, 1, 0, 0, 0, @dt2Now, 2, 5, NULL, NULL, NULL, 0, @nvcGiftMemo, DATEADD(MINUTE, @intGiftLifeSpan, @dt2Now), @nvcGiftSenderName, NULL, NULL, NULL, 0, NULL, 0, NULL;

				SET @inbPurchaseOrderID = SCOPE_IDENTITY();

				INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
				SELECT @inbPurchaseOrderID, @intProductID, 0;

				SET @inbPurchaseOrderDetailID = SCOPE_IDENTITY();

				INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, GiveFailFlag, ItemOption)
				SELECT @inbPurchaseOrderDetailID, ItemID, 0, ItemOption
				  FROM @Temp_SelectedItemID;
			END

			SET @i = @i + 1;
		END
	END

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Accounts_Characters_AccountID%'
		SET @intReturnValue = 101115;

	ELSE IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNF_Characters_CharacterName%'
		SET @intReturnValue = 101116;

	GOTO ErrorHandler;
END CATCH;

RETURN 0;
ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;
IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
