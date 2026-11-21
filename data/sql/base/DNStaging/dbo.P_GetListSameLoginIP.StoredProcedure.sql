USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListSameLoginIP]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-20
description : ?? IP??? n? ?? ???? ??? ?????.
result set : LoginIP, AccountCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
6~7 = ????? VIEW ???? ???? ?? by ??? at 2012-02-06
**/

CREATE PROCEDURE [dbo].[P_GetListSameLoginIP]
	  @datBasisDate date --// ?? ??
	, @intBasisCount int --// ?? IP? ???? ?? ??
    , @bitExcludeRestraintAccount bit = NULL --// ???? ?? ?? (0=??, 1=???)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max)
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

	IF @bitExcludeRestraintAccount = 1

	SET @nvcStmt = N'SELECT dbo.FN_IPNumber2IP(L.LoginIP) AS LoginIP, COUNT(DISTINCT L.AccountID) AS AccountCount
FROM (' + @nvcStmtOUT + N') L LEFT OUTER JOIN dbo.SS_RestraintedAccounts R ON L.AccountID = R.AccountID
WHERE LoginDate >= ''' + CONVERT(nvarchar(10), @datBasisDate, 121) + N''' AND LoginDate < ''' + CONVERT(nvarchar(10), DATEADD(DAY, 1, @datBasisDate), 121) + N'''
AND R.AccountID IS NULL
GROUP BY LoginIP
HAVING COUNT(DISTINCT L.AccountID) >= @intBasisCount
ORDER BY LoginIP';

	ELSE

	SET @nvcStmt = N'SELECT dbo.FN_IPNumber2IP(LoginIP) AS LoginIP, COUNT(DISTINCT AccountID) AS AccountCount
FROM (' + @nvcStmtOUT + N') L 
WHERE LoginDate >= ''' + CONVERT(nvarchar(10), @datBasisDate, 121) + N''' AND LoginDate < ''' + CONVERT(nvarchar(10), DATEADD(DAY, 1, @datBasisDate), 121) + N'''
GROUP BY LoginIP
HAVING COUNT(DISTINCT AccountID) >= @intBasisCount
ORDER BY LoginIP';

	EXEC sp_executesql @nvcStmt, N'@intBasisCount int', @intBasisCount = @intBasisCount;
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
