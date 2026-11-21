USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_FirstUseCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 17
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-15
description : ?? ???? ???? ?? ?? ??? ?????. ?, Equip ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
12 = AchieveMissionBin, AppellationBin ?? ????? ???? ??? ?? at 2010-07-26
13 = CompleteQuestBin ?? ????? ???? ??? ?? at 2010-08-30
15 = PVPScore ? INSERT ?? ?? ?? ?? (P_ModPVPScore ?? UPDATE ?? Data? ???? ??? INSERT ??? ??) - 20110111.wgkim
16 = ??? ?? dbo.Skills.SkillCode ??? default 1? ????? ?? at 2011-04-06
17 = ?? ?? by ??? at 2012-03-07
**/

CREATE PROCEDURE [dbo].[P_FirstUseCharacter]
	  @inbCharacterID bigint --// ??? ID
	, @vchSkillIDs varchar(8000) --// ,? ?????? ?? ID ???
	, @vchSkillLevels varchar(8000) --// ,? ?????? ?? ?? ???
	, @vchQuickSlotObjectTypeCodes varchar(8000) --// ,? ?????? ? ??? ??? ???? ?? ???
	, @vchQuickSlotObjectValues varchar(8000) --// ,? ?????? ???? ??? ????? ID ???
	, @intNotifierIndexSize tinyint --// ??? ?? ???
	, @inyQuestIndexSize tinyint --// ??? ?? ???
-- WITH EXECUTE AS 'SPExecutor'
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
	/**_# [Skills]???? INSERT???.*/
	DELETE dbo.Skills WHERE CharacterID = @inbCharacterID;

	WITH ARR1 (Seq, String) AS (
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
	SELECT @inbCharacterID, C1.SkillID, C1.SkillLevel, 0, 0, C1.SkillLevel, 1
	FROM C1;

	/**_# [P_SetQuickSlot]? ???? ? ??? ??????.*/
	EXEC @intReturnValue = dbo.P_SetQuickSlot
		  @inbCharacterID = @inbCharacterID
		, @vchObjectTypeCodes = @vchQuickSlotObjectTypeCodes
		, @vchObjectValues = @vchQuickSlotObjectValues;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	/**_# [P_InitNotifier]? ???? ???? ??????.*/
	DELETE dbo.Notifiers WHERE CharacterID = @inbCharacterID;

	EXEC @intReturnValue = dbo.P_InitNotifier
		  @inbCharacterID = @inbCharacterID
		, @intNotifierIndexSize = @intNotifierIndexSize;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	/**_# [MissionBinaries]???? INSERT???.*/
	DELETE dbo.MissionBinaries WHERE CharacterID = @inbCharacterID;
	INSERT dbo.MissionBinaries (CharacterID, GainMissionBin)
	VALUES (@inbCharacterID, 0x0);
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
