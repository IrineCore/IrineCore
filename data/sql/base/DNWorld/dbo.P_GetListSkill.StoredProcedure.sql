USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListSkill]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-19
description : ?? ??? ?????.
result set :
ver.1 = SkillID/int, SkillLevel/tinyint, CoolTime/int
ver.2 = SkillCode/tinyint, SkillID/int, SkillLevel/tinyint, CoolTime/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103146 = ??? ????.
history :
2 = input param. @intVersion ?? at 2011-04-06
**/

CREATE PROCEDURE [dbo].[P_GetListSkill]
	  @inbCharacterID bigint --// ??? ID
	, @intVersion int = 1 --// ??? SP ??
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
	IF @intVersion = 1
		SELECT SkillID, SkillLevel, CoolTime
		FROM dbo.Skills WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

	ELSE IF @intVersion = 2
		SELECT SkillCode, SkillID, SkillLevel, CoolTime
		FROM dbo.Skills WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103146;
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
