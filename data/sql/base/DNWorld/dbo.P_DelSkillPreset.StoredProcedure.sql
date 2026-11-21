USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelSkillPreset]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-08-30
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DelSkillPreset]
	  @inbCharacterID bigint --// ???ID
	, @inySkillPresetIndex tinyint --// ?? ??? Index
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

	DELETE dbo.PresetSkills
	FROM dbo.PresetSkills WITH (INDEX(PK_PresetSkills), FORCESEEK)
	WHERE CharacterID = @inbCharacterID AND SkillPresetIndex = @inySkillPresetIndex
	OPTION (KEEPFIXED PLAN);

	DELETE dbo.SkillPresets
	FROM dbo.SkillPresets WITH (INDEX(PK_SkillPresets), FORCESEEK)
	WHERE CharacterID = @inbCharacterID AND SkillPresetIndex = @inySkillPresetIndex
	OPTION (KEEPFIXED PLAN);

	COMMIT TRANSACTION;
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
