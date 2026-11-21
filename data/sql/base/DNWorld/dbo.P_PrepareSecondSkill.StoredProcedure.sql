USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_PrepareSecondSkill]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-10-10
description : 2? ?? ?? - ?? ?? ? 2? ?? ??? ??
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_PrepareSecondSkill]
	  @inbCharacterID bigint --// ??? ID
	, @vchSkillIDs varchar(8000) --// ,? ?????? ?? ID ???
	, @vchSkillLevels varchar(8000) --// ,? ?????? ?? ?? ???
	, @insSkillPoint2 smallint --// ??? 2? ?? ???
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
	;WITH ARR1 (Seq, String) AS (
		SELECT ROW_NUMBER() OVER(ORDER BY n), SUBSTRING(',' + @vchSkillIDs + ',', n + 1, CHARINDEX(',', ',' + @vchSkillIDs + ',', n + 1) - n - 1)
		FROM dbo.Tally WITH (NOLOCK)
		WHERE n < DATALENGTH(',' + @vchSkillIDs + ',') AND SUBSTRING(',' + @vchSkillIDs + ',', n, 1) = ','
	), ARR2 (Seq, String) AS (
		SELECT ROW_NUMBER() OVER(ORDER BY n), SUBSTRING(',' + @vchSkillLevels + ',', n + 1, CHARINDEX(',', ',' + @vchSkillLevels + ',', n + 1) - n - 1)
		FROM dbo.Tally WITH (NOLOCK)
		WHERE n < DATALENGTH(',' + @vchSkillLevels + ',') AND SUBSTRING(',' + @vchSkillLevels + ',', n, 1) = ','
	), C1 (SkillID, SkillLevel) AS (
		SELECT CAST(ARR1.String AS int), CAST(ARR2.String AS tinyint)
		FROM ARR1
			INNER JOIN ARR2 ON ARR1.Seq = ARR2.Seq
		WHERE ARR1.String > '' AND ARR2.String > ''
	)
	INSERT dbo.Skills (CharacterID, SkillID, SkillLevel, CoolTime, UsedSkillPoint, DefaultSkillLevel, SkillCode)
	SELECT @inbCharacterID, C1.SkillID, C1.SkillLevel, 0, 0, C1.SkillLevel, 2
	FROM C1
	OPTION (KEEPFIXED PLAN);

	UPDATE dbo.CharacterStatus
	SET SkillPoint2 = @insSkillPoint2
	FROM dbo.CharacterStatus WITH (INDEX(PK_CharacterStatus), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);
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
