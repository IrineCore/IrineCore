USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UnlockItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-21
description : ?? ???? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103375 = ?? ??? ???? ??? ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_UnlockItem]
	  @inbOwnerCharacterID bigint --// ?? ??? ID
	, @inyItemLocationCode tinyint --// ??? ?? ?? (1=????, 7=??????)
	, @inyItemLocationIndex tinyint --// ??? ?? ???	
	, @intItemID int --// ???ID
	, @inbItemSerial bigint --// ??????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [LockedItems] ???? DELETE???.*/
	DELETE dbo.LockedItems 
	WHERE OwnerCharacterID = @inbOwnerCharacterID
	  AND ItemLocationCode = @inyItemLocationCode
      AND ItemLocationIndex = @inyItemLocationIndex;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103375;
		GOTO ErrorHandler;
	END	

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 72
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = NULL
		, @insItemCount = 1
		, @insItemCountBefore = 1
		, @inbItemLogFKey1 = @inyItemLocationCode
		, @inbItemLogFKey2 = @inyItemLocationIndex
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL
		, @inySealCount = NULL
		, @bitSoulBoundFlag = NULL
		, @inyItemOption = NULL
		, @inyItemPotential = NULL
		, @inyItemPotentialMoveCount = NULL;
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
