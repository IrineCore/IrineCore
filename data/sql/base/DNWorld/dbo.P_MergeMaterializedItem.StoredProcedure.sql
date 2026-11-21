USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_MergeMaterializedItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 15
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-29
description : ???? ??? ????. (???? ?? ?? ?? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103147 = ItemID? ?? ????. ??? ??? ??? ? ????.
history :
5 = Merge ?? ??? ???? MaterializedItems ????? ????? ?? at 2010-07-02
6 = ??? ?? ?? at 2011-03-18
7 = ??? ??? ?? ?? at 2011-03-29
10 = ItemLocation.MaterializedItemID ?? ??? ?? ?? at 2011-03-31
12 = ItemLevel > 0? ?? ??? ???? ?? & input param. @intCallerSP ?? at 2011-04-18
14 = ItemMerge? ItemLog ?? ??? ?? by ??? at 2012-02-24
15 = ItemMerge? ItemLog ??? ?? by ??? at 2012-08-24
**/

CREATE PROCEDURE [dbo].[P_MergeMaterializedItem]
	  @dt2MergeDate datetime2(0) --// ?? ??
	, @inbOldMaterializedItemID bigint --// ?? ???
	, @inbNewMaterializedItemID bigint --// ? ???
	, @intItemID int --// ? ???? ??? ID
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @intCallerSP int = 1 --// 1=P_AddMaterializedItem, 2=P_ChangeItemLocation, 3=P_ExchangeProperty, 4=P_MergeItemInGuildDepot, 5=P_MoveItemFromGuildDepot, 6=P_MoveItemIntoGuildDepot
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @insNewItemCount smallint
	, @insOldItemRemainCount smallint
	, @insNewItemRemainCount smallint
	, @inbOldOwnerCharacterID bigint
	, @dt2OldOwnershipStartDate datetime2(0)
	, @inbOldItemSerial bigint
	, @inbNewItemSerial bigint;

DECLARE @tblTemp1 table (OldItemSerial bigint NOT NULL PRIMARY KEY, ItemCount smallint NOT NULL, ItemPrice bigint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inbOldMaterializedItemID + @inbNewMaterializedItemID IS NULL
		RETURN 0;

	SELECT @insOldItemRemainCount = ItemRemainCount
		, @inbOldOwnerCharacterID = OwnerCharacterID
		, @dt2OldOwnershipStartDate = OwnershipStartDate
		, @inbOldItemSerial = ItemSerial
	FROM dbo.MaterializedItems WITH (NOLOCK)
	WHERE MaterializedItemID = @inbOldMaterializedItemID AND ItemID = @intItemID AND ItemLevel = 0;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103147;
		GOTO ErrorHandler;
	END

	UPDATE dbo.MaterializedItems
	SET @insNewItemCount = ItemRemainCount
		, @inbNewItemSerial = ItemSerial
		, @insNewItemRemainCount = ItemRemainCount += @insOldItemRemainCount
	WHERE MaterializedItemID = @inbNewMaterializedItemID AND ItemLevel = 0;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103147;
		GOTO ErrorHandler;
	END

	DELETE dbo.MaterializedItems
	WHERE MaterializedItemID = @inbOldMaterializedItemID;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2MergeDate
		, @inbItemSerial = @inbOldItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 33
		, @inbOwnerCharacterID = NULL
		, @inbOwnerCharacterIDBefore = @inbOldOwnerCharacterID
		, @insItemCount = 0
		, @insItemCountBefore = @insOldItemRemainCount
		, @inbItemLogFKey1 = @inbNewItemSerial
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = @dt2OldOwnershipStartDate
		, @intIP = NULL;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2MergeDate
		, @inbItemSerial = @inbNewItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 34
		, @inbOwnerCharacterID = @inbOldOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbOldOwnerCharacterID
		, @insItemCount = @insNewItemRemainCount
		, @insItemCountBefore = @insNewItemCount
		, @inbItemLogFKey1 = @inbOldItemSerial
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL;
	
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
