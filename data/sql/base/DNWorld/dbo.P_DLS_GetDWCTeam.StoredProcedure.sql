USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetDWCTeam]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-03-08
description : DWC ? ??? ?????.(???)
result set : DWCTeamName/nvarchar(50), DWCTeamID/int ,Win/int, Lose/int, Draw/int, DWCPoint/int, TodayWin/smallint, TodayLose/smallint, TodayDraw/smallint, ConsecutiveWin/smallint, ConsecutiveLose/smallint, WeeklyPlayCount/int, LastPlayDate/datetime2(0), DWCHiddenPoint/int, Ranking/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103361 = ???? ?? DWC????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetDWCTeam]
	  @intDWCTeamID int = NULL --// DWC ?ID
	, @nvcDWCTeamName nvarchar(50) = NULL --// DWC ???
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

	IF @intDWCTeamID > 0
		SELECT DWCTeamName, DWCTeamID, Win, Lose, Draw, DWCPoint
			 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN TodayWin ELSE 0 END AS TodayWin
			 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN TodayLose ELSE 0 END AS TodayLose
			 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN TodayDraw ELSE 0 END AS TodayDraw
			 , ConsecutiveWin, ConsecutiveLose, WeeklyPlayCount, LastPlayDate, DWCHiddenPoint
		FROM dbo.DWCTeams DT WITH(NOLOCK, INDEX(PK_DWCTeams), FORCESEEK)
		WHERE DWCTeamID = @intDWCTeamID
		OPTION(KEEPFIXED PLAN);
	ELSE IF @nvcDWCTeamName IS NOT NULL
		SELECT DWCTeamName, DWCTeamID, Win, Lose, Draw, DWCPoint
			 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN TodayWin ELSE 0 END AS TodayWin
			 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN TodayLose ELSE 0 END AS TodayLose
			 , CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN TodayDraw ELSE 0 END AS TodayDraw
			 , ConsecutiveWin, ConsecutiveLose, WeeklyPlayCount, LastPlayDate, DWCHiddenPoint
		FROM dbo.DWCTeams DT WITH(NOLOCK, INDEX(IX_UNF_DWCTeams_DWCTeamName), FORCESEEK)
		WHERE DWCTeamName = @nvcDWCTeamName
		  AND CloseFlag = 0
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
