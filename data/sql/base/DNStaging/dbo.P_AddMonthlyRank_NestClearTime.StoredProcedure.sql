USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddMonthlyRank_NestClearTime]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-12-17
description : ?? ??? ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddMonthlyRank_NestClearTime]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2StartDate datetime2(0) = DATEADD(MONTH, DATEDIFF(MONTH, 0, GETDATE() - 1), 0);
DECLARE @dt2EndDate datetime2(0) = DATEADD(MONTH, 1, @dt2StartDate);
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
	
	TRUNCATE TABLE dbo.MonthlyRank_NestClearTime;
	SET @nvcStmt = N'
	WITH C1 AS
	(
		SELECT ROW_NUMBER() OVER(PARTITION BY StageStartID ORDER BY CharacterID) AS Seq, V.StageStartID, DS.StageID, V.CharacterID, V.CharacterLevel, V.JobCode, V.PartySize, V.ClearTime
		FROM (' + @nvcStmtOUT + N') V
			INNER JOIN dbo.DimMaps DM WITH(NOLOCK) ON V.MapID = DM.MapID
			INNER JOIN dbo.DimStages DS WITH(NOLOCK) ON DM.StageID = DS.StageID
		WHERE V.ClearFlag = CAST(1 AS bit) 
		  AND DS.NestFlag = CAST(1 AS bit)  
	), 
	C2 AS(
	SELECT A.StageStartID, StageID, ClearTime
		, MAX(CASE WHEN Seq = 1 THEN CharacterID END) AS CharacterID1
		, MAX(CASE WHEN Seq = 1 THEN CharacterLevel END) AS CharacterLevel1
		, MAX(CASE WHEN Seq = 1 THEN JobCode END) AS JobCode1
		, MAX(CASE WHEN Seq = 2 THEN CharacterID END) AS CharacterID2
		, MAX(CASE WHEN Seq = 2 THEN CharacterLevel END) AS CharacterLevel2
		, MAX(CASE WHEN Seq = 2 THEN JobCode END) AS JobCode2
		, MAX(CASE WHEN Seq = 3 THEN CharacterID END) AS CharacterID3
		, MAX(CASE WHEN Seq = 3 THEN CharacterLevel END) AS CharacterLevel3
		, MAX(CASE WHEN Seq = 3 THEN JobCode END) AS JobCode3
		, MAX(CASE WHEN Seq = 4 THEN CharacterID END) AS CharacterID4
		, MAX(CASE WHEN Seq = 4 THEN CharacterLevel END) AS CharacterLevel4
		, MAX(CASE WHEN Seq = 4 THEN JobCode END) AS JobCode4
		, MAX(CASE WHEN Seq = 5 THEN CharacterID END) AS CharacterID5
		, MAX(CASE WHEN Seq = 5 THEN CharacterLevel END) AS CharacterLevel5
		, MAX(CASE WHEN Seq = 5 THEN JobCode END) AS JobCode5
		, MAX(CASE WHEN Seq = 6 THEN CharacterID END) AS CharacterID6
		, MAX(CASE WHEN Seq = 6 THEN CharacterLevel END) AS CharacterLevel6
		, MAX(CASE WHEN Seq = 6 THEN JobCode END) AS JobCode6
		, MAX(CASE WHEN Seq = 7 THEN CharacterID END) AS CharacterID7
		, MAX(CASE WHEN Seq = 7 THEN CharacterLevel END) AS CharacterLevel7
		, MAX(CASE WHEN Seq = 7 THEN JobCode END) AS JobCode7
		, MAX(CASE WHEN Seq = 8 THEN CharacterID END) AS CharacterID8
		, MAX(CASE WHEN Seq = 8 THEN CharacterLevel END) AS CharacterLevel8
		, MAX(CASE WHEN Seq = 8 THEN JobCode END) AS JobCode8
	FROM C1 A
	GROUP BY A.StageStartID, StageID, ClearTime
	)
	INSERT dbo.MonthlyRank_NestClearTime(StageID, ClearTime, CharacterID1, CharacterLevel1, JobCode1, CharacterID2, CharacterLevel2, JobCode2, CharacterID3, CharacterLevel3, JobCode3, CharacterID4, CharacterLevel4, JobCode4, CharacterID5, CharacterLevel5, JobCode5, CharacterID6, CharacterLevel6, JobCode6, CharacterID7, CharacterLevel7, JobCode7, CharacterID8, CharacterLevel8, JobCode8, TotalRanking)
	SELECT StageID
		, ClearTime
		, CharacterID1
		, CharacterLevel1
		, JobCode1
		, CharacterID2
		, CharacterLevel2
		, JobCode2
		, CharacterID3
		, CharacterLevel3
		, JobCode3
		, CharacterID4
		, CharacterLevel4
		, JobCode4
		, CharacterID5
		, CharacterLevel5
		, JobCode5
		, CharacterID6
		, CharacterLevel6
		, JobCode6
		, CharacterID7
		, CharacterLevel7
		, JobCode7
		, CharacterID8
		, CharacterLevel8
		, JobCode8
		, ROW_NUMBER() OVER(PARTITION BY StageID ORDER BY ClearTime, StageStartID) AS TotalRanking
	FROM C2
	ORDER BY TotalRanking;

	IF DATEPART(DAY, GETDATE()) = 1
		INSERT dbo.PastMonthlyRank_NestClearTime (BasisDate, StageID, ClearTime, CharacterID1, CharacterLevel1, JobCode1, CharacterID2, CharacterLevel2, JobCode2, CharacterID3, CharacterLevel3, JobCode3, CharacterID4, CharacterLevel4, JobCode4, CharacterID5, CharacterLevel5, JobCode5, CharacterID6, CharacterLevel6, JobCode6, CharacterID7, CharacterLevel7, JobCode7, CharacterID8, CharacterLevel8, JobCode8, TotalRanking)
		SELECT CAST(@dt2StartDate AS date), StageID, ClearTime, CharacterID1, CharacterLevel1, JobCode1, CharacterID2, CharacterLevel2, JobCode2, CharacterID3, CharacterLevel3, JobCode3, CharacterID4, CharacterLevel4, JobCode4, CharacterID5, CharacterLevel5, JobCode5, CharacterID6, CharacterLevel6, JobCode6, CharacterID7, CharacterLevel7, JobCode7, CharacterID8, CharacterLevel8, JobCode8, TotalRanking
		FROM dbo.MonthlyRank_NestClearTime WITH(NOLOCK)
		WHERE TotalRanking <= 100;'

	EXEC sp_executesql @nvcStmt, N'
		  @dt2StartDate datetime2(0), @dt2EndDate datetime2(0)'
		, @dt2StartDate = @dt2StartDate, @dt2EndDate = @dt2EndDate;
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
