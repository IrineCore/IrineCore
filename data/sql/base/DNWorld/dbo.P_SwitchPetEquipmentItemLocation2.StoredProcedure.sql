USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SwitchPetEquipmentItemLocation2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : "?? or ?"? ??? ???? ????? ?? ???? ?? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103178 = ? ???? ?? ??? ???? ????.
103183 = ??? ??????.
**/

CREATE PROCEDURE [dbo].[P_SwitchPetEquipmentItemLocation2]
	  @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// 2=????, 5=?? ????
	, @inbItemSerial bigint --// ????? ??? ???
	, @inbPetItemSerial bigint --// ???? ??? ?? or ?? ??? ???
	, @inyPetEquipmentIndex tinyint --// ???? ??? ??
	, @inbPetEquipmentItemSerial bigint --// ??? ???? ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbMaterializedItemID bigint, @inbPetEquipmentMaterializedItemID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inbItemSerial = @inbPetEquipmentItemSerial
	BEGIN
		SET @intReturnValue = 103183;
		GOTO ErrorHandler;
	END

	SELECT @inbPetEquipmentMaterializedItemID = MaterializedItemID
	FROM dbo.MaterializedItems WITH (NOLOCK)
	WHERE ItemSerial = @inbPetEquipmentItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemLocationCode = 13;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103178;
		GOTO ErrorHandler;
	END

	SELECT @inbMaterializedItemID = MaterializedItemID
	FROM dbo.MaterializedItems WITH (NOLOCK)
	WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemLocationCode IN (2, 5);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103178;
		GOTO ErrorHandler;
	END

	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = @inyItemLocationCode
	WHERE MaterializedItemID = @inbPetEquipmentMaterializedItemID;

	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = 13
	WHERE MaterializedItemID = @inbMaterializedItemID;

	UPDATE dbo.PetEquipments
	SET PetEquipmentItemSerial = @inbItemSerial
	WHERE PetItemSerial = @inbPetItemSerial AND PetEquipmentIndex = @inyPetEquipmentIndex;

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
