USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModManufactureSkillRecipe]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-03
description : ?? ??? ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103231 = ?? ?? ???? ???? 0?? ??? ??? ???.
history :
2 = ??? ?? ?? at 2011-03-18
3 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_ModManufactureSkillRecipe]
	  @inbCharacterID bigint --// ??? ID
	, @inbItemSerial bigint --// ???? ??? ???? ??? ????
	, @insItemDurability smallint --// ??? ??? (+/- ?? ??)
	, @insItemDurabilityAfter smallint OUTPUT --// ?? ? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intItemID int
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
		, @insItemCount = ItemRemainCount
		, @insItemDurabilityAfter = ItemDurability += @insItemDurability
	WHERE ItemSerial = @inbItemSerial;

	/**_# ??? ??? ?????.*/
	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 40
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = @inbCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = NULL
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL
		, @inySealCount = NULL
		, @bitSoulBoundFlag = NULL
		, @inyItemOption = NULL
		, @inyItemPotential = NULL
		, @insItemDurability = @insItemDurabilityAfter;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_MaterializedItems_ItemDurability%'
		SET @intReturnValue = 103231;

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
