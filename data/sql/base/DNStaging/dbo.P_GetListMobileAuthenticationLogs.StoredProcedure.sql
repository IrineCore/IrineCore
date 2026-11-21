USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMobileAuthenticationLogs]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-11-12
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListMobileAuthenticationLogs]
	  @dt2StartLogDate datetime2(0) --// (?? ??) ?? ?? ??
	, @dt2EndLogDate datetime2(0) --// (?? ??) ?? ? ??
	, @intAccountID int = NULL --// ??ID	
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @nvcStmt nvarchar(max);

DECLARE @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'MobileAuthenticationLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartLogDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndLogDate, 112)
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

	SET @nvcStmt = N'
	SELECT LogDate, AccountID, AuthenticationLogCode, AccountAuthenticationKey
	  FROM (' + @nvcStmtOUT + N') V	
	 WHERE LogDate >= @dt2StartLogDate AND LogDate <= @dt2EndLogDate
	   AND AccountID = COALESCE(@intAccountID, AccountID);'

	EXEC sp_executesql @nvcStmt, N'
	  @dt2StartLogDate datetime2(0), @dt2EndLogDate datetime2(0), @intAccountID int'
	, @dt2StartLogDate = @dt2StartLogDate, @dt2EndLogDate = @dt2EndLogDate, @intAccountID = @intAccountID
	
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
