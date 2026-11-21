USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyDomant]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-08
description : ?? ??, ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~3 = ???? by ??? at 2013-04-08
4 = CTE ?? by ??? at 2013-05-20
5 = ?? ?? Hash ???? ?? by ??? at 2013-05-20
6 = ???? ?? ?? by ??? at 2013-06-28
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyDomant]
	  @dtmBasisDate datetime = NULL --// ?? ??
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
	SET @dtmBasisDate = CAST(ISNULL(@dtmBasisDate, GETDATE()) AS date);
	
	--7? ???
	IF OBJECT_ID('tempdb..#A7') IS NOT NULL DROP TABLE #A7;
	SELECT a.AccountID
	, CASE WHEN b.RegisterDate = DATEADD(DD, -30, @dtmBasisDate) THEN 1 
		 WHEN b.RegisterDate >= DATEADD(WW, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 2
		 WHEN b.RegisterDate >= DATEADD(WW, -2, DATEADD(DD, -30, @dtmBasisDate)) THEN 3
		 WHEN b.RegisterDate >= DATEADD(WW, -3, DATEADD(DD, -30, @dtmBasisDate)) THEN 4
		 WHEN b.RegisterDate >= DATEADD(WW, -4, DATEADD(DD, -30, @dtmBasisDate)) THEN 5
		 WHEN b.RegisterDate >= DATEADD(YY, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 5
		 WHEN b.RegisterDate < DATEADD(YY, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 6 END AS RegisterCode
	INTO #A7
	FROM dbo.DailyUVAccounts a WITH (NOLOCK, INDEX(IX_NC_DailyUVAccounts_BasisDate)) 
		INNER JOIN dbo.DimAccounts b WITH (NOLOCK, INDEX(PK_DimAccounts)) ON b.AccountID = a.AccountID
	WHERE a.BasisDate >= DATEADD(DD, -7, @dtmBasisDate) AND a.BasisDate < DATEADD(DD, -6, @dtmBasisDate)
	OPTION (HASH JOIN);
	
	--15? ???
	IF OBJECT_ID('tempdb..#A15') IS NOT NULL DROP TABLE #A15;
	SELECT a.AccountID
	, CASE WHEN b.RegisterDate = DATEADD(DD, -30, @dtmBasisDate) THEN 1 
		 WHEN b.RegisterDate >= DATEADD(WW, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 2
		 WHEN b.RegisterDate >= DATEADD(WW, -2, DATEADD(DD, -30, @dtmBasisDate)) THEN 3
		 WHEN b.RegisterDate >= DATEADD(WW, -3, DATEADD(DD, -30, @dtmBasisDate)) THEN 4
		 WHEN b.RegisterDate >= DATEADD(WW, -4, DATEADD(DD, -30, @dtmBasisDate)) THEN 5
		 WHEN b.RegisterDate >= DATEADD(YY, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 5
		 WHEN b.RegisterDate < DATEADD(YY, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 6 END AS RegisterCode
	INTO #A15
	FROM dbo.DailyUVAccounts a WITH (NOLOCK, INDEX(IX_NC_DailyUVAccounts_BasisDate)) 
		INNER JOIN dbo.DimAccounts b WITH (NOLOCK, INDEX(PK_DimAccounts)) ON b.AccountID = a.AccountID
	WHERE a.BasisDate >= DATEADD(DD, -15, @dtmBasisDate) AND a.BasisDate < DATEADD(DD, -14, @dtmBasisDate)
	OPTION (HASH JOIN);

	--30? ???
	IF OBJECT_ID('tempdb..#A30') IS NOT NULL DROP TABLE #A30;
	SELECT a.AccountID
	, CASE WHEN b.RegisterDate = DATEADD(DD, -30, @dtmBasisDate) THEN 1 
		 WHEN b.RegisterDate >= DATEADD(WW, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 2
		 WHEN b.RegisterDate >= DATEADD(WW, -2, DATEADD(DD, -30, @dtmBasisDate)) THEN 3
		 WHEN b.RegisterDate >= DATEADD(WW, -3, DATEADD(DD, -30, @dtmBasisDate)) THEN 4
		 WHEN b.RegisterDate >= DATEADD(WW, -4, DATEADD(DD, -30, @dtmBasisDate)) THEN 5
		 WHEN b.RegisterDate >= DATEADD(YY, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 5
		 WHEN b.RegisterDate < DATEADD(YY, -1, DATEADD(DD, -30, @dtmBasisDate)) THEN 6 END AS RegisterCode
	INTO #A30
	FROM dbo.DailyUVAccounts a WITH (NOLOCK, INDEX(IX_NC_DailyUVAccounts_BasisDate)) 
		INNER JOIN dbo.DimAccounts b WITH (NOLOCK, INDEX(PK_DimAccounts)) ON b.AccountID = a.AccountID
	WHERE a.BasisDate >= DATEADD(DD, -31, @dtmBasisDate) AND a.BasisDate < DATEADD(DD, -30, @dtmBasisDate)
	OPTION (HASH JOIN);
	
	
	--7? ??
	IF OBJECT_ID('tempdb..#R7') IS NOT NULL DROP TABLE #R7;
	SELECT DATEADD(DD, -7, @dtmBasisDate) AS BasisDate, RegisterCode, SUM(d7) AS d7
	INTO #R7
	FROM
	(
		SELECT a.RegisterCode, CASE WHEN ISNULL(MIN(b.BasisDate), @dtmBasisDate) >= @dtmBasisDate THEN 1 ELSE 0 END AS d7
		FROM #A7 a LEFT OUTER JOIN dbo.DailyUVAccounts b WITH (NOLOCK) ON b.AccountID = a.AccountID AND b.BasisDate >= DATEADD(DD, -6, @dtmBasisDate) AND b.BasisDate < @dtmBasisDate
		GROUP BY a.AccountID, a.RegisterCode
	) x
	GROUP BY RegisterCode
	OPTION (HASH JOIN);
	
	--7? ??, 15? ??
	IF OBJECT_ID('tempdb..#R15') IS NOT NULL DROP TABLE #R15;
	SELECT DATEADD(DD, -15, @dtmBasisDate) AS BasisDate, RegisterCode, SUM(r7) AS r7, SUM(d15) AS d15
	INTO #R15
	FROM
	(
		SELECT a.RegisterCode, CASE WHEN MIN(b.BasisDate) >= DATEADD(DD, -8, @dtmBasisDate) THEN 1 ELSE 0 END AS r7
			, CASE WHEN ISNULL(MIN(b.BasisDate), @dtmBasisDate) >= @dtmBasisDate THEN 1 ELSE 0 END AS d15
		FROM #A15 a LEFT OUTER JOIN dbo.DailyUVAccounts b WITH (NOLOCK) ON b.AccountID = a.AccountID AND b.BasisDate >= DATEADD(DD, -14, @dtmBasisDate) AND b.BasisDate < @dtmBasisDate
		GROUP BY a.AccountID, a.RegisterCode
	) x
	GROUP BY RegisterCode
	OPTION (HASH JOIN);
	
	--15? ??, 30? ??/??
	IF OBJECT_ID('tempdb..#R30') IS NOT NULL DROP TABLE #R30;
	SELECT DATEADD(DD, -31, @dtmBasisDate) AS BasisDate, RegisterCode, SUM(d30) AS d30, SUM(r15) AS r15, SUM(r30) AS r30
	INTO #R30
	FROM
	(
		SELECT a.RegisterCode, CASE WHEN ISNULL(MIN(b.BasisDate), @dtmBasisDate) >= DATEADD(DD, -1, @dtmBasisDate) THEN 1 ELSE 0 END AS d30
			, CASE WHEN MIN(b.BasisDate) >= DATEADD(DD, -16, @dtmBasisDate) THEN 1 ELSE 0 END AS r15
			, CASE WHEN MIN(b.BasisDate) >= DATEADD(DD, -1, @dtmBasisDate) THEN 1 ELSE 0 END AS r30
		FROM #A30 a LEFT OUTER JOIN dbo.DailyUVAccounts b WITH (NOLOCK) ON b.AccountID = a.AccountID AND b.BasisDate >= DATEADD(DD, -30, @dtmBasisDate) AND b.BasisDate <= @dtmBasisDate
		GROUP BY a.AccountID, a.RegisterCode
	) x
	GROUP BY RegisterCode
	OPTION (HASH JOIN);


	/**_# [ST_DailyDomant] ???? INSERT???.*/
	--ST_DailyDomant (?? UV ? ???, 7? ??)
	;MERGE dbo.ST_DailyDomant AS t
	USING #R7 AS s ON (t.BasisDate = s.BasisDate AND t.RegisterCode = s.RegisterCode)
	WHEN MATCHED THEN
		UPDATE SET t.D7LeaveCount = ISNULL(s.d7, t.D7LeaveCount)
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (BasisDate, RegisterCode, D7LeaveCount)
		VALUES (s.BasisDate, s.RegisterCode, s.d7)
	OPTION (MAXDOP 1);

	--ST_DailyDomant (?? UV ? ???, 7? ??, 15? ??)
	;MERGE dbo.ST_DailyDomant AS t
	USING #R15 AS s ON (t.BasisDate = s.BasisDate AND t.RegisterCode = s.RegisterCode)
	WHEN MATCHED THEN
		UPDATE SET t.D15LeaveCount = ISNULL(s.d15, t.D15LeaveCount), t.D7ReviveCount = ISNULL(s.r7, t.D7ReviveCount)
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (BasisDate, RegisterCode, D15LeaveCount, D7ReviveCount)
		VALUES (s.BasisDate, s.RegisterCode, s.d15, s.r7)
	OPTION (MAXDOP 1);

	--ST_DailyDomant (?? UV ? ???, 15? ??, 30? ??/??)
	;MERGE dbo.ST_DailyDomant AS t
	USING #R30 AS s ON (t.BasisDate = s.BasisDate AND t.RegisterCode = s.RegisterCode)
	WHEN MATCHED THEN
		UPDATE SET t.D30LeaveCount = ISNULL(s.d30, t.D30LeaveCount), t.D15ReviveCount = ISNULL(s.r15, t.D15ReviveCount), t.D30ReviveCount = ISNULL(s.r30, t.D30ReviveCount)
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (BasisDate, RegisterCode, D30LeaveCount, D15ReviveCount, D30ReviveCount)
		VALUES (s.BasisDate, s.RegisterCode, s.d30, s.r15, s.r30)
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
