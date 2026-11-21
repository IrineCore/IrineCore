USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddUVAccount]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-22
description : ?? UV ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
10~11 = ????? VIEW ???? ???? ?? by ??? at 2012-02-06
**/

CREATE PROCEDURE [dbo].[P_AddUVAccount]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int;
DECLARE @datBasisDate date = CAST(@dtmBasisDate AS date);
DECLARE @dt2BasisDate datetime2(0) = CAST(@datBasisDate AS datetime2(0));
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'AccountLoginLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)
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
		
	SET @nvcStmt = N'	
	INSERT dbo.DailyUVAccounts (BasisDate, AccountID)
	SELECT @datBasisDate, AccountID
	FROM (' + @nvcStmtOUT + N') L
	WHERE LoginDate >= @dt2BasisDate AND LoginDate < DATEADD(DAY, 1, @dt2BasisDate)
	GROUP BY AccountID;
	'
	EXEC sp_executesql @nvcStmt, N'@datBasisDate date, @dt2BasisDate datetime2(0)', @datBasisDate = @datBasisDate, @dt2BasisDate = @dt2BasisDate;

	IF DATEPART(WEEKDAY, @datBasisDate) = 7
		INSERT dbo.WeeklyUVAccounts (BasisDate, AccountID, VisitDayCount)
		SELECT DATEADD(DAY, -6, @datBasisDate), AccountID, CAST(COUNT(*) AS tinyint)
		FROM dbo.DailyUVAccounts
		WHERE BasisDate BETWEEN DATEADD(DAY, -6, @datBasisDate) AND @datBasisDate
		GROUP BY AccountID;

	IF DATEPART(DAY, DATEADD(DAY, 1, @datBasisDate)) = 1
		INSERT dbo.MonthlyUVAccounts (BasisDate, AccountID, VisitDayCount)
		SELECT DATEADD(MONTH, DATEDIFF(MONTH, 0, @datBasisDate), 0), AccountID, CAST(COUNT(*) AS tinyint)
		FROM dbo.DailyUVAccounts
		WHERE BasisDate BETWEEN DATEADD(MONTH, DATEDIFF(MONTH, 0, @datBasisDate), 0) AND @datBasisDate
		GROUP BY AccountID;
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
