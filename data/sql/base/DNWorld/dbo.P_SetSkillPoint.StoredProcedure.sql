USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SetSkillPoint]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-06-11
description : ?????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_SetSkillPoint]
	  @inbCharacterID bigint --// ??? ID
	, @insSkillPoint smallint --// ?? ???
	, @inySkillCode tinyint = 1 --// 1=1? ??, 2=2? ??
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
	/**_# [CharacterStatus]???? UPDATE???.*/
	IF @inySkillCode = 1
		UPDATE dbo.CharacterStatus
		SET SkillPoint = @insSkillPoint
		WHERE CharacterID = @inbCharacterID;

	ELSE IF @inySkillCode = 2
		UPDATE dbo.CharacterStatus
		SET SkillPoint2 = @insSkillPoint
		WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END
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
