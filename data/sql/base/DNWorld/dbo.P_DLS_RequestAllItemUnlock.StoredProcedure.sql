USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_RequestAllItemUnlock]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-20
description : ?? ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103375 = ?? ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DLS_RequestAllItemUnlock]
	  @inbOwnerCharacterID bigint --// ?? ??? ID		
	, @intItemLockTime int = 72 --// ??? ?? ??(????, ???? ?? ?? Default 72??)
	, @dt2UnlockRequestDate datetime2(0) = NULL OUTPUT --// ???? ????	
	, @dt2UnlockDate datetime2(0) = NULL OUTPUT --// ???? ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE(), @i int = 1, @j int;
DECLARE @inbItemSerial bigint, @inyItemLocationCode int, @inyItemLocationIndex int, @intItemID int;
DECLARE @tblUnlockItems table (
	Seq int NOT NULL IDENTITY(1,1),	
	ItemID int NOT NULL,
	ItemSerial bigint NOT NULL,
	ItemLocationCode tinyint NOT NULL,
	ItemLocationIndex tinyint NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [LockedItems] ???? UPDATE???.*/
	UPDATE dbo.LockedItems
	SET UnlockRequestDate = @dt2Now
	  , UnlockDate = DATEADD(HOUR, @intItemLockTime, @dt2Now)
	  , @dt2UnlockRequestDate = @dt2Now	  
	  , @dt2UnlockDate = DATEADD(HOUR, @intItemLockTime, @dt2Now)
	OUTPUT deleted.ItemSerial, deleted.ItemLocationCode, deleted.ItemLocationIndex, MI.ItemID
	INTO @tblUnlockItems (ItemSerial, ItemLocationCode, ItemLocationIndex, ItemID)	
	FROM dbo.LockedItems LI WITH (NOLOCK) INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON LI.ItemSerial = MI.ItemSerial
	WHERE LI.OwnerCharacterID = @inbOwnerCharacterID;

	SET @j = @@ROWCOUNT;

	IF @j = 0
	BEGIN
		SET @intReturnValue = 103375;
		GOTO ErrorHandler;
	END	

	WHILE @i <= @j
	BEGIN	
		SELECT @inbItemSerial = ItemSerial, @inyItemLocationCode = ItemLocationCode, @inyItemLocationIndex = ItemLocationIndex, @intItemID = ItemID
		FROM @tblUnlockItems
		WHERE Seq = @i;

		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 71
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

		SET @i = @i + 1;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
