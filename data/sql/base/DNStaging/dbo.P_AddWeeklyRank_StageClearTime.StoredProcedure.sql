USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddWeeklyRank_StageClearTime]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-02-22
description : ?? ???? ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddWeeklyRank_StageClearTime]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int;
DECLARE @dt2StartDate datetime2(0) = CAST(CAST(DATEADD(DAY, - DATEPART(WEEKDAY, GETDATE() - 1), GETDATE()) AS date) AS datetime2(0))
      , @dt2EndDate datetime2(0) = CAST(CAST(GETDATE() AS date) AS datetime2(0))
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'StageEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	EXEC dbo.P_MakeTableString
	  @nvcDBName = @nvcDBName
	, @nvcTblName = @nvcTblName
	, @nvcStartDate = @nvcStartDate
	, @nvcEndDate = @nvcEndDate
	, @nvcHint = @nvcHint
	, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	

	TRUNCATE TABLE dbo.WeeklyRank_StageClearTime;
	SET @nvcStmt = N'
	WITH C0 AS (
		SELECT ROW_NUMBER() OVER(PARTITION BY V.CharacterID, DS.StageID ORDER BY V.ClearTime, V.StageStartDate) AS Seq, V.CharacterID, V.CharacterLevel, V.JobCode, DS.StageID, V.ClearTime, V.StageStartDate
		FROM (' + @nvcStmtOUT + N') V
			INNER JOIN dbo.DimMaps DM WITH (NOLOCK) ON V.MapID = DM.MapID
			INNER JOIN dbo.DimStages DS WITH (NOLOCK) ON DM.StageID = DS.StageID
		WHERE V.PartySize = 1 AND V.DifficultyCode = 5 AND V.ClearFlag = CAST(1 AS bit)
			AND V.CharacterLevel <= (DS.RecommendedCharacterLevel + 2)
	), C1 AS (
		SELECT C0.CharacterID, DC.WorldID, C0.CharacterLevel, C0.JobCode, C0.StageID, C0.ClearTime, C0.StageStartDate, DC.CharacterClassCode
		FROM C0
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON C0.CharacterID = DC.CharacterID
		WHERE C0.Seq = 1 AND DC.DeleteFlag = CAST(0 AS bit) AND NOT EXISTS (SELECT * FROM dbo.SS_RestraintedAccounts WHERE AccountID = DC.AccountID)
	), C2 AS (
		SELECT StageID, CharacterID, RANK() OVER(PARTITION BY StageID ORDER BY ClearTime, StageStartDate) AS TotalRank FROM C1
	), C3 AS (
		SELECT StageID, CharacterID, RANK() OVER(PARTITION BY StageID, WorldID ORDER BY ClearTime, StageStartDate) AS WorldRank FROM C1
	), C4 AS (
		SELECT StageID, CharacterID, RANK() OVER(PARTITION BY StageID, WorldID, CharacterClassCode ORDER BY ClearTime, StageStartDate) AS WorldCharacterClassRank FROM C1
	), C5 AS (
		SELECT StageID, CharacterID, RANK() OVER(PARTITION BY StageID, CharacterClassCode ORDER BY ClearTime, StageStartDate) AS CharacterClassRank FROM C1
	)
	SELECT C1.CharacterID, C1.WorldID, C1.CharacterLevel, C1.JobCode, C1.StageID, C1.ClearTime, C1.StageStartDate AS ClearDate, C2.TotalRank, C3.WorldRank, C4.WorldCharacterClassRank, C5.CharacterClassRank
	INTO #WeeklyRank_StageClearTime
	FROM C1
		INNER JOIN C2 ON C1.CharacterID = C2.CharacterID AND C1.StageID = C2.StageID
		INNER JOIN C3 ON C1.CharacterID = C3.CharacterID AND C1.StageID = C3.StageID
		INNER JOIN C4 ON C1.CharacterID = C4.CharacterID AND C1.StageID = C4.StageID
		INNER JOIN C5 ON C1.CharacterID = C5.CharacterID AND C1.StageID = C5.StageID;

	INSERT dbo.WeeklyRank_StageClearTime (CharacterID, WorldID, CharacterLevel, JobCode, StageID, ClearTime, ClearDate, TotalRank, WorldRank, WorldCharacterClassRank, CharacterClassRank)
	SELECT CharacterID, WorldID, CharacterLevel, JobCode, StageID, ClearTime, ClearDate, TotalRank, WorldRank, WorldCharacterClassRank, CharacterClassRank 
	FROM #WeeklyRank_StageClearTime

	IF DATEPART(WEEKDAY, GETDATE()) = 1
		WITH C0 AS (
			SELECT ROW_NUMBER() OVER(PARTITION BY StageID, WorldID ORDER BY WorldRank) AS Seq, CharacterID, StageID, WorldID, CharacterLevel, JobCode, ClearTime, ClearDate, TotalRank, WorldRank, WorldCharacterClassRank, CharacterClassRank
			FROM dbo.WeeklyRank_StageClearTime
		)
		INSERT dbo.PastWeeklyRank_StageClearTime (BasisDate, CharacterID, StageID, WorldID, CharacterLevel, JobCode, ClearTime, ClearDate, TotalRank, WorldRank, WorldCharacterClassRank, CharacterClassRank)
		SELECT CAST(@dt2StartDate AS date), CharacterID, StageID, WorldID, CharacterLevel, JobCode, ClearTime, ClearDate, TotalRank, WorldRank, WorldCharacterClassRank, CharacterClassRank
		FROM C0
		WHERE Seq <= 100
		ORDER BY CharacterID, StageID;'
	
	EXEC sp_executesql @nvcStmt, N'
		  @dt2StartDate datetime2(0)'
		, @dt2StartDate = @dt2StartDate
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
