USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelDragonJewel]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-06
description : ??? ?????.
history : 
	1 ~ 7 : ?? ?? by ??? 
	8 : ?? ?? ??? ?? by ??? at 2013-04-26
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103383 = ?????? ???? ????.
103387 = ??? ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DelDragonJewel]
	  @inbParentItemSerial bigint --// ????????	
	, @inyDragonJewelLocationIndex tinyint --// ?? ?? ???	
	, @inbOwnerCharacterID bigint --// ????? ???	
	, @intItemID int OUTPUT --// ??? ID
	, @insItemCount smallint OUTPUT --// ??? ??
	, @insItemDurability smallint OUTPUT --// ??? ???
	, @intRandomSeed int OUTPUT --// ?? ??
	, @inyItemLevel tinyint OUTPUT --// ??? ??
	, @inyItemPotential tinyint OUTPUT --// ??? ???
	, @inyItemOption tinyint OUTPUT --// ??? ??
	, @inySealCount tinyint	 OUTPUT --// ?? ??		
	, @inyItemPotentialMoveCount tinyint = NULL OUTPUT --// ???? ??
	, @inyDragonJewelType tinyint = NULL OUTPUT --// ?? ?? ??
	, @bitSoulBoundFlag bit = NULL OUTPUT --// ????
	, @inbItemSerial bigint = NULL OUTPUT --// ??? ???
	, @inbCommission bigint = 0 --// ?? ?? ???
	, @inbCurrentCoin bigint = NULL OUTPUT --// ?? ?? ?? ? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inbMaterializedItemID bigint
	  , @inbParentItemMaterializedItemID bigint
	  , @intDragonJewelID int
	  , @dt2Now datetime2(0) = GETDATE()	  
	  , @inbDragonJewelSerial bigint
	  , @inbChangedCoin bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ???? ?? ??? ????? ?????.*/
	SELECT @intDragonJewelID = DragonJewelID,  @inbMaterializedItemID = MaterializedItemID, @inbDragonJewelSerial = ItemSerial
	FROM dbo.DragonJewels WITH (NOLOCK, INDEX(IX_UNI_DragonJewels_ParentItemSerial_DragonJewelLocationIndex), FORCESEEK) 
	WHERE ParentItemSerial = @inbParentItemSerial AND DragonJewelLocationIndex = @inyDragonJewelLocationIndex;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103383;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ???? ????? ?????.*/
	SELECT @inbParentItemMaterializedItemID = ParentItemMaterializedItemID
	FROM dbo.DragonJewels WITH (NOLOCK, INDEX(IX_UNI_DragonJewels_ParentItemSerial_DragonJewelLocationIndex), FORCESEEK) 
	WHERE ParentItemSerial = @inbParentItemSerial;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103387;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;

	/**_# [MaterializedItems] ????? UPDATE ???.*/
	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = 2
	  , OwnerCharacterID = @inbOwnerCharacterID
	  , @intItemID = ItemID
	  , @insItemCount = ItemRemainCount
	  , @insItemDurability = ItemDurability
	  , @intRandomSeed = RandomSeed
	  , @inyItemLevel = ItemLevel
	  , @inyItemPotential = ItemPotential
	  , @inyItemOption = ItemOption
	  , @inySealCount = SealCount
	  , @inyItemPotentialMoveCount = ItemPotentialMoveCount
	  , @inyDragonJewelType = DragonJewelType
	  , @bitSoulBoundFlag = SoulBoundFlag
	  , @inbItemSerial = ItemSerial
	WHERE MaterializedItemID = @inbMaterializedItemID;

	/**_# [DragonJewels] ????? DELETE ???.*/
	DELETE dbo.DragonJewels WHERE DragonJewelID = @intDragonJewelID;

	/**_# ???? ?????.*/
	UPDATE dbo.CharacterStatus
	SET Coin = Coin - @inbCommission
	  , @inbCurrentCoin = Coin - @inbCommission
	WHERE CharacterID = @inbOwnerCharacterID;

	SET @inbChangedCoin = @inbCommission * -1;

	COMMIT TRANSACTION;	

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbDragonJewelSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 76
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

	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dt2Now
		, @inbCharacterID = @inbOwnerCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = 60
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @inbChangedCoin
		, @intChannelID = NULL
		, @intMapID = NULL
		, @inbCoinChangeFKey = NULL;
	
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
