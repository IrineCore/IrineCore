USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_MoveItemPotential]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-02-28
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
103332 = ? ?? ??? ??? ? ????. (?? ?? ?? ??)
history :
3~6 = [dbo.MaterializedItems]? ItemPotentialMoveCount(?? ?? ??) ?? ??     by ??? at 2012-04-09
7 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-18
8~9 = ??? ?? ????? ?? ?? by ??? at 2012-04-18
**/

CREATE PROCEDURE [dbo].[P_MoveItemPotential]
	  @inbCharacterID bigint --// ??? ID
	, @inbSourceItemSerial bigint --// ?? ???? ???
	, @inySourceItemPotential tinyint --// ?? ???? ?? ??? ??? Potential ?
	, @inbTargetItemSerial bigint --// ?? ???? ???
	, @inyTargetItemPotential tinyint --// ?? ???? ??? ??? ? ??? Potential ?
	, @inyItemPotentialMoveCountLimit tinyint --// ?? ??? ??? ? ?? ?? ?? (? ??? ???? exception? ?????)
	, @intMapID int --// ???? ??? Map ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intSourceItemID int
	, @intTargetItemID int
	, @dt2LogDate datetime2(0) = GETDATE()
	, @inbSourceMaterializedItemID bigint
	, @bitSourceSoulBoundFlagB4 bit
	, @inySourceSealCountB4 tinyint
	, @inySourceItemOptionB4 tinyint
	, @inySourceItemPotentialB4 tinyint
	, @insSourceItemCountB4 smallint
	, @inySourceItemLevelB4 tinyint
	, @inbTargetMaterializedItemID bigint
	, @bitTargetSoulBoundFlagB4 bit
	, @inyTargetSealCountB4 tinyint
	, @inyTargetItemOptionB4 tinyint
	, @inyTargetItemPotentialB4 tinyint
	, @insTargetItemCountB4 smallint
	, @inyTargetItemLevelB4 tinyint
	, @inyItemPotentialMoveCount tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	UPDATE dbo.MaterializedItems
	SET @inbSourceMaterializedItemID = MaterializedItemID
		, @intSourceItemID = ItemID
		, @bitSourceSoulBoundFlagB4 = SoulBoundFlag
		, @inySourceSealCountB4 = SealCount
		, @inySourceItemOptionB4 = ItemOption
		, @inySourceItemPotentialB4 = ItemPotential
		, @insSourceItemCountB4 = ItemRemainCount
		, @inySourceItemLevelB4 = ItemLevel
		, ItemPotential = 0
		, @inyItemPotentialMoveCount = ISNULL(ItemPotentialMoveCount, 0) + 1
		, ItemPotentialMoveCount = 0
	FROM dbo.MaterializedItems WITH (INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
	WHERE ItemSerial = @inbSourceItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemPotential = @inySourceItemPotential
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	UPDATE dbo.MaterializedItems
	SET @inbTargetMaterializedItemID = MaterializedItemID
		, @intTargetItemID = ItemID
		, @bitTargetSoulBoundFlagB4 = SoulBoundFlag
		, @inyTargetSealCountB4 = SealCount
		, @inyTargetItemOptionB4 = ItemOption
		, @inyTargetItemPotentialB4 = ItemPotential
		, @insTargetItemCountB4 = ItemRemainCount
		, @inyTargetItemLevelB4 = ItemLevel
		, ItemPotential = @inyTargetItemPotential
		, ItemPotentialMoveCount = @inyItemPotentialMoveCount
	FROM dbo.MaterializedItems WITH (INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
	WHERE ItemSerial = @inbTargetItemSerial AND OwnerCharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	IF @inyItemPotentialMoveCount > @inyItemPotentialMoveCountLimit
	BEGIN
		SET @intReturnValue = 103332;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2LogDate
		, @inbItemSerial = @inbSourceItemSerial
		, @intItemID = @intSourceItemID
		, @inyItemLogCode = 62
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = @inbCharacterID
		, @insItemCount = @insSourceItemCountB4
		, @insItemCountBefore = @insSourceItemCountB4
		, @inbItemLogFKey1 = @intMapID
		, @inbItemLogFKey2 = @inbTargetItemSerial
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL
		, @inySealCount = @inySourceSealCountB4
		, @bitSoulBoundFlag = @bitSourceSoulBoundFlagB4
		, @inyItemOption = @inySourceItemOptionB4
		, @inyItemPotential = 0
		, @inyItemLevelBefore = @inySourceItemLevelB4
		, @inyItemLevel = @inySourceItemLevelB4
		, @inyItemPotentialMoveCount = 0;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2LogDate
		, @inbItemSerial = @inbTargetItemSerial
		, @intItemID = @intTargetItemID
		, @inyItemLogCode = 63
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = @inbCharacterID
		, @insItemCount = @insTargetItemCountB4
		, @insItemCountBefore = @insTargetItemCountB4
		, @inbItemLogFKey1 = @intMapID
		, @inbItemLogFKey2 = @inbSourceItemSerial
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL
		, @inySealCount = @inyTargetSealCountB4
		, @bitSoulBoundFlag = @bitTargetSoulBoundFlagB4
		, @inyItemOption = @inyTargetItemOptionB4
		, @inyItemPotential = @inyTargetItemPotential
		, @inyItemLevelBefore = @inyTargetItemLevelB4
		, @inyItemLevel = @inyTargetItemLevelB4
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount;

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
