USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddDragonJewel]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-06
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103383 = ?????? ???? ????.
103384 = ?????? ??? ???? ???? ????.
103385 = ?? ??? ?? ??? ??????.
103386 = ?? ?? ???? ??? ??? ?????.
103389 = ?? ??? ???? ??????.
**/

CREATE PROCEDURE [dbo].[P_AddDragonJewel]	  
	  @inbItemSerial bigint --// ??????	
	, @inbParentItemSerial bigint --// ????? ???	
	, @inyDragonJewelLocationIndex tinyint --// ?? ?? ???	
	, @inyDragonJewelMaxCount tinyint --// ?????? ????? ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int	  
	  , @intItemID int
	  , @inbMaterializedItemID bigint
	  , @intParentItemID int
	  , @inbParentItemMaterializedItemID bigint
	  , @inyDragonJewelCount tinyint
	  , @inyDragonJewelType tinyint
	  , @inyItemPotential tinyint
	  , @dt2Now datetime2(0) = GETDATE()
	  , @inbOwnerCharacterID bigint
	  , @inySealCount tinyint
	  , @bitSoulBoundFlag bit
	  , @inyItemOption tinyint
	  , @inyItemPotentialMoveCount tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ???? ????? ?????.*/
	SELECT @inbMaterializedItemID = MaterializedItemID, @intItemID = ItemID, @inbOwnerCharacterID = OwnerCharacterID
		 , @inySealCount = SealCount, @bitSoulBoundFlag = SoulBoundFlag, @inyItemOption = ItemOption, @inyItemPotentialMoveCount = ItemPotentialMoveCount
	FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) 
	WHERE ItemSerial = @inbItemSerial;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103383;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ???? ????? ?????.*/
	SELECT @intParentItemID = ItemID, @inbParentItemMaterializedItemID = MaterializedItemID, @inyDragonJewelType = DragonJewelType
	FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) 
	WHERE ItemSerial = @inbParentItemSerial;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103384;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ??? ????? ?????.*/
	IF @inyDragonJewelType = 0 OR @inyDragonJewelType IS NULL
	BEGIN
		SET @intReturnValue = 103389;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ??? ????? ?????.*/
	SELECT @inyDragonJewelCount = DragonJewelCount
	FROM dbo.VIX_DragonJewelCount WITH (NOLOCK)
	WHERE ParentItemSerial = @inbParentItemSerial;
	
	IF @inyDragonJewelCount >= @inyDragonJewelMaxCount
	BEGIN
		SET @intReturnValue = 103385;
		GOTO ErrorHandler;
	END
	
	/**_# ?? ???? ItemLocationCode? ?? ??? ?????.*/
	UPDATE dbo.MaterializedItems 
	SET ItemLocationCode = 23, ItemLocationIndex = 0, OwnerCharacterID = NULL
	WHERE MaterializedItemID = @inbMaterializedItemID;

	/**_# ?? ???? Insert ???.*/
	INSERT dbo.DragonJewels (ItemID, ItemSerial, MaterializedItemID, ParentItemID, ParentItemSerial, ParentItemMaterializedItemID, DragonJewelLocationIndex, ItemOption)
	SELECT @intItemID, @inbItemSerial, @inbMaterializedItemID, @intParentItemID, @inbParentItemSerial, @inbParentItemMaterializedItemID, @inyDragonJewelLocationIndex, @inyItemOption;	

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 75
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = 1
		, @insItemCountBefore = 1
		, @inbItemLogFKey1 = @inbParentItemSerial
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount
		, @inyDragonJewelType = 0;
	
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_%'
		SET @intReturnValue = 103386;
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
