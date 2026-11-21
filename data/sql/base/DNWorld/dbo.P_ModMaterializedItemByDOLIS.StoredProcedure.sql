USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModMaterializedItemByDOLIS]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-16
description : ??? ???? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
history :
2 = ??? ?? ?? at 2011-03-18
3 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_ModMaterializedItemByDOLIS]
	  @inbItemSerial bigint --// ??? ?? ??
	, @intModFieldBitmap int --// ??? ??? bitmap
	, @insItemDurability smallint --// ??? (0)
	, @inyItemLevel tinyint --// ??? ?? (1)
	, @inyItemPotential tinyint --// ??? ??? (2)
	, @bitSoulBoundFlag bit --// 0=??, 1=?? (3)
	, @inySealCount tinyint --// ?? ??? ?? (4)
	, @inyItemOption tinyint --// ??? ?? (5)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intItemID int
	, @dt2LogDate datetime2(0) = GETDATE()
	, @inyItemLevelBefore tinyint
	, @bitSoulBoundFlagAfter bit
	, @inySealCountAfter tinyint
	, @inyItemOptionAfter tinyint
	, @inyItemLevelAfter tinyint
	, @inyItemPotentialAfter tinyint
	, @insItemDurabilityAfter smallint
	, @inbOwnerCharacterID bigint
	, @insItemCount smallint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [MaterializedItems] ???? UPDATE???.*/
	UPDATE dbo.MaterializedItems
	SET @intItemID = ItemID
		, @inbOwnerCharacterID = OwnerCharacterID
		, @insItemCount = ItemRemainCount
		, @inyItemLevelBefore = ItemLevel
		, @insItemDurabilityAfter = ItemDurability = CASE @intModFieldBitmap & POWER(2, 0) WHEN 0 THEN ItemDurability ELSE @insItemDurability END
		, @inyItemLevelAfter = ItemLevel = CASE @intModFieldBitmap & POWER(2, 1) WHEN 0 THEN ItemLevel ELSE @inyItemLevel END
		, @inyItemPotentialAfter = ItemPotential = CASE @intModFieldBitmap & POWER(2, 2) WHEN 0 THEN ItemPotential ELSE @inyItemPotential END
		, @bitSoulBoundFlagAfter = SoulBoundFlag = CASE @intModFieldBitmap & POWER(2, 3) WHEN 0 THEN SoulBoundFlag ELSE @bitSoulBoundFlag END
		, @inySealCountAfter = SealCount = CASE @intModFieldBitmap & POWER(2, 4) WHEN 0 THEN SealCount ELSE @inySealCount END
		, @inyItemOptionAfter = ItemOption = CASE @intModFieldBitmap & POWER(2, 5) WHEN 0 THEN ItemOption ELSE @inyItemOption END
	WHERE ItemSerial = @inbItemSerial;

	/**_# ??? ??? ???? ??? 103151? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_AddItemLog
		  @dt2LogDate = @dt2LogDate
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 44
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = NULL
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL
		, @inySealCount = @inySealCountAfter
		, @bitSoulBoundFlag = @bitSoulBoundFlagAfter
		, @inyItemOption = @inyItemOptionAfter
		, @inyItemPotential = @inyItemPotentialAfter
		, @insItemDurability = @insItemDurabilityAfter
		, @inyItemLevel = @inyItemLevelAfter
		, @inyItemLevelBefore = @inyItemLevelBefore;

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
