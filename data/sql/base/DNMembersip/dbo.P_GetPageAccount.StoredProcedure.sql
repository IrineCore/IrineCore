USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageAccount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetPageAccount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetPageAccount] AS' 
END
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : ?? ??? ????? ?????? ?????.
result set : 
	@intSelectClauseCode = 1 : AccountID, AccountName, AccountLevelCode, CharacterCreateLimit, CharacterMaxCount, LastLoginDate, LastLogoutDate, LastLoginIP, JoinIP, RegisterDate, GenderCode, BirthDate, AccountRestraintFlag, CharacterCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_GetPageAccount]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @intSelectClauseCode int --// ?? ?? ??.
	, @nvcAccountName nvarchar(50) = NULL --// ??? ?? ??
	, @inyAccountLevelCode tinyint = NULL --// 0=?? ??, 10=??(??), 20=????(??), 30=???(??), 99=QA, 100=???
	, @vchLastLoginIP varchar(15) = NULL --// ?? ???? IP ??
	, @vchJoinIP varchar(15) = NULL --// ?? Join ? IP ??
	, @dt2StartRegisterDate datetime2(0) = NULL --// ?? ?? ?? - ?? ??
	, @dt2EndRegisterDate datetime2(0) = NULL --// ?? ? ?? - ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@nvcSelectStmt2 nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int,
	@intLastLoginIP int,
	@intJoinIP int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'AccountID, AccountName, AccountLevelCode, CharacterCreateLimit, CharacterMaxCount, LastLoginDate, LastLogoutDate, LastLoginIP, JoinIP, RegisterDate, GenderCode, BirthDate, AccountRestraintFlag, CharacterCount'
		END;

	SET @nvcSelectStmt2 = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'ACC.AccountID, ACC.AccountName, ACC.AccountLevelCode, CASE WHEN ACC.LastCharacterCreateDate < CAST(GETDATE() AS date) THEN ''MAX'' ELSE CAST(ACC.CharacterCreateLimit AS varchar(10)) END AS CharacterCreateLimit, ACC.CharacterMaxCount, ACC.LastLoginDate, ACC.LastLogoutDate, dbo.FN_IPNumber2IP(ACC.LastLoginIP) AS LastLoginIP, dbo.FN_IPNumber2IP(ACC.JoinIP) AS JoinIP, ACC.RegisterDate, ACC.GenderCode, ACC.BirthDate, CASE WHEN EXISTS (SELECT * FROM dbo.Restraints WITH (NOLOCK) WHERE AccountID = ACC.AccountID AND CAST(GETDATE() AS smalldatetime) BETWEEN StartDate AND EndDate AND CancelFlag = CAST(0 AS bit)) THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS AccountRestraintFlag, CAST(CC.CharacterCount AS int) AS CharacterCount'
		END;

	SET @intColumnCount = 6;

	SET @intLastLoginIP = dbo.FN_IP2IPNumber(@vchLastLoginIP);
	SET @intJoinIP = dbo.FN_IP2IPNumber(@vchJoinIP);

	SET @inbNullBitmap = 
		CASE WHEN @nvcAccountName IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @inyAccountLevelCode IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @intLastLoginIP IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @intJoinIP IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @dt2StartRegisterDate IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @dt2EndRegisterDate IS NOT NULL THEN POWER(2, 5) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @dt2EndRegisterDate = DATEADD(DAY, 1, @dt2EndRegisterDate);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY ACC.AccountID DESC) AS rowNumber, ' + @nvcSelectStmt2 + N'
	FROM dbo.Accounts ACC WITH (NOLOCK)
		LEFT OUTER JOIN dbo.VIX_CharacterCount CC WITH (NOEXPAND, NOLOCK) ON ACC.AccountID = CC.AccountID
'		+ dbo.FN_DynamicWhereClause2(N'ACC.AccountName', N'@nvcAccountName', @inbNullBitmap, 1, @intColumnCount, 'A%')
		+ dbo.FN_DynamicWhereClause2(N'ACC.AccountLevelCode', N'@inyAccountLevelCode', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'ACC.LastLoginIP', N'@intLastLoginIP', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'ACC.JoinIP', N'@intJoinIP', @inbNullBitmap, 4, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'ACC.RegisterDate', N'@dt2StartRegisterDate', @inbNullBitmap, 5, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'ACC.RegisterDate', N'@dt2EndRegisterDate', @inbNullBitmap, 6, @intColumnCount, '<') + N'
)
SELECT ' + @nvcSelectStmt + N'
FROM PagingCTE
WHERE rowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY rowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcAccountName nvarchar(50)
			, @inyAccountLevelCode tinyint
			, @intLastLoginIP int
			, @intJoinIP int
			, @dt2StartRegisterDate datetime2(0)
			, @dt2EndRegisterDate datetime2(0)
			, @intPageNumber int
			, @intPageSize int
		'
		, @nvcAccountName = @nvcAccountName
		, @inyAccountLevelCode = @inyAccountLevelCode
		, @intLastLoginIP = @intLastLoginIP
		, @intJoinIP = @intJoinIP
		, @dt2StartRegisterDate = @dt2StartRegisterDate
		, @dt2EndRegisterDate = @dt2EndRegisterDate
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
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
