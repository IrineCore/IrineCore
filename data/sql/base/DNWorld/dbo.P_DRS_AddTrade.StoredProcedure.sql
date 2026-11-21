USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddTrade]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-22
description : ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000026 = ??? ???? ????.
1030000059 = ?? ?? ??? ??? ? ????.
1030000060 = ?? ?? ??? ?? ? ? ????.
1030000063 = ?????? ??? ? ????.
history :
3 = ?? ??? CharacterName ????? ?? by ??? at 2013-05-15
4 = ????? ?? ??? ??? ?? by ??? at 2013-06-05
5 = PushQueue ??? AccountID? ????? ?? by ??? at 2013-08-16
**/

CREATE PROCEDURE [dbo].[P_DRS_AddTrade]
	  @inbCharacterID bigint --// ??? ID
	, @insMaxRegisterCount1 smallint --// ??? ? ?? ?? ?? (?? ?? ?? ?? ?? ? ??? ?? ?)
	, @insMaxRegisterCount2 smallint --// ??? ? ?? ?? ?? (??? AM 04:00 ?? ???? ??? ? ?? ?? ??)
	, @intItemID int --// ??? ??? ID
	, @nvcItemName nvarchar(50) --// ??? ??
	, @inyItemRankCode tinyint --// 0=??, 1=??, 2=??, 3=??, 4=???, 5=???
	, @inyLevelLimit tinyint --// ???? ???? ?? ?? ??? ??
	, @inyJobCode tinyint --// 1=???, 2=??, 3=????, 4=???, 5=????, 10=???, 11=?????, 12=????, 13=???, 14=?????, 15=????, 16=??, 17=??????, 18=????, 19=??, 20=???, 21=??, 22=????, 23=??????, 24=???, 25=????, 26=??????, 27=?????, 28=?????, 29=????, 30=????, 31=????, 32=????, 33=???, 34=?????, 35=????, 36=?????, 37=???, 38=????, 39=?????, 40=??????, 41=???, 42=?????, 43=???, 44=????, 45=?????, 46=????, 47=????, 48=?????, 49=?????, 50=???, 51=???
	, @bitCashItemFlag bit --// 0=?? ???, 1=?? ???
	, @intItemCategoryID int --// ???? ?? ???? ID
	, @inbItemSerial bigint --// ??? ??? ?? ??
	, @insItemCount smallint --// ??? ??? ??	
	, @inyItemLevel tinyint --// ??? ??? ??
	, @inyPayMethodCode tinyint --// 1=????, 3=??
	, @intPrice int --// ?? ??
	, @intListPrice int --// ?? ??
	, @intRegisterCommission int --// ??? ?? ???
	, @intTradeCommission int --// ?? ?? ???
	, @inySellPeriod int --// ?? ?? (??)
	, @bitPremiumFlag bit = 0 --// 0=???? ??, 1=????
	, @intCharacterExp int OUTPUT --// ??? ?? ? ??? ???
	, @inbCoin bigint OUTPUT --// ??? ?? ? ??? ?? ??
	, @inbWarehouseCoin bigint OUTPUT --// ??? ?? ? ??? ?? ?? ??
	, @inyCharacterLevel tinyint OUTPUT --// ??? ?? ? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intTradeCount int
	, @dt2Now datetime2(0) = GETDATE()
	, @inyItemLocationCode tinyint
	, @inbMaterializedItemID bigint
	, @inbNewMaterializedItemID bigint
	, @dt2OwnershipStartDate datetime2(0)
	, @bitSoulBoundFlag bit
	, @inySealCount tinyint
	, @inyItemOption tinyint
	, @inyItemPotential tinyint
	, @intTradeIDForAutoTrade int
	, @inyItemPotentialMoveCount tinyint
	, @inyDragonJewelType tinyint
	, @i int = 1
	, @j int
	, @intTradeID int
	, @intAdditiveItemID int;

DECLARE @tblTemp table (
	MaterializedItemID bigint NOT NULL,
	ItemID int NOT NULL,
	OwnerCharacterID bigint NOT NULL,
	OwnershipStartDate datetime2(0) NULL,
	ItemCount smallint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	ItemPotentialMoveCount tinyint NULL,
	DragonJewelType tinyint NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ???? ???? ??? 1030000026? ?????.*/
	SELECT @inbMaterializedItemID = MaterializedItemID, @intAdditiveItemID = AdditiveItemID
	FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
	WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemLocationCode IN (2, 5)
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000026;
		GOTO ErrorHandler;
	END

	/**_# ?????? ?? 1030000026? ?????.*/	
	IF @intAdditiveItemID IS NOT NULL
	BEGIN
		SET @intReturnValue = 1030000063;
		GOTO ErrorHandler;
	END	

	/**_# ?? ? ??? ??? ??? ?????.*/
	SELECT @intTradeCount = TradeCount
	FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK)
	WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 0
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
		SET @intTradeCount = 0;
	IF @insMaxRegisterCount1 <= @intTradeCount
	BEGIN
		SET @intReturnValue = 1030000059;
		GOTO ErrorHandler;
	END

	IF @bitPremiumFlag = 0
	BEGIN
		SELECT @intTradeCount = COUNT(*)
		FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_CharacterID_TradeStatusCode), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
			AND PremiumFlag = CAST(0 AS bit)
			AND RegisterDate >= DATEADD(DAY, - DATEPART(WEEKDAY, DATEADD(HOUR, 4, DATEADD(DAY, DATEDIFF(DAY, 0, @dt2Now) + CASE WHEN DATEPART(HOUR, @dt2Now) BETWEEN 0 AND 3 THEN 0 ELSE 1 END, 0))), DATEADD(HOUR, 4, DATEADD(DAY, DATEDIFF(DAY, 0, @dt2Now) + CASE WHEN DATEPART(HOUR, @dt2Now) BETWEEN 0 AND 3 THEN 0 ELSE 1 END, 0)))
		OPTION (KEEPFIXED PLAN);

		IF @insMaxRegisterCount2 <= @intTradeCount
		BEGIN
			SET @intReturnValue = 1030000060;
			GOTO ErrorHandler;
		END
	END

	BEGIN TRANSACTION;

	UPDATE dbo.MaterializedItems
	SET SealCount = 
			CASE @bitCashItemFlag
				WHEN 1 THEN CASE WHEN SealCount <= 1 THEN 0 ELSE SealCount - 1 END
				ELSE SealCount
			END
		, ItemLocationCode = 12
		, ItemLocationIndex = NULL
	FROM dbo.MaterializedItems WITH (INDEX(PK_MaterializedItems), FORCESEEK)
	WHERE OwnerCharacterID = @inbCharacterID
		AND ItemLocationCode IN (2, 5)
		AND MaterializedItemID = 
			CASE
				WHEN @inbNewMaterializedItemID IS NULL THEN @inbMaterializedItemID
				ELSE @inbNewMaterializedItemID
			END
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000026;
		GOTO ErrorHandler;
	END

	DECLARE @inbMaterializedItemsItemSerial bigint, @dt2MaterializedItemsTradeExpireDate datetime2, @sdtMaterializedItemsItemDurability smallint, 
            @intMaterializedItemsRandomSeed int, @inyMaterializedItemsItemPotential tinyint, @inyMaterializedItemsItemOption tinyint, @inyMaterializedItemsSealCount tinyint;

	SELECT @inbMaterializedItemsItemSerial = ItemSerial, @dt2MaterializedItemsTradeExpireDate = DATEADD(HOUR, @inySellPeriod, @dt2Now), @sdtMaterializedItemsItemDurability = ItemDurability, 
		   @intMaterializedItemsRandomSeed = RandomSeed, @inyMaterializedItemsItemPotential = ItemPotential, @inyMaterializedItemsItemOption = ItemOption, @inyMaterializedItemsSealCount = SealCount, @inyItemPotentialMoveCount = ItemPotentialMoveCount, @insItemCount = ItemRemainCount, @inyDragonJewelType = DragonJewelType
	FROM dbo.MaterializedItems WITH (UPDLOCK, INDEX (PK_MaterializedItems), FORCESEEK)
	WHERE MaterializedItemID = 
		CASE
			WHEN @inbNewMaterializedItemID IS NULL THEN @inbMaterializedItemID
			ELSE @inbNewMaterializedItemID
		END
	OPTION (KEEPFIXED PLAN);

	/**_# [Trade] ???? INSERT???.*/
	INSERT dbo.Trade (CharacterID, ItemID, ItemSerial, ItemLevel, ItemCount, SkillUsingTypeCode, Price, ListPrice, RegisterCommission, TradeCommission, DisplayFlag1, DisplayFlag2, TradeStatusCode, TradeExpireDate, RegisterDate, BuyerCharacterID, TradeDate, ItemDurability, RandomSeed, ItemPotential, ItemOption, SealCount, PremiumFlag, PayMethodCode, ItemPotentialMoveCount, DragonJewelType, CharacterName)
	SELECT CharacterID, @intItemID, @inbMaterializedItemsItemSerial, @inyItemLevel, @insItemCount, 0, @intPrice, @intListPrice, @intRegisterCommission, @intTradeCommission, 0, 0, 0, @dt2MaterializedItemsTradeExpireDate, @dt2Now, NULL, NULL, @sdtMaterializedItemsItemDurability, @intMaterializedItemsRandomSeed, @inyMaterializedItemsItemPotential, @inyMaterializedItemsItemOption, @inyMaterializedItemsSealCount, @bitPremiumFlag, @inyPayMethodCode, @inyItemPotentialMoveCount, @inyDragonJewelType, CharacterName
	FROM dbo.Characters WITH(NOLOCK)
	WHERE CharacterID = @inbCharacterID;
	SET @intTradeID = SCOPE_IDENTITY();

	COMMIT TRANSACTION;

	/**_# ??? ?? ???? ?????.*/
	EXEC @intReturnValue = dbo.P_ModCoinForSubProc
		  @inbCharacterID = @inbCharacterID
		, @inyCoinChangeCode = 21
		, @inbCoinChangeFKey = @intTradeID
		, @inbCoin = @intRegisterCommission
		, @intChannelID = NULL
		, @intMapID = NULL
		, @dt2LogDate = @dt2Now;

	/**_# [Items] ???? MERGE???.*/
	MERGE dbo.TradeItems AS T
	USING (SELECT @intItemID AS ItemID, @nvcItemName AS ItemName, @inyLevelLimit AS LevelLimit, @inyJobCode AS JobCode, @inyItemRankCode AS ItemRankCode, @bitCashItemFlag AS CashItemFlag, @intItemCategoryID AS ItemCategoryID) AS S
	ON T.ItemID = S.ItemID
	WHEN MATCHED THEN
		UPDATE SET ItemName = S.ItemName, LevelLimit = S.LevelLimit, JobCode = S.JobCode, ItemRankCode = S.ItemRankCode, CashItemFlag = S.CashItemFlag, ItemCategoryID = S.ItemCategoryID
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (ItemID, ItemName, LevelLimit, JobCode, ItemRankCode, CashItemFlag, ItemCategoryID)
		VALUES (S.ItemID, S.ItemName, S.LevelLimit, S.JobCode, S.ItemRankCode, S.CashItemFlag, S.ItemCategoryID)
	OPTION (KEEPFIXED PLAN, LOOP JOIN);

	/**_# [PushQueue] ???? INSERT???.*/
	INSERT dbo.PushQueue (WishListID, CharacterID, RegistedDate, PushTypeCode, ItemID, AccountID)
	SELECT WL.WishListID, WL.CharacterID, @dt2Now, 0, @intItemID, CH.AccountID
	FROM dbo.WishList WL WITH (NOLOCK, INDEX(IX_NNIF_WishList_ItemID), FORCESEEK)
	     INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) 
	     ON WL.CharacterID = CH.CharacterID
	WHERE WL.ItemID = @intItemID
		AND WL.NoticeFlag = CAST(1 AS bit)
		AND WL.NoticeItemLevel <= @inyItemLevel
		AND WL.NoticePrice >= @intListPrice
	OPTION (KEEPFIXED PLAN);

	SELECT @intCharacterExp = CharacterExp, @inbCoin = Coin, @inbWarehouseCoin = WarehouseCoin, @inyCharacterLevel = CharacterLevel
	FROM dbo.CharacterStatus WITH (NOLOCK) 
	WHERE CharacterID = @inbCharacterID;

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
