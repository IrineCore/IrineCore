USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTrade2]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 32
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-08
description : ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
103169 = ??? ?? ?? ??? ???????.
103186 = ???? ???? ?? ??? ? ??? ???????.
103240 = ????? ??? ???? ????.
103292 = ???? ???? ?? ??????.
history :
6 = cash ??? ??? ?? ?? ??? ???? ?? ??? ?? ?? by ??? at 2011-10-25
7 = ?? ?? ??? ?????. by ??? at 2011-11-18
8 = (37976) ?? ?? ?? ??. (1)??? ???? ??? ??, (2)???? 5?? ??? ??? ????? ?? by ??? at 2011-12-06
10 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
11 = ??? ?? ?? by ??? at 2012-03-16
16 = 11?? ??     by ??? at 2012-04-09
17 = ?? ?? ?? ??     by ??? at 2012-04-10
19 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-19 
20 = ??? ??? ??? ?? ??? ?? by ??? at 2012-07-20 
26 = ????? ??? ??? ???? ??? ??? ?? ????? ?? by ??? at 2013-03-20
28~31 = ?? ??? CharacterName ????? ?? by ??? at 2013-05-15
32 = PushQueue ??? AccountID ?? by ??? at 2013-08-16
**/

CREATE PROCEDURE [dbo].[P_AddTrade2]
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
	, @inbNewItemSerial bigint = NULL --// ???? ?? ??? ???? ?? ? ItemSerial? ???? ??? ??? NULL? ?????.
	, @inyItemLevel tinyint --// ??? ??? ??
	, @inyPayMethodCode tinyint --// 1=????, 3=??
	, @intPrice int --// ?? ??
	, @intListPrice int --// ?? ??
	, @intRegisterCommission int --// ??? ?? ???
	, @intTradeCommission int --// ?? ?? ???
	, @inySellPeriod int --// ?? ?? (??)
	, @bitPremiumFlag bit = 0 --// 0=???? ??, 1=????
	, @intMapID int --// ? ID
	, @intTradeID int OUTPUT --// ??? ?? ID;
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
	, @j int;

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

DECLARE @tblDeletedDragonJewelIDs table (
	MaterializedItemID bigint NOT NULL
);

DECLARE @tblDeletedDragonJewels table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	MaterializedItemID bigint NOT NULL,
	ItemID int NOT NULL,
	OwnerCharacterID bigint NULL,
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
	/**_# ???? ???? ??? 103151? ?????.*/
	SELECT @inbMaterializedItemID = MaterializedItemID
	FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
	WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemLocationCode IN (2, 5)
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
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
		SET @intReturnValue = 103169;
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
			SET @intReturnValue = 103186;
			GOTO ErrorHandler;
		END
	END

	/**_# ?? ??? ????? ???? split ???.*/
	IF @inbNewItemSerial <> 0
	BEGIN
		EXEC @intReturnValue = dbo.P_SplitMaterializedItem
			  @dt2SplitDate = @dt2Now
			, @inbMaterializedItemID = @inbMaterializedItemID
			, @inbNewItemSerial = @inbNewItemSerial
			, @insItemCount = @insItemCount
			, @bitInitOwnerCharacter = 0
			, @intChannelID = NULL
			, @intMapID = @intMapID
			, @inyCallProc = 1
			, @inbNewMaterializedItemID = @inbNewMaterializedItemID OUTPUT;
		IF @intReturnValue <> 0
			GOTO ErrorHandler;
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
		SET @intReturnValue = 103151;
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
	FROM dbo.Characters WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK)
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
		, @intMapID = @intMapID
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

	/**_# ?? ?? ??? ??? ??? ? ???? 5?? ??? TOP 1?? ?? ?? ?? ?????.*/
	BEGIN TRANSACTION;

	SELECT TOP (1) @intTradeIDForAutoTrade = TR.TradeID
	FROM dbo.AutoTrade AT WITH (NOLOCK)
		INNER LOOP JOIN dbo.Trade TR WITH (UPDLOCK, READPAST, INDEX(IX_NNIF_Trade_ItemID), FORCESEEK) ON AT.ItemID = TR.ItemID AND AT.MaxUnitPrice >= TR.ListPrice
	WHERE AT.EnableFlag = CAST(1 AS bit)
		AND AT.RemainBudget > 0
		AND TR.TradeStatusCode = 0
		AND TR.PayMethodCode = 1
		AND TR.RegisterDate <= DATEADD(MINUTE, -5, GETDATE())
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
		COMMIT TRANSACTION;

	ELSE
	BEGIN
		UPDATE dbo.Trade
		SET @inbItemSerial = ItemSerial
			, TradeStatusCode = 1
			, BuyerCharacterID = 0
			, TradeDate = @dt2Now
		FROM dbo.Trade WITH (INDEX(PK_Trade), FORCESEEK)
		WHERE TradeID = @intTradeIDForAutoTrade
		OPTION (KEEPFIXED PLAN);

		UPDATE dbo.AutoTrade
		SET RemainBudget -= @intPrice
		WHERE ItemID = @intItemID;

		INSERT dbo.AutoTradeResults (TradeID, ItemID, CharacterID, UnitPrice, UsedBudget, TradeDate)
		VALUES (@intTradeID, @intItemID, @inbCharacterID, @intListPrice, @intPrice, @dt2Now);

		COMMIT TRANSACTION;

		DELETE dbo.MaterializedItems
		OUTPUT deleted.MaterializedItemID, deleted.ItemID, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotential, deleted.DragonJewelType
		INTO @tblTemp (MaterializedItemID, ItemID, OwnerCharacterID, OwnershipStartDate, ItemCount, SoulBoundFlag, SealCount, ItemOption, ItemPotential, DragonJewelType)
		WHERE ItemSerial = @inbItemSerial;	
		
		IF @@ROWCOUNT > 0		
			DELETE dbo.DragonJewels 
			OUTPUT deleted.MaterializedItemID
			INTO @tblDeletedDragonJewelIDs (MaterializedItemID)
			WHERE ParentItemSerial = @inbItemSerial;

		IF @@ROWCOUNT > 0
		BEGIN
			DELETE dbo.MaterializedItems
			OUTPUT deleted.MaterializedItemID, deleted.ItemID, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotential, deleted.DragonJewelType
			INTO @tblDeletedDragonJewels (MaterializedItemID, ItemID, OwnerCharacterID, OwnershipStartDate, ItemCount, SoulBoundFlag, SealCount, ItemOption, ItemPotential, DragonJewelType)
			FROM @tblDeletedDragonJewelIDs A INNER JOIN dbo.MaterializedItems B WITH (NOLOCK) ON A.MaterializedItemID = B.MaterializedItemID
			
			SET @j = @@ROWCOUNT;

			IF @j > 0
			BEGIN
				WHILE @i <= @j
				BEGIN
					SELECT @intItemID = ItemID, @dt2OwnershipStartDate = OwnershipStartDate, @insItemCount = ItemCount
						, @bitSoulBoundFlag = SoulBoundFlag, @inySealCount = SealCount, @inyItemOption = ItemOption, @inyItemPotential = ItemPotential
					FROM @tblDeletedDragonJewels
					WHERE Seq = @i;
	
					EXEC dbo.P_AddItemLog
						  @dt2LogDate = @dt2Now
						, @inbItemSerial = @inbItemSerial
						, @intItemID = @intItemID
						, @inyItemLogCode = 38
						, @inbOwnerCharacterID = NULL
						, @inbOwnerCharacterIDBefore = @inbCharacterID
						, @insItemCount = 0
						, @insItemCountBefore = @insItemCount
						, @inbItemLogFKey1 = 0
						, @inbItemLogFKey2 = NULL
						, @dt2OwnershipStartDate = @dt2OwnershipStartDate
						, @intIP = NULL
						, @inySealCount = @inySealCount
						, @bitSoulBoundFlag = @bitSoulBoundFlag
						, @inyItemOption = @inyItemOption
						, @inyItemPotential = @inyItemPotential
						, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount						
						, @inyDragonJewelType = @inyDragonJewelType;

					SET @i += 1;	
				END						
			END					
		END

		SELECT @intItemID = ItemID, @dt2OwnershipStartDate = OwnershipStartDate, @insItemCount = ItemCount
			, @bitSoulBoundFlag = SoulBoundFlag, @inySealCount = SealCount, @inyItemOption = ItemOption, @inyItemPotential = ItemPotential
		FROM @tblTemp;
		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 38
			, @inbOwnerCharacterID = NULL
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = 0
			, @insItemCountBefore = @insItemCount
			, @inbItemLogFKey1 = 0
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = @dt2OwnershipStartDate
			, @intIP = NULL
			, @inySealCount = @inySealCount
			, @bitSoulBoundFlag = @bitSoulBoundFlag
			, @inyItemOption = @inyItemOption
			, @inyItemPotential = @inyItemPotential
			, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount
			, @inyDragonJewelType = @inyDragonJewelType;

	END

END TRY
BEGIN CATCH
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
