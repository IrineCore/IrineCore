USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddSecondarySkill]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-02
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ????? ???? ????.
103221 = ?? ??? ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddSecondarySkill]
	  @inbCharacterID bigint --// ??? ID
	, @intSecondarySkillID int --// ?? ?? ID
	, @inySecondarySkillTypeCode tinyint --// 1=?? ??, 2=?? ??
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
	/**_# [SecondarySkills] ???? INSERT???.*/
	INSERT dbo.SecondarySkills (CharacterID, SecondarySkillID, SecondarySkillTypeCode, SecondarySkillExp)
	VALUES (@inbCharacterID, @intSecondarySkillID, @inySecondarySkillTypeCode, 0);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_SecondarySkills_CharacterID%'
		SET @intReturnValue = 103102;

	ELSE IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_SecondarySkills%'
		SET @intReturnValue = 103221;

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
