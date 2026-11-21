USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDWCTeam]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-10
description : DWC ? ??? ?????.
result set : DWCTeamName/nvarchar(50), Win/int, Lose/int, Draw/int, DWCPoint/int, TodayWin/smallint, TodayLose/smallint, TodayDraw/smallint, ConsecutiveWin/smallint, ConsecutiveLose/smallint, WeeklyPlayCount/int, LastPlayDate/datetime2(0), DWCHiddenPoint/int, Ranking/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103361 = ???? ?? DWC????.
**/

CREATE PROCEDURE [dbo].[P_GetDWCTeam]
	  @intDWCTeamID int --// DWC ?ID
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

	SELECT DT.DWCTeamName, DT.Win, DT.Lose, DT.Draw, DT.DWCPoint
		 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN DT.TodayWin ELSE 0 END AS TodayWin
		 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN DT.TodayLose ELSE 0 END AS TodayLose
		 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN DT.TodayDraw ELSE 0 END AS TodayDraw
		 , DT.ConsecutiveWin, DT.ConsecutiveLose, DT.WeeklyPlayCount, DT.LastPlayDate, DT.DWCHiddenPoint
		 , ISNULL(DR.Ranking, 0) AS Ranking
	FROM dbo.DWCTeams DT WITH(NOLOCK, INDEX(PK_DWCTeams), FORCESEEK)
		LEFT OUTER JOIN dbo.DWCRanking DR WITH(NOLOCK, INDEX(IX_UN_DWCRanking_DWCTeamID), FORCESEEK) ON DT.DWCTeamID = DR.DWCTeamID
	WHERE DT.DWCTeamID = @intDWCTeamID
	  AND DT.CloseFlag = CAST(0 AS bit)
	OPTION(KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103361;
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
