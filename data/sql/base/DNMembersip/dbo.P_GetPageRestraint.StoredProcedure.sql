USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageRestraint]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetPageRestraint]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetPageRestraint] AS' 
END
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-08
description : ?? ??? ????? ?????? ?????.
result set : 
	@intSelectClauseCode = 1 : RestraintID, RestraintTargetCode, AccountID, AccountName, CharacterID, CharacterName, WorldID, RestraintReasonID, RestraintTypeCode, AdminID, Memo, RestraintMessage, StartDate, EndDate, RegisterDate, CancelFlag, CancelAdminID, CancelMemo, CancelDate, LastLoginIP
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101120 = ?? ??? ????.
history :
4 = ????, ??????? ??? ? ??? input param. ?? at 2010-07-19
**/

ALTER PROCEDURE [dbo].[P_GetPageRestraint]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @intSelectClauseCode int --// ?? ?? ??.
	, @inyRestraintTargetCode tinyint = NULL --// 1=??, 2=???
	, @inyRestraintTypeCode tinyint --// 1=?? ??, 2=?? ??, 3=?? ??
	, @intRestraintReasonID int = NULL --// ?? ?? ID (DNGSM.dbo.RestraintReasons.RestraintReasonID? ?????.)
	, @intAdminID int = NULL --// ??? ??? ID
	, @dt2StartRegisterDate datetime2(0) = NULL --// ?? ?? ?? - ?? ??
	, @dt2EndRegisterDate datetime2(0) = NULL --// ?? ? ?? - ?? ??
	, @nvcAccountName nvarchar(30)  = NULL --// ??? ?? ??
	, @nvcCharacterName nvarchar(50) = NULL --// ??? ??? ??
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
	@intColumnCount int;

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
			WHEN @intSelectClauseCode = 1 THEN N'RestraintID, RestraintTargetCode, AccountID, AccountName, CharacterID, CharacterName, WorldID, RestraintReasonID, RestraintTypeCode, AdminID, Memo, RestraintMessage, StartDate, EndDate, RegisterDate, CancelFlag, CancelAdminID, CancelMemo, CancelDate, LastLoginIP, ISNULL(CurServerID, 0) AS CurServerID'
		END;

	SET @nvcSelectStmt2 = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'R.RestraintID, R.RestraintTargetCode, R.AccountID, COALESCE(ACC1.AccountName, ACC2.AccountName) AS AccountName, R.CharacterID, CH.CharacterName, CH.WorldID, R.RestraintReasonID, R.RestraintTypeCode, R.AdminID, R.Memo, R.RestraintMessage, R.StartDate, R.EndDate, R.RegisterDate, R.CancelFlag, CR.AdminID AS CancelAdminID, CR.Memo AS CancelMemo, CR.CancelDate, dbo.FN_IPNumber2IP(ACC1.LastLoginIP) AS LastLoginIP, AU.CurServerID'
		END;

	SET @intColumnCount = 8;

	SET @inbNullBitmap = 
		CASE WHEN @inyRestraintTargetCode IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @inyRestraintTypeCode IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @intRestraintReasonID IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @intAdminID IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @dt2StartRegisterDate IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @dt2EndRegisterDate IS NOT NULL THEN POWER(2, 5) ELSE 0 END + 
		CASE WHEN @nvcAccountName IS NOT NULL THEN POWER(2, 6) ELSE 0 END + 
		CASE WHEN @nvcCharacterName IS NOT NULL THEN POWER(2, 7) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @dt2EndRegisterDate = DATEADD(DAY, 1, @dt2EndRegisterDate);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY R.RestraintID DESC) AS rowNumber, ' + @nvcSelectStmt2 + N'
	FROM dbo.Restraints R WITH (NOLOCK)
		LEFT OUTER JOIN dbo.Accounts ACC1 WITH (NOLOCK) ON R.AccountID = ACC1.AccountID
		LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON R.CharacterID = CH.CharacterID
		LEFT OUTER JOIN dbo.Accounts ACC2 WITH (NOLOCK) ON CH.AccountID = ACC2.AccountID
		LEFT OUTER JOIN dbo.CanceledRestraints CR WITH (NOLOCK) ON R.RestraintID = CR.RestraintID
		LEFT OUTER JOIN dbo.DNAuth AU WITH (NOLOCK) ON R.AccountID = AU.AccountDBID
'		+ dbo.FN_DynamicWhereClause2(N'R.RestraintTargetCode', N'@inyRestraintTargetCode', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'R.RestraintTypeCode', N'@inyRestraintTypeCode', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'R.RestraintReasonID', N'@intRestraintReasonID', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'R.AdminID', N'@intAdminID', @inbNullBitmap, 4, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'R.RegisterDate', N'@dt2StartRegisterDate', @inbNullBitmap, 5, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'R.RegisterDate', N'@dt2EndRegisterDate', @inbNullBitmap, 6, @intColumnCount, '<')
		+ dbo.FN_DynamicWhereClause2(N'ACC1.AccountName', N'@nvcAccountName', @inbNullBitmap, 7, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'CH.CharacterName', N'@nvcCharacterName', @inbNullBitmap, 8, @intColumnCount, '=') + N'
)
SELECT ' + @nvcSelectStmt + N'
FROM PagingCTE
WHERE rowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY rowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @inyRestraintTargetCode tinyint
			, @inyRestraintTypeCode tinyint
			, @intRestraintReasonID int
			, @intAdminID int
			, @dt2StartRegisterDate datetime2(0)
			, @dt2EndRegisterDate datetime2(0)
			, @nvcAccountName nvarchar(30)
			, @nvcCharacterName nvarchar(50)
			, @intPageNumber int
			, @intPageSize int
		'
		, @inyRestraintTargetCode = @inyRestraintTargetCode
		, @inyRestraintTypeCode = @inyRestraintTypeCode
		, @intRestraintReasonID = @intRestraintReasonID
		, @intAdminID = @intAdminID
		, @dt2StartRegisterDate = @dt2StartRegisterDate
		, @dt2EndRegisterDate = @dt2EndRegisterDate
		, @nvcAccountName = @nvcAccountName
		, @nvcCharacterName = @nvcCharacterName
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, ?(?) ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101120;
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
