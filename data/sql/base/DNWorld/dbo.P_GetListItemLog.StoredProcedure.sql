USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListItemLog]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-01
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag ??? ??? ?????. at 2010-04-26
4 = ItemLevelBefore ??? ??? ?????. at 2010-07-06
7 = DragonJewelType ??? ??? ?????. at 2013-03-18
**/
CREATE PROCEDURE [dbo].[P_GetListItemLog]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int, @dt2LogDate datetime2(0) = CAST(CAST(@dtmBasisDate AS date) AS datetime2(0)), @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	IF @intLogTableSuffix = 0
		SELECT LogDate, @intWorldID AS WorldID, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemPotentialMoveCount, DragonJewelType
		FROM dbo.ItemLogs0 WITH(NOLOCK)
		WHERE LogDate >= @dt2LogDate AND LogDate < DATEADD(DAY, 1, @dt2LogDate)
		OPTION (MAXDOP 1);

	ELSE IF @intLogTableSuffix = 1
		SELECT LogDate, @intWorldID AS WorldID, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemPotentialMoveCount, DragonJewelType
		FROM dbo.ItemLogs1 WITH(NOLOCK)
		WHERE LogDate >= @dt2LogDate AND LogDate < DATEADD(DAY, 1, @dt2LogDate)
		OPTION (MAXDOP 1);

	ELSE
		SELECT LogDate, @intWorldID AS WorldID, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemPotentialMoveCount, DragonJewelType
		FROM dbo.ItemLogs2 WITH(NOLOCK)
		WHERE LogDate >= @dt2LogDate AND LogDate < DATEADD(DAY, 1, @dt2LogDate)
		OPTION (MAXDOP 1);

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
