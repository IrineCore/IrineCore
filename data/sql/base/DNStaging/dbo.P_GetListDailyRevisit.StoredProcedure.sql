USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDailyRevisit]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-28
description : ?? ????? ?????.
result set : RegisterDate, VisitCount_yyyymmdd, VisitRatio_yyyymmdd, VisitCount_yyyymmdd, VisitRatio_yyyymmdd, ...
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDailyRevisit]
	  @datRegisterDate date --// ?? ?? ?? (?? ?? ??)
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max);
DECLARE @datStartDate date = @datRegisterDate;
DECLARE @datEndDate date;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @datEndDate = MAX(BasisDate) FROM dbo.ETLLogs WITH (NOLOCK);

	SET @nvcStmt = N'
CREATE TABLE #RU (
	BasisDate date NOT NULL,
	AccountID int NOT NULL
);

INSERT #RU (BasisDate, AccountID)
SELECT CAST(RegisterDate AS date) AS BasisDate, AccountID
FROM dbo.DimAccounts WITH (NOLOCK)
WHERE RegisterDate >= CAST(@datRegisterDate AS datetime2(0));

ALTER TABLE #RU ADD PRIMARY KEY (AccountID);

WITH C1 AS (
	SELECT BasisDate AS RegisterDate, BasisDate AS RevisitDate, COUNT(*) AS VisitCount
	FROM #RU
	GROUP BY BasisDate
	UNION ALL
	SELECT A.BasisDate, B.BasisDate, COUNT(*)
	FROM #RU A
		INNER JOIN dbo.DailyUVAccounts B ON A.AccountID = B.AccountID
	WHERE B.BasisDate >= @datRegisterDate
	GROUP BY A.BasisDate, B.BasisDate
)
SELECT A.RegisterDate, B.RevisitDate, B.VisitCount, CAST(B.VisitCount * 100.0 / A.VisitCount AS decimal(5, 2)) AS RevisitRatio
INTO #Results
FROM C1 A
	INNER JOIN C1 B ON A.RegisterDate = B.RegisterDate AND A.RevisitDate = B.RegisterDate
ORDER BY A.RegisterDate, B.RevisitDate;

SELECT RegisterDate';

	WHILE @datStartDate <= @datEndDate
	BEGIN
		SET @nvcStmt += N'
	, ISNULL(MAX(CASE RevisitDate WHEN ''' + CONVERT(nvarchar(10), @datStartDate, 121) + N''' THEN VisitCount END), 0) AS VisitCount_' + CONVERT(nvarchar(8), @datStartDate, 112) + N'
	, ISNULL(MAX(CASE RevisitDate WHEN ''' + CONVERT(nvarchar(10), @datStartDate, 121) + N''' THEN RevisitRatio END), 0) AS RevisitRatio_' + CONVERT(nvarchar(8), @datStartDate, 112);
		SET @datStartDate = DATEADD(DAY, 1, @datStartDate);
	END

	SET @nvcStmt += N'
FROM #Results
GROUP BY RegisterDate
ORDER BY RegisterDate;

DROP TABLE #RU, #Results;'

	EXEC sp_executesql @nvcStmt, N'@datRegisterDate date', @datRegisterDate = @datRegisterDate;
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
