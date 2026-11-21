USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccontLoginForActoz]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-03-11
description : ??? ?? ??? ??? ??? ??? ?????.
result set : BasisDate/date, AccountName/nvarchar(50), A.AccountID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListAccontLoginForActoz]
	  @datBasisDate date --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @dt2StartDate datetime2(0) = CAST(@datBasisDate as datetime2(0))
	  , @dt2EndDate datetime2(0) = DATEADD(SECOND, 86399, CAST(@datBasisDate as datetime2(0)))

DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'MapStayLogs'
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
	/** ????? ??? ??**/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
	SET @nvcStmt = N'
	SELECT @datBasisDate AS BasisDate, C.AccountName, C.AccountID
	FROM (' + @nvcStmtOUT + N') A
		INNER JOIN dbo.DimCharacters B On A.CharacterID = B.CharacterID
		INNER JOIN dbo.DimAccounts C ON B.AccountID = C.AccountID
	WHERE A.ExitDate BETWEEN @dt2StartDate AND @dt2EndDate
	GROUP BY C.AccountName, C.AccountID
	HAVING SUM(DATEDIFF(SECOND, EntranceDate, ExitDate)) >= 3600;'
	EXEC sp_executesql @nvcStmt, N'@datBasisDate date, @dt2StartDate datetime2(0), @dt2EndDate datetime2(0)',@datBasisDate = @datBasisDate, @dt2StartDate = @dt2StartDate, @dt2EndDate = @dt2EndDate;

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
