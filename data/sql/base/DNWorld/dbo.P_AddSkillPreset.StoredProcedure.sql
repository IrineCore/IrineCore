USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddSkillPreset]    Script Date: 11/14/2026 8:54:37 AM ******/
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
101210 = ?? ??? ??? ?? ???.
101211 = ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddSkillPreset]
	  @inbCharacterID bigint --// ???ID
	, @inySkillPresetIndex tinyint --// ?? ??? Index
	, @nvcSkillPresetName nvarchar(30) --// ?? ??? ??
	, @vchSkillIDs varchar(max) --// ,? ?????? ?? ID ???
	, @vchSkillLevels varchar(max) --// ,? ?????? ?? ?? ???
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

	/**_# [SkillPresets] ???? INSERT???.*/
	INSERT dbo.SkillPresets (CharacterID, SkillPresetIndex, SkillPresetName)
	VALUES (@inbCharacterID, @inySkillPresetIndex, @nvcSkillPresetName);

	/**_# [PresetSkills] ???? INSERT???.*/
	INSERT dbo.PresetSkills (CharacterID, SkillPresetIndex, SkillID, SkillLevel)
	SELECT @inbCharacterID, @inySkillPresetIndex, CAST(A.string AS int), CAST(B.string AS tinyint)
	FROM dbo.TF_Split(@vchSkillIDs, ',') A
		INNER JOIN dbo.TF_Split(@vchSkillLevels, ',') B ON A.seq = B.seq;

	COMMIT TRANSACTION;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_SkillPresets%'
		SET @intReturnValue = 101210;

	ELSE IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_PresetSkills%'
		SET @intReturnValue = 101211;

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
