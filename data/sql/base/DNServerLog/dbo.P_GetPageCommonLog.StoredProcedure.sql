USE [dnserverlog]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageCommonLog]    Script Date: 11/14/2026 8:58:01 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-17
description : ?? ??? ????? ?????? ?????. ?? ??? ?? ?????.
result set : LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetPageCommonLog]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @inySortCode tinyint --// 1=?? ?, 2=?? ??
	, @inyLogType tinyint = NULL --// ?? ??
	, @intServerType int = NULL --// ?? ??
	, @intWorldID int = NULL --// ?? ID
	, @intAccountID int = NULL --// ?? ID
	, @inbCharacterID bigint = NULL --// ??? ID
	, @intSessionID int = NULL --// ?? ID
	, @dt2StartLogDate datetime2(0) --// ?? ?? ??
	, @dt2EndLogDate datetime2(0) --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @dt2StartLogDate IS NULL OR @dt2EndLogDate IS NULL
		RETURN 100;

	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = N'LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText'

	SET @intColumnCount = 8;

	SET @inbNullBitmap = 
		CASE WHEN @inyLogType IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @intServerType IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @intWorldID IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @intAccountID IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @inbCharacterID IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @intSessionID IS NOT NULL THEN POWER(2, 5) ELSE 0 END + 
		CASE WHEN @dt2StartLogDate IS NOT NULL THEN POWER(2, 6) ELSE 0 END + 
		CASE WHEN @dt2EndLogDate IS NOT NULL THEN POWER(2, 7) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY LogDate ' + CASE @inySortCode WHEN 1 THEN N'ASC' ELSE N'DESC' END + N') AS rowNumber, ' + @nvcSelectStmt + N'
	FROM dbo.V_CommonLogs WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'LogType', N'@inyLogType', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'ServerType', N'@intServerType', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'WorldID', N'@intWorldID', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'AccountID', N'@intAccountID', @inbNullBitmap, 4, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'CharacterID', N'@inbCharacterID', @inbNullBitmap, 5, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'SessionID', N'@intSessionID', @inbNullBitmap, 6, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dt2StartLogDate', @inbNullBitmap, 7, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dt2EndLogDate', @inbNullBitmap, 8, @intColumnCount, '<=') + N'
)
SELECT ' + @nvcSelectStmt + N'
FROM PagingCTE
WHERE rowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY rowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @inyLogType tinyint
			, @intServerType int
			, @intWorldID int
			, @intAccountID int
			, @inbCharacterID bigint
			, @intSessionID int
			, @dt2StartLogDate datetime2(0)
			, @dt2EndLogDate datetime2(0)
			, @intPageNumber int
			, @intPageSize int
		'
		, @inyLogType = @inyLogType
		, @intServerType = @intServerType
		, @intWorldID = @intWorldID
		, @intAccountID = @intAccountID
		, @inbCharacterID = @inbCharacterID
		, @intSessionID = @intSessionID
		, @dt2StartLogDate = @dt2StartLogDate
		, @dt2EndLogDate = @dt2EndLogDate
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

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
