USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListIntroducer]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListIntroducer]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListIntroducer] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-01
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListIntroducer]
	  @nvcAccountName nvarchar(30) = NULL --// (optional) ??? ?? ??
	, @nvcIntroducedAccountName nvarchar(30) = NULL --// (optional) ???? ?? ??? ?? ??
	, @dt2StartDate datetime2(0) = NULL --// (optional) ?? ?? ??
	, @dt2EndDate datetime2(0) = NULL --// (optional) ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int,
	@intAccountID int,
	@intIntroducedAccountID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intAccountID = AccountID FROM dbo.Accounts WITH (NOLOCK) WHERE AccountName = @nvcAccountName;
	SELECT @intIntroducedAccountID = AccountID FROM dbo.Accounts WITH (NOLOCK) WHERE AccountName = @nvcIntroducedAccountName;

	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @intColumnCount = 4;

	SET @inbNullBitmap = 
		CASE WHEN @intAccountID IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @intIntroducedAccountID IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @dt2StartDate IS NOT NULL THEN POWER(2, 2) ELSE 0 END +
		CASE WHEN @dt2EndDate IS NOT NULL THEN POWER(2, 3) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @nvcStmt = N'
SELECT AC1.AccountID, AC1.AccountName, AC2.AccountID AS IntroducedAccountID, AC2.AccountName AS IntroducedAccountName, A.RegisterDate
FROM dbo.Introducers A WITH (NOLOCK)
	INNER JOIN dbo.Accounts AC1 WITH (NOLOCK) ON A.AccountID = AC1.AccountID
	INNER JOIN dbo.Accounts AC2 WITH (NOLOCK) ON A.IntroducedAccountID = AC2.AccountID
'		+ dbo.FN_DynamicWhereClause2(N'A.AccountID', N'@intAccountID', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'A.IntroducedAccountID', N'@intIntroducedAccountID', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'A.RegisterDate', N'@dt2StartDate', @inbNullBitmap, 3, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'A.RegisterDate', N'@dt2EndDate', @inbNullBitmap, 4, @intColumnCount, '<=')
 + N'
ORDER BY A.RegisterDate';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'@intAccountID int, @intIntroducedAccountID int, @dt2StartDate datetime2(0), @dt2EndDate datetime2(0)'
		, @intAccountID = @intAccountID, @intIntroducedAccountID = @intIntroducedAccountID, @dt2StartDate = @dt2StartDate, @dt2EndDate = @dt2EndDate;

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
