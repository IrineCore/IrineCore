USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelManufactureSkillRecipe]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-03
description : ?? ?? ???? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103230 = ??? ?? ?? ???? ????.
history :
4 = ItemLog ???? SkipItem ?? ?? ??     by ??? at 2012-01-15
7 = 4?? ??     by ??? at 2012-04-09
**/

CREATE PROCEDURE [dbo].[P_DelManufactureSkillRecipe]
	  @inbCharacterID bigint --// ??? ID
	, @inbItemSerial bigint --// ??? ???? ??? ????
	, @vchIP varchar(15) --// ?????? ??? ?? IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intLogTableSuffix int
	, @dt2Now datetime2(0) = GETDATE();

DECLARE @tblItems table (
	ItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	OwnershipStartDate datetime2(0) NULL,
	SealCount tinyint NULL,
	ItemOption tinyint NULL,
	SoulBoundFlag bit NULL,
	ItemDurability smallint NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# [ManufactureSkillRecipes] ????? DELETE???.*/
	DELETE dbo.ManufactureSkillRecipes
	WHERE CharacterID = @inbCharacterID AND SecondarySkillID > 0 AND ItemSerial = @inbItemSerial;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103230;
		GOTO ErrorHandler;
	END

	/**_# [MaterializedItems] ????? DELETE???.*/
	DELETE dbo.MaterializedItems
	OUTPUT deleted.ItemSerial, deleted.ItemID, deleted.OwnershipStartDate, deleted.SealCount, deleted.ItemOption, deleted.SoulBoundFlag
	INTO @tblItems (ItemSerial, ItemID, OwnershipStartDate, SealCount, ItemOption, SoulBoundFlag)
	WHERE ItemSerial = @inbItemSerial;

	COMMIT TRANSACTION;

	/**_# ??? ??? ?????. (?? ???? ?? "???" ??)*/
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	IF @intLogTableSuffix = 0
		INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 21, NULL, @inbCharacterID, 0, 1, NULL, NULL, T.OwnershipStartDate, dbo.FN_IP2IPNumber(@vchIP), NULL, NULL, T.SealCount, T.ItemOption, T.SoulBoundFlag, NULL, T.ItemDurability
		FROM @tblItems T
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 21));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 21, NULL, @inbCharacterID, 0, 1, NULL, NULL, T.OwnershipStartDate, dbo.FN_IP2IPNumber(@vchIP), NULL, NULL, T.SealCount, T.ItemOption, T.SoulBoundFlag, NULL, T.ItemDurability
		FROM @tblItems T
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 21));

	ELSE
		INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 21, NULL, @inbCharacterID, 0, 1, NULL, NULL, T.OwnershipStartDate, dbo.FN_IP2IPNumber(@vchIP), NULL, NULL, T.SealCount, T.ItemOption, T.SoulBoundFlag, NULL, T.ItemDurability
		FROM @tblItems T
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 21));
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
