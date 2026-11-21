USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPLadderScore]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-07
description : PVP ?? ??? ?????.
result set : PVPLadderCode/tinyint, PVPLadderGradePoint/int, HiddenPVPLadderGradePoint/int, Win/int, Lose/int, Draw/int, TodayWin/smallint, TodayLose/smallint, TodayDraw/smallint, ConsecutiveWin/smallint, ConsecutiveLose/smallint, LastPVPLadderScoreDate/datetime2, WeeklyPVPCount/smallint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ?? - LastPVPLadderScoreDate - ?? at 2011-02-10
3 = PVPLadderPoint ??? CharacterStatus ???? ?? at 2011-02-10
4 = ?? ??? ??? ?? ?? ?? at 2011-02-11
5 = ?? ?? WeeklyPVPCount ?? at 2011-02-16
6 = output param. @insWeeklyPVPCount ?? at 2011-02-16
7 = @MaxKillJobCode, @MaxDeathJobCode ?? ?? ?? (?? ?, ?? -> ??, ???) at 2011-04-04
8 = version 6?? ?? at 2011-05-12
9 = MaxKill??? MaxDeath?? ?? ?? ?? by ??? at 2011-10-24
**/

CREATE PROCEDURE [dbo].[P_GetListPVPLadderScore]
	  @inbCharacterID bigint --// ??? ID
	, @intPVPLadderPoint int OUTPUT --// PVP ?? ???
	, @MaxKillJobCode tinyint OUTPUT --// ?? ?? Kill Rate ??
	, @MaxDeathJobCode tinyint OUTPUT --// ?? ?? Death Rate ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intPVPLadderPoint = ISNULL(PVPLadderPoint, 0)
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	SELECT PVPLadderCode, PVPLadderGradePoint, HiddenPVPLadderGradePoint, Win, Lose, Draw
		, CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN TodayWin ELSE 0 END AS TodayWin
		, CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN TodayLose ELSE 0 END AS TodayLose
		, CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN TodayDraw ELSE 0 END AS TodayDraw
		, ConsecutiveWin, ConsecutiveLose, LastPVPLadderScoreDate, WeeklyPVPCount
	FROM dbo.PVPLadderScores WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID
	ORDER BY PVPLadderCode;

	SELECT TOP (1) @MaxKillJobCode = JobCode
	FROM dbo.PVPLadderKillDeathByJob WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID AND KOWin > 0
	ORDER BY KOWin DESC;

	IF @@ROWCOUNT = 0
		SET @MaxKillJobCode = 0;

	SELECT TOP (1) @MaxDeathJobCode = JobCode
	FROM dbo.PVPLadderKillDeathByJob WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID AND KOLose > 0
	ORDER BY KOLose DESC;

	IF @@ROWCOUNT = 0
		SET @MaxDeathJobCode = 0;

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
