USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UpsEquipmentAttribute]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-13
description : Equip ?? ??? UPSERT???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_UpsEquipmentAttribute]
	  @inbCharacterID bigint --// ??? ID
	, @inyEquipmentCode tinyint --// 0=Face, 1=Hair, 2=Helmet, 3=Body, 4=Leg, 5=Hand, 6=Foot, 7=Necklace, 8=Earring, 9=Ring1, 10=Ring2, 11=Crest, 12=Weapon1, 13=Weapon2
	, @inyEquipmentAttributeCode tinyint --// 1=??? ??, 2=Remain ??
	, @intAttributeValue int --// ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [EquipmentAttributes] ???? UPSERT???.*/
	/**_# ???? ???? ??? 103102? ?????.*/
	UPDATE dbo.EquipmentAttributes
	SET AttributeValue = @intAttributeValue
	WHERE CharacterID = @inbCharacterID AND EquipmentCode = @inyEquipmentCode AND EquipmentAttributeCode = @inyEquipmentAttributeCode;

	IF @@ROWCOUNT = 0
		INSERT dbo.EquipmentAttributes (CharacterID, EquipmentCode, EquipmentAttributeCode, AttributeValue)
		VALUES (@inbCharacterID, @inyEquipmentCode, @inyEquipmentAttributeCode, @intAttributeValue);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_EquipmentAttributes_CharacterID%'
		SET @intReturnValue = 103102;

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
