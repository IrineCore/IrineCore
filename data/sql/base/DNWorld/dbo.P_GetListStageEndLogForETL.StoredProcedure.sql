USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListStageEndLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-06
description : ???? ???? ?? ??? ?????. DNStaging ???
result set : StageStartDate, StageEndDate, StageStartID, CharacterID, JobCode, CharacterLevel, ClearFlag, ClearGradeCode, MaxComboCount, BossKillCount, RewardCharacterExp, BonusCharacterExp, ClearPoint, BonusClearPoint, ClearTime, RewardBoxCode, RewardItemSerial, RewardItemID, PartyStartID, PartySize, MapID, DifficultyCode, WorldID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
8 = ?? ?? RewardFailReason ?? at 2010-12-23
9 = Reward? ? ? ?? ?? ??? ???? Reward??? ?? ???? ???? ??	20110926_ohtack
10 = Reward? ? ? ?? ?? ??? ???? Reward??? ?? ???? ???? ??	20110926_ohtack
11 = ClearRound ?? ?? ??	20110928_ohtack
12 = TeacherPupilFlag ?? ?? ??	20130719_dailykim
13 = (101322) ??? ??? ?? ??? ???? ??? ???? ???? ??? ???? ????? ?? ??? by ??? at 2013-07-24
**/

CREATE PROCEDURE [dbo].[P_GetListStageEndLogForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dtmBasisDate);
DECLARE @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;
																																																																																																																																																																																											
	IF @intLogTableSuffix = 0
		SELECT L.StageStartDate, L.StageEndDate, EL.StageStartID, EL.CharacterID, EL.JobCode, EL.CharacterLevel, EL.ClearFlag, EL.ClearGradeCode, EL.MaxComboCount, EL.BossKillCount, EL.RewardCharacterExp, EL.BonusCharacterExp, EL.ClearPoint, EL.BonusClearPoint, EL.ClearTime, 0 AS RewardBoxCode, 0 AS RewardItemSerial, 0 AS RewardItemID, L.PartyStartID, L.PartySize, L.MapID, L.DifficultyCode, @intWorldID AS WorldID, 0 AS RewardFailReason, EL.ClearRound, EL.TeacherPupilFlag, EL.NestFatigue
		FROM dbo.StageStartIDs L WITH (NOLOCK)
			INNER JOIN dbo.StageEndLogs0 EL WITH (NOLOCK) ON L.StageStartID = EL.StageStartID
		WHERE L.StageStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND L.StageStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE IF @intLogTableSuffix = 1
		SELECT L.StageStartDate, L.StageEndDate, EL.StageStartID, EL.CharacterID, EL.JobCode, EL.CharacterLevel, EL.ClearFlag, EL.ClearGradeCode, EL.MaxComboCount, EL.BossKillCount, EL.RewardCharacterExp, EL.BonusCharacterExp, EL.ClearPoint, EL.BonusClearPoint, EL.ClearTime, 0 AS RewardBoxCode, 0 AS RewardItemSerial, 0 AS RewardItemID, L.PartyStartID, L.PartySize, L.MapID, L.DifficultyCode, @intWorldID AS WorldID, 0 AS RewardFailReason, EL.ClearRound, EL.TeacherPupilFlag, EL.NestFatigue
		FROM dbo.StageStartIDs L WITH (NOLOCK)
			INNER JOIN dbo.StageEndLogs1 EL WITH (NOLOCK) ON L.StageStartID = EL.StageStartID
		WHERE L.StageStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND L.StageStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE
		SELECT L.StageStartDate, L.StageEndDate, EL.StageStartID, EL.CharacterID, EL.JobCode, EL.CharacterLevel, EL.ClearFlag, EL.ClearGradeCode, EL.MaxComboCount, EL.BossKillCount, EL.RewardCharacterExp, EL.BonusCharacterExp, EL.ClearPoint, EL.BonusClearPoint, EL.ClearTime, 0 AS RewardBoxCode, 0 AS RewardItemSerial, 0 AS RewardItemID, L.PartyStartID, L.PartySize, L.MapID, L.DifficultyCode, @intWorldID AS WorldID, 0 AS RewardFailReason, EL.ClearRound, EL.TeacherPupilFlag, EL.NestFatigue
		FROM dbo.StageStartIDs L WITH (NOLOCK)
			INNER JOIN dbo.StageEndLogs2 EL WITH (NOLOCK) ON L.StageStartID = EL.StageStartID
		WHERE L.StageStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND L.StageStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

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
