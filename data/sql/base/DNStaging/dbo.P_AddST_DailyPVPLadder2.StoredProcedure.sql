USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyPVPLadder2]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-11
description : ?? PVP ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ???? by ??? at 2013-04-11
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyPVPLadder2]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
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
	SET @datBasisDate = CAST(ISNULL(@datBasisDate, GETDATE()) AS date);

	/**_# [ST_DailyPVP] ???? INSERT???.*/
	INSERT dbo.ST_DailyPVPLadder2 (BasisDate, WorldID, JobCode, CharacterLevel, Ladder0, Ladder9, Ladder10, Ladder11, LadderPoint1800, PVPLadderGradePoint_AVG)
	SELECT p.BasisDate, dc.WorldID, dc.JobCode, dc.CharacterLevel
		, SUM(CASE WHEN p.TodayWin + p.TodayLose + p.TodayDraw = 0 THEN 1 END) AS Ladder0
		, SUM(CASE WHEN p.TodayWin + p.TodayLose + p.TodayDraw >= 1 AND p.TodayWin + p.TodayLose + p.TodayDraw < 10 THEN 1 END) AS Ladder9
		, SUM(CASE WHEN p.TodayWin + p.TodayLose + p.TodayDraw = 10 THEN 1 END) AS Ladder10
		, SUM(CASE WHEN p.TodayWin + p.TodayLose + p.TodayDraw >= 11 THEN 1 END) AS Ladder11
		, SUM(CASE WHEN p.PVPLadderGradePoint >= 1800 THEN 1 END) AS LadderPoint1800
		, AVG(p.PVPLadderGradePoint) AS PVPLadderGradePoint_AVG
	FROM dbo.DailyPVPLadderScores p WITH (NOLOCK)
	INNER JOIN dbo.DimCharacters dc WITH (NOLOCK) ON dc.CharacterID = p.CharacterID
	WHERE p.BasisDate >= @datBasisDate and p.BasisDate < DATEADD(DD, 1, @datBasisDate)
	GROUP BY p.BasisDate, dc.WorldID, dc.JobCode, dc.CharacterLevel
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
