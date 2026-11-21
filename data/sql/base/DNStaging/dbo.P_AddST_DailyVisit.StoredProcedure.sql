USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyVisit]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2012-09-12
description : ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ?? ?? at 2012-09-12
2 = NULL ? ?? by ??? at 2012-09-19
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyVisit]
	  @datBasisDate date --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @datStartDate date;
SET @datBasisDate = ISNULL(@datBasisDate, DATEADD(DD, -1, CAST(GETDATE() AS date)));
SET @datStartDate = DATEADD(DD, -6, @datBasisDate);
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'MapStayLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @datStartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;

	IF OBJECT_ID('tempdb..#t1') IS NOT NULL DROP TABLE #t1;
	CREATE TABLE #t1 (BasisDate date, D15NoVisitCount int, D30NoVisitCount int, D31NoVisitCount int);
	CREATE CLUSTERED INDEX #IX_NC_t1 ON #t1 (BasisDate);
	
	INSERT #t1 (BasisDate, D15NoVisitCount, D30NoVisitCount, D31NoVisitCount)
	SELECT @datBasisDate AS BasisDate
		, ISNULL(SUM(CASE WHEN LastLoginDate = DATEADD(DD, -15, @nvcStartDate) THEN 1 ELSE 0 END), 0) AS D15NoVisitCount
		, ISNULL(SUM(CASE WHEN LastLoginDate = DATEADD(DD, -30, @nvcStartDate) THEN 1 ELSE 0 END), 0) AS D30NoVisitCount
		, ISNULL(SUM(CASE WHEN LastLoginDate <= DATEADD(DD, -31, @nvcStartDate) THEN 1 ELSE 0 END), 0) AS D31NoVisitCount
	FROM (SELECT CAST(MAX(LastLoginDate) AS date) AS LastLoginDate FROM dbo.DimCharacters WITH (NOLOCK) WHERE AccountLevelCode = 0 GROUP BY AccountID) A;
	
	SET @nvcStmt = N'
		SELECT ''' + @nvcEndDate + N''' AS BasisDate
			, ISNULL(SUM(CASE WHEN Cnt >=5 THEN 1 ELSE 0 END), 0) AS HardUserCnt
			, ISNULL(AVG(CASE WHEN Cnt >= 5 THEN PlayTimeMin / Cnt END), 0) AS HardUserPlayTimeMinAvg
			, ISNULL(SUM(CASE WHEN Cnt <= 3 THEN 1 ELSE 0 END), 0) AS LightUserCnt
			, ISNULL(AVG(CASE WHEN Cnt <= 3 THEN PlayTimeMin / Cnt END), 0) AS LightUserPlayTimeMinAvg
		FROM
		(
			SELECT COUNT(DISTINCT CAST(V.ExitDate AS date)) AS Cnt, dc.AccountID, SUM(DATEDIFF(mi, V.EntranceDate, V.ExitDate)) AS PlayTimeMin
				FROM (' + @nvcStmtOUT + N') V
				INNER JOIN dbo.DimCharacters AS dc WITH (NOLOCK) ON V.CharacterID = dc.CharacterID
				WHERE V.ExitDate >= @nvcStartDate AND dc.AccountLevelCode = 0
				GROUP BY dc.AccountID
		) x;
	';
	
	IF OBJECT_ID('tempdb..#t2') IS NOT NULL DROP TABLE #t2;
	CREATE TABLE #t2 (BasisDate date, HardUserCnt int, HardUserPlayTimeMinAvg int, LightUserCnt int, LightUserPlayTimeMinAvg int)
	CREATE CLUSTERED INDEX #IX_NC_t2 ON #t2 (BasisDate);
	
	INSERT #t2 (BasisDate, HardUserCnt, HardUserPlayTimeMinAvg, LightUserCnt, LightUserPlayTimeMinAvg)
	EXEC sp_executesql @nvcStmt, N'@nvcStartDate datetime', @nvcStartDate = @nvcStartDate;

	INSERT dbo.ST_DailyVisit (BasisDate, D15NoVisitCount, D30NoVisitCount, D31NoVisitCount, HardUserCnt, HardUserPlayTimeMinAvg, LightUserCnt, LightUserPlayTimeMinAvg)
	SELECT a.BasisDate, D15NoVisitCount, D30NoVisitCount,  D31NoVisitCount, b.HardUserCnt, b.HardUserPlayTimeMinAvg, b.LightUserCnt, b.LightUserPlayTimeMinAvg
	FROM #t1 a INNER JOIN #t2 b ON a.BasisDate = b.BasisDate	
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
