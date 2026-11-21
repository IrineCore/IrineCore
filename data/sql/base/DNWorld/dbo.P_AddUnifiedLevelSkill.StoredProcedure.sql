USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddUnifiedLevelSkill]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-07-30
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103342 = ?? ?? ??? ??????, ?? ???? ??? ???? ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddUnifiedLevelSkill]
	  @inbCharacterID bigint --// ??? ID
	, @inyUnifiedLevelSkillIndex tinyint --// ?? ?? ?? ???
	, @intUnifiedLevelSkillID int --// ?? ?? ??ID
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
	/**_# ?? ??? ??? ???? ??? UPDATE ???.*/
	UPDATE dbo.UnifiedLevelSkills
	SET UnifiedLevelSkillID = @intUnifiedLevelSkillID
	WHERE CharacterID = @inbCharacterID
      AND UnifiedLevelSkillIndex = @inyUnifiedLevelSkillIndex;
	
	/**_# [UnifiedLevelSkills] ???? INSERT???.*/
	IF @@ROWCOUNT = 0	
		INSERT dbo.UnifiedLevelSkills (CharacterID, UnifiedLevelSkillIndex, UnifiedLevelSkillID)
		VALUES (@inbCharacterID, @inyUnifiedLevelSkillIndex, @intUnifiedLevelSkillID);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_UnifiedLevelSkills%'
		SET @intReturnValue = 103342;

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
