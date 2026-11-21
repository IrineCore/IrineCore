USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelPetEquipment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-16
description : ?? or ?? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
history :
2 = ??? ?? ?? at 2011-03-18
3 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_DelPetEquipment]
	  @inbPetItemSerial bigint --// ?? or ?? ??? ???
	, @inbPetEquipmentItemSerial bigint --// ?? or ?? ??? ???? ??? ???
	, @vchIP varchar(15) --// ????? IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2LogDate datetime2(0) = GETDATE()
	, @intItemID int
	, @insItemDurability smallint
	, @inyItemLevel tinyint
	, @inyItemPotential tinyint
	, @bitSoulBoundFlag bit
	, @inySealCount tinyint
	, @inyItemOption tinyint
	, @inbOwnerCharacterID bigint
	, @dt2OwnershipStartDate datetime2(0)
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PetEquipments] ????? DELETE???.*/
	DELETE dbo.PetEquipments
	WHERE PetItemSerial = @inbPetItemSerial AND PetEquipmentItemSerial = @inbPetEquipmentItemSerial;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	SELECT @intItemID = ItemID
		, @insItemDurability = ItemDurability
		, @inyItemLevel = ItemLevel
		, @inyItemPotential = ItemPotential
		, @bitSoulBoundFlag = SoulBoundFlag
		, @inySealCount = SealCount
		, @inyItemOption = ItemOption
		, @inbOwnerCharacterID = OwnerCharacterID
		, @dt2OwnershipStartDate = OwnershipStartDate
	FROM dbo.MaterializedItems WITH (NOLOCK)
	WHERE ItemSerial = @inbPetEquipmentItemSerial;

	DELETE dbo.MaterializedItems WHERE ItemSerial = @inbPetEquipmentItemSerial;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2LogDate
		, @inbItemSerial = @inbPetEquipmentItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 21
		, @inbOwnerCharacterID = NULL
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = 0
		, @insItemCountBefore = 1
		, @inbItemLogFKey1 = NULL
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @insItemDurability = @insItemDurability
		, @inyItemLevelBefore = @inyItemLevel
		, @inyItemLevel = @inyItemLevel;
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
