USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountLoginLogByLoginIP]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-20
description : ?? IP???? ?? ??? ??? ?????.
result set : LoginDate, AccountName, JoinIP, RegisterDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5~7 = ????? VIEW ???? ???? ?? by ??? at 2012-02-06
**/

CREATE PROCEDURE [dbo].[P_GetListAccountLoginLogByLoginIP]
	  @datBasisDate date --// ?? ??
	, @vchIP varchar(15) --// ??? IP ??
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
		
	SET @nvcStmt = N'SELECT L.LoginDate, A.AccountName, dbo.FN_IPNumber2IP(A.JoinIP) AS JoinIP, A.RegisterDate
FROM (' + @nvcStmtOUT + N') L
	INNER JOIN dbo.DimAccounts A WITH (NOLOCK) ON L.AccountID = A.AccountID
WHERE L.LoginDate >= ''' + CONVERT(nvarchar(10), @datBasisDate, 121) + N''' AND L.LoginDate < ''' + CONVERT(nvarchar(10), DATEADD(DAY, 1, @datBasisDate), 121) + N'''
	AND L.LoginIP = dbo.FN_IP2IPNumber(@vchIP)
ORDER BY L.LoginDate;'

	EXEC sp_executesql @nvcStmt, N'@vchIP varchar(15)', @vchIP = @vchIP;
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
