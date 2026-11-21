USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModSecondarySkillExp]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-02
description : ?? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103223 = ?? ??? ????.
103224 = ?? ?? ???? 0?? ??? ??? ???.
**/

CREATE PROCEDURE [dbo].[P_ModSecondarySkillExp]
	  @inbCharacterID bigint --// ??? ID
	, @intSecondarySkillID int --// ?? ?? ID
	, @intSecondarySkillExp int --// ?? ?? ???? ??? (+/- ?? ??)
	, @intSecondarySkillExpAfter int OUTPUT --// ?? ? ???
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
	UPDATE dbo.SecondarySkills
	SET @intSecondarySkillExpAfter = SecondarySkillExp += @intSecondarySkillExp
	WHERE CharacterID = @inbCharacterID AND SecondarySkillID = @intSecondarySkillID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103223;
		GOTO ErrorHandler;
	END
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_SecondarySkills_SecondarySkillExp%'
		SET @intReturnValue = 103224;

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
