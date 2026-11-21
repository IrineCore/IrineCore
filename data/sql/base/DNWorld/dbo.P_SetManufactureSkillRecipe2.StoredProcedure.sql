USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SetManufactureSkillRecipe2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ?? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103225 = ??? ?? ?? ???? ???? ????.
103226 = ?? ??? ???? ?????. ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_SetManufactureSkillRecipe2]
	  @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// 2=????, 5=?? ????
	, @inbItemSerial bigint --// ??? ???? ??? ????
	, @intSecondarySkillID int --// ???? ?? ?? ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbMaterializedItemID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# [ManufactureSkillRecipes] ???? INSERT???.*/
	INSERT dbo.ManufactureSkillRecipes (CharacterID, SecondarySkillID, ItemSerial)
	VALUES (@inbCharacterID, @intSecondarySkillID, @inbItemSerial);

	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = 14
	WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103225;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_SecondarySkills_ManufactureSkillRecipes_CharacterID_SecondarySkillID%'
		SET @intReturnValue = 103226;

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
