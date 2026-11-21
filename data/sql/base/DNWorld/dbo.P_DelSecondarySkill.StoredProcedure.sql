USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelSecondarySkill]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-02
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103222 = ??? ?? ??? ????.
103229 = ?? ?? ???? ???? ????. ?? ??? ??? ? ????.
history :
2 = ?? ??? ???? ?? ?? ??, ?? ??? ??? ? ??? ?? at 2010-09-03
3 = IP ??? ?? ?? ??? ?? at 2010-09-03
**/

CREATE PROCEDURE [dbo].[P_DelSecondarySkill]
	  @inbCharacterID bigint --// ??? ID
	, @intSecondarySkillID int --// ?? ?? ID
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
	BEGIN TRANSACTION;

	/**_# [SecondarySkills] ????? DELETE ???.*/
	DELETE dbo.SecondarySkills
	WHERE CharacterID = @inbCharacterID AND SecondarySkillID = @intSecondarySkillID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103222;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_SecondarySkills_ManufactureSkillRecipes_CharacterID_SecondarySkillID%'
		SET @intReturnValue = 103229;

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
