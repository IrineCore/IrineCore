USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ExpireMaterializedItem2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ??? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3~7 = (44494) input parameter ?? (@inyVersion) by ??? at 2011-10-31
8 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
10 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-18
11 = ?? ??? ???? ?? by ??? at 2013-06-03
**/

CREATE PROCEDURE [dbo].[P_ExpireMaterializedItem2]
	  @inbCharacterID bigint --// ??? ID
	, @inyVersion tinyint = 0 --// ?? ?? (0=??, 1=??? ItemSerial ??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE(), @i int;
DECLARE @dtsNow smalldatetime = CAST(@dt2Now AS smalldatetime);
DECLARE @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dt2Now);

DECLARE @tblExpiredItems table (
	MaterializedItemID bigint NOT NULL,
	ItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	OwnershipStartDate datetime2(0) NOT NULL,
	ItemRemainCount smallint NOT NULL,
	PayMethodCode tinyint NULL,
	EternityFlag bit NOT NULL,
	ItemLevel tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	ItemPotentialMoveCount tinyint NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	DELETE dbo.MaterializedItems
	OUTPUT deleted.MaterializedItemID, deleted.ItemSerial, deleted.ItemID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.PayMethodCode, deleted.EternityFlag, deleted.ItemLevel, deleted.ItemPotential, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotentialMoveCount
	INTO @tblExpiredItems (MaterializedItemID, ItemSerial, ItemID, OwnershipStartDate, ItemRemainCount, PayMethodCode, EternityFlag, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount)
	FROM dbo.MaterializedItems WITH (INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
	WHERE OwnerCharacterID = @inbCharacterID
		AND EternityFlag = CAST(0 AS bit)
		AND ItemExpireDate <= @dtsNow
		AND ItemLocationCode <> 10
		AND NOT (ItemLocationCode = 1 AND ItemLocationIndex = 254)
	OPTION (KEEPFIXED PLAN);

	SET @i = @@ROWCOUNT;

	DELETE dbo.MaterializedItems
	OUTPUT deleted.MaterializedItemID, deleted.ItemSerial, deleted.ItemID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.PayMethodCode, deleted.EternityFlag, deleted.ItemLevel, deleted.ItemPotential, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotentialMoveCount
	INTO @tblExpiredItems (MaterializedItemID, ItemSerial, ItemID, OwnershipStartDate, ItemRemainCount, PayMethodCode, EternityFlag, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount)
	FROM dbo.MaterializedItems WITH (INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
	WHERE OwnerCharacterID = @inbCharacterID
		AND ItemLocationCode = 11
		AND ItemMaterializeDate <= DATEADD(DAY, -15, @dt2Now)
	OPTION (KEEPFIXED PLAN);

	SET @i += @@ROWCOUNT;

	DELETE dbo.MaterializedItems
	OUTPUT deleted.MaterializedItemID, deleted.ItemSerial, deleted.ItemID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.PayMethodCode, deleted.EternityFlag, deleted.ItemLevel, deleted.ItemPotential, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotentialMoveCount
	INTO @tblExpiredItems (MaterializedItemID, ItemSerial, ItemID, OwnershipStartDate, ItemRemainCount, PayMethodCode, EternityFlag, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount)
	FROM @tblExpiredItems A
		INNER LOOP JOIN dbo.PetEquipments B WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON A.ItemSerial = B.PetItemSerial AND B.PetEquipmentIndex = 2
		INNER LOOP JOIN dbo.MaterializedItems C WITH (INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON B.PetEquipmentItemSerial = C.ItemSerial
	OPTION (FORCE ORDER, KEEPFIXED PLAN);

	SET @i += @@ROWCOUNT;
	/**_## (@inyVersion = 1) ? ???? ??? ItemSerial ?? */
	IF @inyVersion = 1
	BEGIN
		SELECT ItemSerial FROM @tblExpiredItems;
	END

	IF @i > 0
	BEGIN
		UPDATE dbo.Trade
		SET TradeStatusCode = 3, TradeDate = CAST(@dtsNow AS datetime2(0))
		FROM @tblExpiredItems T
			INNER LOOP JOIN dbo.Trade TR WITH (INDEX(IX_NNI_Trade_ItemSerial), FORCESEEK) ON TR.ItemSerial = T.ItemSerial
		WHERE TR.TradeStatusCode = 0
		OPTION (KEEPFIXED PLAN);

		DELETE dbo.PetEquipments
		FROM @tblExpiredItems T
			INNER LOOP JOIN dbo.PetEquipments PE WITH (INDEX(IX_UNF_PetEquipments_PetEquipmentItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = T.ItemSerial
		OPTION (KEEPFIXED PLAN);

		DELETE dbo.PetEquipments
		FROM @tblExpiredItems T
			INNER LOOP JOIN dbo.PetEquipments PE WITH (INDEX(PK_PetEquipments), FORCESEEK) ON PE.PetItemSerial = T.ItemSerial
		OPTION (KEEPFIXED PLAN);

		DELETE dbo.ManufactureSkillRecipes
		FROM @tblExpiredItems T
			INNER LOOP JOIN dbo.ManufactureSkillRecipes MSR WITH (INDEX(IX_UN_ManufactureSkillRecipes_ItemSerial), FORCESEEK) ON MSR.ItemSerial = T.ItemSerial
		WHERE MSR.CharacterID = @inbCharacterID AND MSR.SecondarySkillID > 0
		OPTION (KEEPFIXED PLAN);

		DELETE dbo.Fairies
		FROM @tblExpiredItems T
			INNER LOOP JOIN dbo.Fairies F WITH (INDEX(PK_Fairies), FORCESEEK) ON F.FairyItemSerial = T.ItemSerial
		OPTION (KEEPFIXED PLAN);

		/**_## [ItemLogs] ???? INSERT???.*/
		IF @intLogTableSuffix = 0
			INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount)
			SELECT @dt2Now, ItemSerial, ItemID, 27, NULL, @inbCharacterID, 0, ItemRemainCount, NULL, NULL, OwnershipStartDate, NULL, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount 
			FROM @tblExpiredItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 27))
			OPTION (KEEPFIXED PLAN);

		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount)
			SELECT @dt2Now, ItemSerial, ItemID, 27, NULL, @inbCharacterID, 0, ItemRemainCount, NULL, NULL, OwnershipStartDate, NULL, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount 
			FROM @tblExpiredItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 27))
			OPTION (KEEPFIXED PLAN);

		ELSE
			INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount)
			SELECT @dt2Now, ItemSerial, ItemID, 27, NULL, @inbCharacterID, 0, ItemRemainCount, NULL, NULL, OwnershipStartDate, NULL, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemPotentialMoveCount 
			FROM @tblExpiredItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 27))
			OPTION (KEEPFIXED PLAN);
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
