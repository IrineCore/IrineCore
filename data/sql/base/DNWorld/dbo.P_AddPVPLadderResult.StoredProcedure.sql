USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPLadderResult]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-07
description : PVP ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = PVP ?? ?? ?? ?? at 2011-02-15
4 = WeeklyPVPCount ?? ?? at 2011-02-16
5 = JobCode ?? ?? at 2011-02-17
6 = @vchKillDeathCounts ????? JobCode ?? 0?? ????? ?? at 2011-03-07
7 = ???? ?? WeeklyPVPCount? ????? ??? ?? at 2011-03-10
8 = @inyVersusCharacterJobCode ?? 1 ??? ??? PVPLadderScoresByJob ???? ???? ??? ?? at 2011-03-11
9 = TodayWin, TodayLose, TodayDraw ?? ?? ?? at 2011-04-04
**/
CREATE PROCEDURE [dbo].[P_AddPVPLadderResult]
	  @inbCharacterID bigint --// ??? ID
	, @inyPVPLadderCode tinyint --// 1=1:1, 2=2:2, 3=3:3, 4=4:4
	, @intPVPLadderGradePoint int --// PVP ?? ??
	, @intHiddenPVPLadderGradePoint int --// PVP ?? ???
	, @inyResult tinyint --// 1=??, 2=??, 3=???
	, @vchKillDeathCounts varchar(8000) --// ?? ?? ? Kill ?? ???. (JobCode1,JobCode2,JobCode3,...|vsJobCode1KillCount,vsJobCode2KillCount,vsJobCode3KillCount,...|vsJobCode1DeathCount,vsJobCode2DeathCount,vsJobCode3DeathCount,...)
	, @inyVersusCharacterJobCode tinyint = NULL --// 1:1??? ?? ?? ???? ??. 1=???, 2=??, 3=????, 4=???, 10=???, 11=?????, 12=????, 13=???, 14=?????, 15=????, 16=??, 17=??????, 18=????, 19=??, 20=???, 21=??, 22=????, 23=??????, 24=???, 25=????, 26=??????, 27=?????, 28=?????, 29=????, 30=????, 31=????, 32=????, 33=???, 34=?????, 35=????, 36=?????, 37=???, 38=????, 39=?????, 40=??????, 41=???, 42=?????, 43=???, 44=????, 45=?????, 46=???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE(), @vchJobTemp varchar(4000), @vchKillTemp varchar(4000), @vchDeathTemp varchar(4000);
DECLARE @tblKillDeathCount table (JobCode tinyint NOT NULL PRIMARY KEY, KillCount int NOT NULL, DeathCount int NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PVPLadderScores] ???? UPSERT ???.*/
	UPDATE dbo.PVPLadderScores
	SET PVPLadderGradePoint = @intPVPLadderGradePoint
		, HiddenPVPLadderGradePoint = @intHiddenPVPLadderGradePoint
		, Win += CASE @inyResult WHEN 1 THEN 1 ELSE 0 END
		, Lose += CASE @inyResult WHEN 2 THEN 1 ELSE 0 END
		, Draw += CASE @inyResult WHEN 3 THEN 1 ELSE 0 END
		, TodayWin = CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN CASE @inyResult WHEN 1 THEN TodayWin + 1 ELSE TodayWin END ELSE CASE @inyResult WHEN 1 THEN 1 ELSE 0 END END
		, TodayLose = CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN CASE @inyResult WHEN 2 THEN TodayLose + 1 ELSE TodayLose END ELSE CASE @inyResult WHEN 2 THEN 1 ELSE 0 END END
		, TodayDraw = CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN CASE @inyResult WHEN 3 THEN TodayDraw + 1 ELSE TodayDraw END ELSE CASE @inyResult WHEN 3 THEN 1 ELSE 0 END END
		, ConsecutiveWin += CASE @inyResult WHEN 1 THEN 1 ELSE - ConsecutiveWin END
		, ConsecutiveLose += CASE @inyResult WHEN 2 THEN 1 ELSE - ConsecutiveLose END
		, LastPVPLadderScoreDate = @dt2Now
		, WeeklyPVPCount += CASE WHEN @inyResult IN (1, 2) THEN 1 ELSE 0 END
	WHERE CharacterID = @inbCharacterID AND PVPLadderCode = @inyPVPLadderCode;

	IF @@ROWCOUNT = 0
		INSERT dbo.PVPLadderScores (CharacterID, PVPLadderCode, PVPLadderGradePoint, HiddenPVPLadderGradePoint, Win, Lose, Draw, TodayWin, TodayLose, TodayDraw, ConsecutiveWin, ConsecutiveLose, LastPVPLadderScoreDate, WeeklyPVPCount)
		VALUES (@inbCharacterID, @inyPVPLadderCode, @intPVPLadderGradePoint, @intHiddenPVPLadderGradePoint, CASE @inyResult WHEN 1 THEN 1 ELSE 0 END, CASE @inyResult WHEN 2 THEN 1 ELSE 0 END, CASE @inyResult WHEN 3 THEN 1 ELSE 0 END, CASE @inyResult WHEN 1 THEN 1 ELSE 0 END, CASE @inyResult WHEN 2 THEN 1 ELSE 0 END, CASE @inyResult WHEN 3 THEN 1 ELSE 0 END, CASE @inyResult WHEN 1 THEN 1 ELSE 0 END, CASE @inyResult WHEN 2 THEN 1 ELSE 0 END, @dt2Now, CASE WHEN @inyResult IN (1, 2) THEN 1 ELSE 0 END);

	/**_# ?? ???? ?? ? Kill / Death ??? ?????.*/
	SELECT @vchJobTemp = MAX(CASE seq WHEN 1 THEN string END)
		, @vchKillTemp = MAX(CASE seq WHEN 2 THEN string END)
		, @vchDeathTemp = MAX(CASE seq WHEN 3 THEN string END)
	FROM dbo.TF_Split(@vchKillDeathCounts, '|')
	OPTION (KEEPFIXED PLAN);

	INSERT @tblKillDeathCount (JobCode, KillCount, DeathCount)
	SELECT A.string, B.string, C.string
	FROM dbo.TF_Split(@vchJobTemp, ',') A
		INNER JOIN dbo.TF_Split(@vchKillTemp, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchDeathTemp, ',') C ON A.seq = C.seq
	WHERE A.string > '0' AND (B.string > '0' OR C.string > '0')
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

	WITH T AS (
		SELECT CharacterID, JobCode, KOWin, KOLose
		FROM dbo.PVPLadderKillDeathByJob WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID
	), S AS (
		SELECT @inbCharacterID AS CharacterID, JobCode, KillCount, DeathCount
		FROM @tblKillDeathCount
	)
	MERGE T
	USING S
	ON T.CharacterID = S.CharacterID AND T.JobCode = S.JobCode
	WHEN MATCHED THEN
		UPDATE SET KOWin += S.KillCount, KOLose += S.DeathCount
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (CharacterID, JobCode, KOWin, KOLose) VALUES (S.CharacterID, S.JobCode, S.KillCount, S.DeathCount);

	/**_# 1:1 ??? ?? ?? ???? ?? ? ?? / ?? / ???? ?????.*/
	IF @inyPVPLadderCode = 1 AND @inyVersusCharacterJobCode >= 1
	BEGIN
		UPDATE dbo.PVPLadderScoresByJob
		SET Win += CASE @inyResult WHEN 1 THEN 1 ELSE 0 END
			, Lose += CASE @inyResult WHEN 2 THEN 1 ELSE 0 END
			, Draw += CASE @inyResult WHEN 3 THEN 1 ELSE 0 END
		WHERE CharacterID = @inbCharacterID AND JobCode = @inyVersusCharacterJobCode;

		IF @@ROWCOUNT = 0
			INSERT dbo.PVPLadderScoresByJob (CharacterID, JobCode, Win, Lose, Draw)
			VALUES (@inbCharacterID, @inyVersusCharacterJobCode, CASE @inyResult WHEN 1 THEN 1 ELSE 0 END, CASE @inyResult WHEN 2 THEN 1 ELSE 0 END, CASE @inyResult WHEN 3 THEN 1 ELSE 0 END);
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
