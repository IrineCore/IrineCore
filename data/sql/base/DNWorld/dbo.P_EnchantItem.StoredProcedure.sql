USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_EnchantItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 56
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-22
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103123 = ??? ?????.
103153 = ??? ???? ???? ????.
103154 = ?? ?? ???? ???? ????.
103305 = ??? DB??? ??? ???? ?? ???? ????.
103306 = ?? ?? ? "????"? ?? ????.
103341 = ?? ?? ?? ???? ???? ????.
history :
27 = ItemEnchantLogs, ItemEnchantMaterials ???? ??? ??? ?? ItemLogs ???? ???? ?? at 2010-04-26
28 = ?? ???? ???? ???? ???? OwnershipStartDate? ????? ?? at 2010-04-26
29 = ?? ?? ?? ??? ????? ?? at 2010-07-06
30 = SkipLoggingItems ???? ???? ?? ??? ????? ?? at 2010-08-09
31 = TF_Split? IF_Split?? ?? at 2010-12-21
32 = ???? ??? ????? ????? ?? at 2010-12-30
33 = ???? at 2011-03-29
35 = ItemLocation.MaterializedItemID ?? ??? ?? ?? at 2011-03-31
36 = ?? ?? ?? ??? ??? ?? ??? ?? at 2011-04-20
38 = ???? ??? ? ?? ?? ?? ?? at 2011-05-03
44 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
45 = [ItemLocation] ??? ??? ?? ?? by ??? at 2011-12-20
47 = (63036) ?? ?? ?? ?? by ??? at 2012-07-24
48~49 = ?? ??? ?? ?? by ??? at 2012-07-30
51 = ??? ?? ??? ?? ?? by ??? at 2013-03-19
54~55 = ??? ?? ??? ??? ??? ????? ?? by ??? at 2013-03-25
**/

CREATE PROCEDURE [dbo].[P_EnchantItem]
	  @inbItemSerial bigint --// ??? ?? ??
	, @vchMaterialItemSerials varchar(8000) --// ,? ?????? ?? ??? ??? ??? ?? ?? ???
	, @vchMaterialItemCounts varchar(8000) --// ,? ?????? ?? ??? ??? ??? ?? ???
	, @inbCoin bigint --// ?? ??
	, @bitEnchantSuccessFlag bit --// 0=??, 1=??
	, @bitItemDestroyFlag bit --// ??? ??? ?? ?? ???? ?????? ??. 0=???? ??, 1=???
	, @inyCurrentItemLevel tinyint --// ?? ? ??
	, @inyCurrentItemPotential tinyint --// ?? ? ??? ???
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @intNPCID int --// ?? NPC ID
	, @inbNewItemSerial bigint = NULL --// (optional) ?? ?? ? ?? ???? ??? ?? ??
	, @intNewItemID int = NULL --// (optional) ?? ?? ? ?? ???? ??? ID
	, @insNewItemCount smallint = NULL --// (optional) ??? ??
	, @insNewItemDurability smallint = NULL --// (optional) ???
	, @intNewRandomSeed int = NULL --// (optional) ?? ??
	, @inyNewItemLevel tinyint = NULL --// (optional) ??? ??
	, @intNewItemPotential tinyint = NULL --// (optional) ??? ???
	, @bitNewSoulBoundFlag bit = NULL --// (optional) 0=??, 1=?????
	, @inyNewSealCount tinyint = NULL --// (optional) ?? ???
	, @inyNewItemOption tinyint = NULL --// (optional) ??? ??
	, @bitNewEternityFlag bit = NULL --// (optional) 0=???? ?? ??, 1=?? ???
	, @intNewItemLifespan int = NULL --// (optional) ??? ?? (?)
	, @inbCharacterID bigint = NULL --// ?? ?? ?? ??? ??? (=?? ???)
	, @intSupportItemID int = NULL --// ??? ?? ?? ???? ItemID (? : ??????)
	, @inbOriginalItemSerial bigint = NULL --// ?? ??? ??, ?? ?? ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2EnchantDate datetime2(0) = GETDATE()
	, @insItemRemainCountBefore smallint
	, @insItemRemainCountAfter smallint
	, @insUsedItemCount smallint
	, @intLogTableSuffix int
	, @intItemID int
	, @dt2OwnershipStartDate datetime2(0)
	, @inyPayMethodCode tinyint
	, @inbCurrentCoin bigint
	, @inyPreviousItemLevel tinyint
	, @inbMaterializedItemID bigint
	, @inbNewMaterializedItemID bigint
	, @intMaterialItemCount int
	, @inySealCount tinyint
	, @inyItemOption tinyint
	, @intOriginalItemID int
	, @inyOriginalItemCount tinyint
	, @dt2OriginalItemOwnershipStartDate datetime2(0)
	, @inyOriginalItemPotential tinyint
	, @inyOriginalItemOption tinyint
	, @inyOriginalSealCount tinyint;

DECLARE @tblTemp1 table (seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemSerial bigint NOT NULL);
DECLARE @tblTemp2 table (seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemCount smallint NOT NULL);
DECLARE @tblMaterialItems table (ItemSerial bigint NOT NULL, ItemCount smallint NOT NULL);
DECLARE @tblMaterialItems2 table (MaterializedItemID bigint NOT NULL PRIMARY KEY, ItemSerial bigint NOT NULL, ItemID int NOT NULL, OwnershipStartDate datetime2(0) NULL, ItemRemainCountBefore smallint NOT NULL, ItemRemainCountAfter smallint NOT NULL, PayMethodCode tinyint NULL);
DECLARE @tblDeletedDragonJewelIDs table (MaterializedItemID bigint NOT NULL);
DECLARE @tblDeletedDragonJewels table (
	ItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	RemainItemCount smallint NOT NULL, 	
	ItemLevel tinyint NOT NULL,
	ItemOption tinyint NULL,
	ItemPotential tinyint NULL,
	ItemPotentialMoveCount tinyint NULL,
	SealCount tinyint NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inbCharacterID IS NULL
		SELECT @inbCharacterID = OwnerCharacterID
		FROM dbo.MaterializedItems WITH (NOLOCK)
		WHERE ItemSerial = @inbItemSerial;

	/**_# ?? ??? ??? ??????.*/
	INSERT @tblTemp1(ItemSerial)
	SELECT CAST(string AS bigint) 
	FROM dbo.TF_Split(@vchMaterialItemSerials, ',') 
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);

	IF EXISTS (
		SELECT *
		FROM @tblTemp1 T
			INNER JOIN dbo.Trade TR WITH (NOLOCK) ON T.ItemSerial = TR.ItemSerial
		WHERE TR.TradeStatusCode = 0
	)
	BEGIN
		UPDATE dbo.Trade
		SET TradeStatusCode = 3, TradeDate = @dt2EnchantDate
		FROM @tblTemp1 T
			INNER JOIN dbo.Trade TR WITH (NOLOCK) ON T.ItemSerial = TR.ItemSerial;
	END

	IF EXISTS (
		SELECT *
		FROM dbo.Trade WITH (NOLOCK)
		WHERE ItemSerial = @inbItemSerial AND TradeStatusCode = 0
	)
	BEGIN
		UPDATE dbo.Trade
		SET TradeStatusCode = 3, TradeDate = @dt2EnchantDate
		WHERE ItemSerial = @inbItemSerial;
	END

	INSERT @tblTemp2(ItemCount)
	SELECT CAST(string AS smallint) 
	FROM dbo.TF_Split(@vchMaterialItemCounts, ',') 
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);

	INSERT @tblMaterialItems (ItemSerial, ItemCount)
	SELECT A.ItemSerial, SUM(B.ItemCount)
	FROM @tblTemp1 A
		INNER JOIN @tblTemp2 B ON A.seq = B.seq
	GROUP BY A.ItemSerial;

	SET @intMaterialItemCount = @@ROWCOUNT;

	/**_# ?? ???? ??? ?????.*/
	BEGIN TRANSACTION;

	UPDATE dbo.MaterializedItems
	SET ItemRemainCount = (MI.ItemRemainCount - T.ItemCount)
	OUTPUT deleted.MaterializedItemID, deleted.ItemSerial, deleted.ItemID, deleted.OwnershipStartDate, deleted.ItemRemainCount, inserted.ItemRemainCount, deleted.PayMethodCode
	INTO @tblMaterialItems2 (MaterializedItemID, ItemSerial, ItemID, OwnershipStartDate, ItemRemainCountBefore, ItemRemainCountAfter, PayMethodCode)
	FROM dbo.MaterializedItems MI WITH (NOLOCK)
		INNER JOIN @tblMaterialItems T ON MI.ItemSerial = T.ItemSerial
	WHERE MI.OwnerCharacterID = @inbCharacterID;

	IF @@ROWCOUNT <> @intMaterialItemCount
	BEGIN
		SET @intReturnValue = 103154;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	/**_# ?? ??? 0? ?? ???? ???? ?????.*/
	DELETE dbo.MaterializedItems
	FROM dbo.MaterializedItems MI
		INNER JOIN @tblMaterialItems2 T ON MI.MaterializedItemID = T.MaterializedItemID
	WHERE T.ItemRemainCountAfter = 0;

	/**_# ?? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_ModCoinForSubProc
		  @inbCharacterID = @inbCharacterID
		, @inyCoinChangeCode = 16
		, @inbCoinChangeFKey = NULL
		, @inbCoin = @inbCoin
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID
		, @dt2LogDate = @dt2EnchantDate;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	/**_# ?? ??? ??, ?? ???? ????? 0?? ?????.*/
	IF @inbOriginalItemSerial > 0
	BEGIN
		UPDATE dbo.MaterializedItems
		SET @intOriginalItemID = ItemID
		  , @inyOriginalItemCount = ItemRemainCount
	      , @dt2OriginalItemOwnershipStartDate = OwnershipStartDate
		  , @inyOriginalItemPotential = ItemPotential
		  , @inyOriginalItemOption = ItemOption
		  , @inyOriginalSealCount = SealCount
          , ItemLevel = 0
		FROM dbo.MaterializedItems WITH (INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
		WHERE ItemSerial = @inbOriginalItemSerial
			AND OwnerCharacterID = @inbCharacterID
			AND ItemLevel = @inyCurrentItemLevel
			AND ItemLocationCode = 2
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103341;
			GOTO ErrorHandler;
		END
	END

	BEGIN TRANSACTION;
	/**_# ?? ??? ???? ?????.*/
	UPDATE dbo.MaterializedItems
	SET @inbMaterializedItemID = MaterializedItemID
		, @intItemID = ItemID
		, @dt2OwnershipStartDate = OwnershipStartDate
		, @insItemRemainCountBefore = ItemRemainCount
		, @inyPayMethodCode = PayMethodCode
		, @inyPreviousItemLevel = ItemLevel
		, @inyItemOption = ItemOption
		, @inySealCount = SealCount
		, ItemLevel = @inyCurrentItemLevel
		, ItemPotential = @inyCurrentItemPotential
		, @insItemRemainCountAfter = ItemRemainCount = CASE @bitItemDestroyFlag WHEN 1 THEN 0 ELSE ItemRemainCount END
		, OwnerCharacterID = CASE @bitItemDestroyFlag WHEN 1 THEN NULL ELSE OwnerCharacterID END
		, ItemLocationCode = CASE @bitItemDestroyFlag WHEN 1 THEN 2 ELSE ItemLocationCode END --// ???? ?????? IX_UNF_MaterializedItems_OwnerCharacterID_ItemLocationCode_ItemLocationIndex ??? ?? ? ? ??? ?????? ??? ???.
	FROM dbo.MaterializedItems WITH (INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
	WHERE ItemSerial = @inbItemSerial
		AND OwnerCharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	/**_# ??? ???? ???? ??? 103153? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103153;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ? "????"? +1? ??? 103306? ?????.*/
	IF (@inbOriginalItemSerial IS NULL OR @inbOriginalItemSerial <= 0) AND @bitEnchantSuccessFlag = 1 AND (@inyCurrentItemLevel <> @inyPreviousItemLevel + 1)
	BEGIN
		SET @intReturnValue = 103306;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	SET @insUsedItemCount = @insItemRemainCountBefore - @insItemRemainCountAfter;

	/**_# ?? ??? ???? ?????? ???? ?????.*/
	IF @bitItemDestroyFlag = 1
	BEGIN
		IF @inbNewItemSerial IS NOT NULL
		BEGIN
			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ItemLocationCode, ItemLocationIndex)
			SELECT @inbNewItemSerial, @intNewItemID, @insNewItemCount, NULL, NULL, 20, NULL, @insNewItemDurability, @intNewRandomSeed, 0, @inyNewItemLevel, @intNewItemPotential, @bitNewSoulBoundFlag, @inyNewSealCount, @inyNewItemOption, @dt2EnchantDate, @intNewItemLifespan, @bitNewEternityFlag, DATEADD(MINUTE, CASE @bitNewEternityFlag WHEN 0 THEN @intNewItemLifespan ELSE 525600 END, @dt2EnchantDate), @inbCharacterID, @dt2EnchantDate, ItemLocationCode, ItemLocationIndex
			FROM dbo.MaterializedItems WITH (NOLOCK)
			WHERE MaterializedItemID = @inbMaterializedItemID;

			SET @inbNewMaterializedItemID = SCOPE_IDENTITY();
		END

		DELETE dbo.MaterializedItems
		WHERE MaterializedItemID = @inbMaterializedItemID;

		DELETE dbo.DragonJewels
		OUTPUT deleted.MaterializedItemID
		INTO @tblDeletedDragonJewelIDs (MaterializedItemID)
		WHERE ParentItemMaterializedItemID = @inbMaterializedItemID;

		IF @@ROWCOUNT > 0
		BEGIN
			DELETE dbo.MaterializedItems	
			OUTPUT deleted.ItemSerial, deleted.ItemID, deleted.ItemLevel, deleted.ItemPotential, deleted.ItemPotentialMoveCount, deleted.ItemRemainCount, deleted.SealCount
			INTO @tblDeletedDragonJewels(ItemSerial, ItemID, ItemLevel, ItemPotential, ItemPotentialMoveCount, RemainItemCount, SealCount)
			FROM @tblDeletedDragonJewelIDs A 
				INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(PK_MaterializedItems), FORCESEEK) ON A.MaterializedItemID = MI.MaterializedItemID;						
		END
	END

	/**_# [ItemLogs] ???? INSERT???.*/
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2EnchantDate);

	IF @intLogTableSuffix = 0
		INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, ItemLevelBefore, SealCount, ItemOption)
		SELECT @dt2EnchantDate, @inbItemSerial, @intItemID, 12, CASE @bitItemDestroyFlag WHEN 1 THEN NULL ELSE @inbCharacterID END, @inbCharacterID, @insItemRemainCountAfter, @insItemRemainCountBefore, @intSupportItemID, @intMapID, CASE @bitItemDestroyFlag WHEN 1 THEN @dt2OwnershipStartDate ELSE NULL END, NULL, @inyCurrentItemLevel, @inyCurrentItemPotential, @inyPreviousItemLevel, @inySealCount, @inyItemOption
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intItemID AND (ItemLogCode IS NULL OR ItemLogCode = 12))
		UNION ALL
		SELECT @dt2EnchantDate, @inbNewItemSerial, @intNewItemID, 39, @inbCharacterID, NULL, @insNewItemCount, 0, NULL, @intMapID, @dt2EnchantDate, NULL, @inyNewItemLevel, @intNewItemPotential, NULL, NULL, NULL
		WHERE @bitItemDestroyFlag = 1 AND @inbNewItemSerial IS NOT NULL AND NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intNewItemID AND (ItemLogCode IS NULL OR ItemLogCode = 39))
		UNION ALL
		SELECT @dt2EnchantDate, ItemSerial, ItemID, 24, CASE ItemRemainCountAfter WHEN 0 THEN NULL ELSE @inbCharacterID END, @inbCharacterID, ItemRemainCountAfter, ItemRemainCountBefore, NULL, @intMapID, CASE ItemRemainCountAfter WHEN 0 THEN OwnershipStartDate ELSE NULL END, NULL, NULL, NULL, NULL, NULL, NULL
		FROM @tblMaterialItems2 A
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 24))
		UNION ALL
		SELECT @dt2EnchantDate, @inbOriginalItemSerial, @intOriginalItemID, 66, @inbCharacterID, NULL, @inyOriginalItemCount, 0, @inbItemSerial, @intMapID, @dt2OriginalItemOwnershipStartDate, NULL, 0, @inyOriginalItemPotential, @inyCurrentItemLevel, @inyOriginalSealCount, @inyOriginalItemOption
		WHERE @inbOriginalItemSerial > 0 AND NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intOriginalItemID AND (ItemLogCode IS NULL OR ItemLogCode = 66))
		UNION ALL
		SELECT @dt2EnchantDate, ItemSerial, ItemID, 12, NULL, NULL, 0, RemainItemCount, NULL, @intMapID, NULL, NULL, NULL, ItemPotential, ItemLevel, SealCount, ItemOption
		FROM @tblDeletedDragonJewels
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intItemID AND (ItemLogCode IS NULL OR ItemLogCode = 12));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, ItemLevelBefore, SealCount, ItemOption)
		SELECT @dt2EnchantDate, @inbItemSerial, @intItemID, 12, CASE @bitItemDestroyFlag WHEN 1 THEN NULL ELSE @inbCharacterID END, @inbCharacterID, @insItemRemainCountAfter, @insItemRemainCountBefore, @intSupportItemID, @intMapID, CASE @bitItemDestroyFlag WHEN 1 THEN @dt2OwnershipStartDate ELSE NULL END, NULL, @inyCurrentItemLevel, @inyCurrentItemPotential, @inyPreviousItemLevel, @inySealCount, @inyItemOption
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intItemID AND (ItemLogCode IS NULL OR ItemLogCode = 12))
		UNION ALL
		SELECT @dt2EnchantDate, @inbNewItemSerial, @intNewItemID, 39, @inbCharacterID, NULL, @insNewItemCount, 0, NULL, @intMapID, @dt2EnchantDate, NULL, @inyNewItemLevel, @intNewItemPotential, NULL, NULL, NULL
		WHERE @bitItemDestroyFlag = 1 AND @inbNewItemSerial IS NOT NULL AND NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intNewItemID AND (ItemLogCode IS NULL OR ItemLogCode = 39))
		UNION ALL
		SELECT @dt2EnchantDate, ItemSerial, ItemID, 24, CASE ItemRemainCountAfter WHEN 0 THEN NULL ELSE @inbCharacterID END, @inbCharacterID, ItemRemainCountAfter, ItemRemainCountBefore, NULL, @intMapID, CASE ItemRemainCountAfter WHEN 0 THEN OwnershipStartDate ELSE NULL END, NULL, NULL, NULL, NULL, NULL, NULL
		FROM @tblMaterialItems2 A
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 24))
		UNION ALL
		SELECT @dt2EnchantDate, @inbOriginalItemSerial, @intOriginalItemID, 66, @inbCharacterID, NULL, @inyOriginalItemCount, 0, @inbItemSerial, @intMapID, @dt2OriginalItemOwnershipStartDate, NULL, 0, @inyOriginalItemPotential, @inyCurrentItemLevel, @inyOriginalSealCount, @inyOriginalItemOption
		WHERE @inbOriginalItemSerial > 0 AND NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intOriginalItemID AND (ItemLogCode IS NULL OR ItemLogCode = 66))
		UNION ALL
		SELECT @dt2EnchantDate, ItemSerial, ItemID, 12, NULL, NULL, 0, RemainItemCount, NULL, @intMapID, NULL, NULL, NULL, ItemPotential, ItemLevel, SealCount, ItemOption
		FROM @tblDeletedDragonJewels
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intItemID AND (ItemLogCode IS NULL OR ItemLogCode = 12));

	ELSE
		INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, ItemLevelBefore, SealCount, ItemOption)
		SELECT @dt2EnchantDate, @inbItemSerial, @intItemID, 12, CASE @bitItemDestroyFlag WHEN 1 THEN NULL ELSE @inbCharacterID END, @inbCharacterID, @insItemRemainCountAfter, @insItemRemainCountBefore, @intSupportItemID, @intMapID, CASE @bitItemDestroyFlag WHEN 1 THEN @dt2OwnershipStartDate ELSE NULL END, NULL, @inyCurrentItemLevel, @inyCurrentItemPotential, @inyPreviousItemLevel, @inySealCount, @inyItemOption
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intItemID AND (ItemLogCode IS NULL OR ItemLogCode = 12))
		UNION ALL
		SELECT @dt2EnchantDate, @inbNewItemSerial, @intNewItemID, 39, @inbCharacterID, NULL, @insNewItemCount, 0, NULL, @intMapID, @dt2EnchantDate, NULL, @inyNewItemLevel, @intNewItemPotential, NULL, NULL, NULL
		WHERE @bitItemDestroyFlag = 1 AND @inbNewItemSerial IS NOT NULL AND NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intNewItemID AND (ItemLogCode IS NULL OR ItemLogCode = 39))
		UNION ALL
		SELECT @dt2EnchantDate, ItemSerial, ItemID, 24, CASE ItemRemainCountAfter WHEN 0 THEN NULL ELSE @inbCharacterID END, @inbCharacterID, ItemRemainCountAfter, ItemRemainCountBefore, NULL, @intMapID, CASE ItemRemainCountAfter WHEN 0 THEN OwnershipStartDate ELSE NULL END, NULL, NULL, NULL, NULL, NULL, NULL
		FROM @tblMaterialItems2 A
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 24))
		UNION ALL
		SELECT @dt2EnchantDate, @inbOriginalItemSerial, @intOriginalItemID, 66, @inbCharacterID, NULL, @inyOriginalItemCount, 0, @inbItemSerial, @intMapID, @dt2OriginalItemOwnershipStartDate, NULL, 0, @inyOriginalItemPotential, @inyCurrentItemLevel, @inyOriginalSealCount, @inyOriginalItemOption
		WHERE @inbOriginalItemSerial > 0 AND NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intOriginalItemID AND (ItemLogCode IS NULL OR ItemLogCode = 66))
		UNION ALL
		SELECT @dt2EnchantDate, ItemSerial, ItemID, 12, NULL, NULL, 0, RemainItemCount, NULL, @intMapID, NULL, NULL, NULL, ItemPotential, ItemLevel, SealCount, ItemOption
		FROM @tblDeletedDragonJewels
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = @intItemID AND (ItemLogCode IS NULL OR ItemLogCode = 12));

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_MaterializedItems_ItemRemainCount%'
		SET @intReturnValue = 103154;

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
