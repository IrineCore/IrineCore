USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModMaterializedItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 22
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-22
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
103296 = ??? ?? ??????.
103305 = ??? DB??? ??? ???? ?? ???? ????.
history :
4 = ItemAttributeLogs??? ?? ItemLogs???? ????? ?? & ???? ?? ?? ???? ??? ?? at 2010-04-26
5 = IP? ????? ?? at 2010-06-28
6 = ??? ??? ?? ??? ? ??? ??. ?, ? ?? ??? ?? ??? ???? ????. at 2010-06-28
7 = ??? ?? ????? ?? at 2010-08-11
8 = ??? ??? ???? ??? ???? AccountLevelCode ?? 0?? ?? ????? ?? at 2011-02-05
9 = ??? ?? ?? at 2011-03-18
10 = ??? ??? ?? ?? at 2011-03-29
12 = ?? ????? ??? ???? ?? ?? ????? ?? at 2011-04-20
15 = ??? ?? or ???? ??? ?? ??? ??, ItemLogCode ?? 12? ????? ?? at 2011-09-16
16 = (52315) ??? ???? ?? ?? ??? ????? ?? by ??? at 2012-02-29
17 = [dbo.MaterializedItems]? ItemPotentialMoveCount(?? ?? ??) ?? ??     by ??? at 2012-04-09
18 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-20
19 = ??? ??? ????? ?? ItemID ??? ??? ?? by ??? at 2012-06-05
20 = ??? ??? ????? ?? ItemID ? ???? ?? ???? Default? 308281618 -> -1 ? ?? by ??? at 2012-07-06
21 = Input Parameter?? (@inyItemPotentialMoveCount) by ??? at 2012-12-14
**/

CREATE PROCEDURE [dbo].[P_ModMaterializedItem]
	  @inbItemSerial bigint --// ??? ?? ??
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @intModFieldBitmap int --// ??? ??? bitmap
	, @insItemDurability smallint --// ??? (0)
	, @inyItemLevel tinyint --// ??? ?? (1)
	, @inyItemPotential tinyint --// ??? ??? (2)
	, @bitSoulBoundFlag bit --// 0=??, 1=????? (3)
	, @inySealCount tinyint --// ?? ??? (4)
	, @inyItemOption tinyint --// ??? ?? (5)
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @inbCharacterID bigint = NULL --// ?? ?? ?? ??? ???
	, @intSupportItemID int = -1 --// ??? ????? ItemID (? : ??????)
	, @inyItemPotentialMoveCount tinyint = NULL --// ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intItemID int
	, @dt2LogDate datetime2(0) = GETDATE()
	, @bitSoulBoundFlagAfter bit
	, @inySealCountAfter tinyint
	, @inyItemOptionAfter tinyint
	, @inyItemPotentialAfter tinyint
	, @inbOwnerCharacterID bigint
	, @insItemCount smallint
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @inyPreviousItemLevel tinyint
	, @inyCurrentItemLevel tinyint
	, @inyItemLogCode tinyint
	, @inyItemPotentialBefore tinyint
	, @inbMaterializedItemID bigint
	, @inyItemPotentialMoveCountBefore tinyint
	, @inyItemPotentialMoveCountAfter tinyint
	, @inbItemLogFKey1 bigint
	, @inbItemLogFKey2 bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# [MaterializedItems] ???? UPDATE???.*/
	UPDATE dbo.MaterializedItems
	SET @inbMaterializedItemID = MaterializedItemID
		, @intItemID = ItemID
		, @inbOwnerCharacterID = OwnerCharacterID
		, @insItemCount = ItemRemainCount
		, @inyItemPotentialBefore = ItemPotential
		, @inyPreviousItemLevel = ItemLevel
		, ItemDurability = CASE @intModFieldBitmap & POWER(2, 0) WHEN 0 THEN ItemDurability ELSE @insItemDurability END
		, @inyCurrentItemLevel = ItemLevel = CASE @intModFieldBitmap & POWER(2, 1) WHEN 0 THEN ItemLevel ELSE @inyItemLevel END
		, @inyItemPotentialAfter = ItemPotential = CASE @intModFieldBitmap & POWER(2, 2) WHEN 0 THEN ItemPotential ELSE @inyItemPotential END
		, @bitSoulBoundFlagAfter = SoulBoundFlag = CASE @intModFieldBitmap & POWER(2, 3) WHEN 0 THEN SoulBoundFlag ELSE @bitSoulBoundFlag END
		, @inySealCountAfter = SealCount = CASE @intModFieldBitmap & POWER(2, 4) WHEN 0 THEN SealCount ELSE @inySealCount END
		, @inyItemOptionAfter = ItemOption = CASE @intModFieldBitmap & POWER(2, 5) WHEN 0 THEN ItemOption ELSE @inyItemOption END
		, @inyItemPotentialMoveCountBefore = ItemPotentialMoveCount
		, @inyItemPotentialMoveCountAfter = ItemPotentialMoveCount = CASE WHEN @intModFieldBitmap & POWER(2, 2) > 0 AND @inyItemPotentialAfter <> @inyItemPotentialBefore AND (@inyItemPotentialAfter = 0 OR @inyItemPotentialMoveCount IS NOT NULL) THEN @inyItemPotentialMoveCount ELSE ItemPotentialMoveCount END
	WHERE ItemSerial = @inbItemSerial;
	/**_# ??? ??? ???? ??? 103151? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END
	/**_# ????? DB ?? ??? ???? ?? ???? ??? 103305? ?????.*/
	IF ISNULL(@inbCharacterID, 0) > 0 AND @inbCharacterID <> @inbOwnerCharacterID
	BEGIN
		SET @intReturnValue = 103305;
		GOTO ErrorHandler
	END

	COMMIT TRANSACTION;

	SET @inyItemLogCode  =
		CASE
			WHEN @intModFieldBitmap & POWER(2, 1) = 0 OR @inyPreviousItemLevel = @inyCurrentItemLevel THEN 40
			ELSE 12
		END;

	/**_# ????? ??? ??? ?? ????? ItemID? ????. */
	SET @inbItemLogFKey1 = CASE WHEN @inyItemLogCode = 12 THEN @intSupportItemID ELSE @intMapID END;
	SET @inbItemLogFKey2 = CASE WHEN @inyItemLogCode = 12 THEN @intMapID ELSE NULL END;

	SET @inyItemPotentialMoveCount = CASE WHEN @inyItemPotentialMoveCountBefore <> @inyItemPotentialMoveCountAfter THEN @inyItemPotentialMoveCountAfter ELSE @inyItemPotentialMoveCountBefore END;
	/**_# ??? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_AddItemLog
		  @dt2LogDate = @dt2LogDate
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = @inyItemLogCode
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = @inbItemLogFKey1
		, @inbItemLogFKey2 = @inbItemLogFKey2
		, @dt2OwnershipStartDate = NULL
		, @intIP = @intIP
		, @inySealCount = @inySealCountAfter
		, @bitSoulBoundFlag = @bitSoulBoundFlagAfter
		, @inyItemOption = @inyItemOptionAfter
		, @inyItemPotential = @inyItemPotentialAfter
		, @inyItemLevelBefore = @inyPreviousItemLevel
		, @inyItemLevel = @inyCurrentItemLevel
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
