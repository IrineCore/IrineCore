USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageAccountLoginLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-22
description : ?? ??? ??? ????? ?????? ?????.
result set : LoginDate/datetime2, AccountID/int, AccountName/nvarchar, LoginIP/varchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104101 = ?? ?? ??? ????.
history :
5~6 = ????? VIEW ???? ???? ?? by ??? at 2012-02-06
**/

CREATE PROCEDURE [dbo].[P_GetPageAccountLoginLog]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @nvcAccountName nvarchar(50) = NULL --// ??? ??
	, @vchStartLoginIP varchar(15) = NULL --// ?? ?? IP ??
	, @vchEndLoginIP varchar(15) = NULL --// ?? ? IP ??
	, @dt2StartLoginDate datetime2(0) --// (?? ??) ?? ?? ?? - ??? ?? ??
	, @dt2EndLoginDate datetime2(0) --// (?? ??) ?? ? ?? - ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int,
	@intStartLoginIP int = dbo.FN_IP2IPNumber(@vchStartLoginIP),
	@intEndLoginIP int = dbo.FN_IP2IPNumber(@vchEndLoginIP);
	
DECLARE @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'AccountLoginLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartLoginDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndLoginDate, 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';	

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @intColumnCount = 5;

	SET @inbNullBitmap = 
		CASE WHEN @dt2StartLoginDate IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @dt2EndLoginDate IS NOT NULL THEN POWER(2, 1) ELSE 0 END +
		CASE WHEN @nvcAccountName IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @intStartLoginIP IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @intEndLoginIP IS NOT NULL THEN POWER(2, 4) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);
	
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY V.LoginDate) AS RowNumber, V.LoginDate, DA.AccountID, DA.AccountName, dbo.FN_IPNumber2IP(V.LoginIP) AS LoginIP
	FROM (' + @nvcStmtOUT + N') V
		INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON V.AccountID = DA.AccountID
	WHERE V.LoginDate BETWEEN ''' + CONVERT(nvarchar(19), @dt2StartLoginDate, 121) + N''' AND ''' + CONVERT(nvarchar(19), @dt2EndLoginDate, 121) + N'''
'		+ CASE WHEN @inbNullBitmap > 3 THEN N' AND ' ELSE N'' END
		+ dbo.FN_DynamicWhereClause2(N'DA.AccountName', N'@nvcAccountName', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'V.LoginIP', N'@intStartLoginIP', @inbNullBitmap, 4, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'V.LoginIP', N'@intEndLoginIP', @inbNullBitmap, 5, @intColumnCount, '<=') + N'
)
SELECT LoginDate, AccountID, AccountName, LoginIP
FROM PagingCTE
WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY RowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcAccountName nvarchar(30)
			, @intStartLoginIP int
			, @intEndLoginIP int
			, @intPageNumber int
			, @intPageSize int
		'
		, @nvcAccountName = @nvcAccountName
		, @intStartLoginIP = @intStartLoginIP
		, @intEndLoginIP = @intEndLoginIP
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, 104101? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 104101;
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
