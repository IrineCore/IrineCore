USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddLockItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 15
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-21
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
103374 = ?? ?? ??????.
history :
12 = UPSERT ?? ?? by ??? at 2013-02-14
13 = ??? ???? ?? ?? ?? ? ItemSerial ?? ???? by ??? at 2013-02-14
**/

CREATE PROCEDURE [dbo].[P_AddLockItem]
	  @inbOwnerCharacterID bigint --// ?? ??? ID
	, @inyItemLocationCode tinyint --// ??? ?? ?? (1=????, 7=??????)
	, @inyItemLocationIndex tinyint --// ??? ?? ???	
	, @intItemID int --// ?? ??? ID
	, @inbItemSerial bigint --// ?? ??? ???
	, @dt2LockedDate datetime2(0) = NULL OUTPUT --// ??? ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @inySealCount tinyint
	  , @bitSoulBoundFlag bit
	  , @inyItemOption tinyint
	  , @inyItemPotential tinyint
	  , @inyItemPotentialMoveCount tinyint
	  , @inyDragonJewelType tinyint;	  

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ??? ?? ??? ?????.*/
	SELECT @inySealCount = SealCount
	     , @bitSoulBoundFlag = SoulBoundFlag
	     , @inyItemOption = ItemOption
	     , @inyItemPotential = ItemPotential
	     , @inyItemPotentialMoveCount = ItemPotentialMoveCount
		 , @inyDragonJewelType = DragonJewelType
	FROM dbo.MaterializedItems WITH(NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) 
	WHERE ItemSerial = @inbItemSerial
      AND ItemID = @intItemID
      AND OwnerCharacterID = @inbOwnerCharacterID
      AND ItemLocationCode = @inyItemLocationCode
      AND ItemLocationIndex = @inyItemLocationIndex

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	/**_# [LockedItems] ???? UPSERT???.*/
	UPDATE dbo.LockedItems
	SET UnlockRequestDate = NULL, LockedDate = @dt2Now, UnlockDate = NULL
	WHERE OwnerCharacterID = @inbOwnerCharacterID 
	  AND ItemLocationCode = @inyItemLocationCode 
	  AND ItemLocationIndex = @inyItemLocationIndex;
	
	IF @@ROWCOUNT = 0		
		INSERT dbo.LockedItems (OwnerCharacterID, ItemLocationCode, ItemLocationIndex, LockedDate, ItemSerial)
		VALUES (@inbOwnerCharacterID, @inyItemLocationCode, @inyItemLocationIndex, @dt2Now, @inbItemSerial);

	SET @dt2LockedDate = @dt2Now;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 70
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = NULL
		, @insItemCount = 1
		, @insItemCountBefore = 1
		, @inbItemLogFKey1 = @inyItemLocationCode
		, @inbItemLogFKey2 = @inyItemLocationIndex
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount
		, @inyDragonJewelType = @inyDragonJewelType; 

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_%'
		SET @intReturnValue = 103374;	
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
