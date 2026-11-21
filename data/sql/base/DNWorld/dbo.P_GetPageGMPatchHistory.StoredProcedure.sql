USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGMPatchHistory]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-12
description : ?? ??? ????? ?????? ?????.
result set : 
	@intSelectClauseCode = 1 : PatchVersion, PatchDate, GMID, PatchMemo
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102109 = ?? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPageGMPatchHistory]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @intSelectClauseCode int --// ?? ?? ??.
	, @intGMID int = NULL --// ??? ID
	, @dtmStartPatchDate datetime = NULL --// ?? ?? ?? - ?? ??
	, @dtmEndPatchDate datetime = NULL --// ?? ? ?? - ?? ??
WITH EXECUTE AS 'SPExecutor'
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
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'PatchVersion, PatchDate, GMID, PatchMemo'
		END;

	SET @intColumnCount = 3;

	SET @inbNullBitmap = 
		CASE WHEN @intGMID IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @dtmStartPatchDate IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @dtmEndPatchDate IS NOT NULL THEN POWER(2, 2) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);
	SET @dtmEndPatchDate = @dtmEndPatchDate + 1;
	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY PatchDate DESC) AS RowNumber, ' + @nvcSelectStmt + N'
	FROM dbo.GMPatchHistory WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'GMID', N'@intGMID', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PatchDate', N'@dtmStartPatchDate', @inbNullBitmap, 2, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'PatchDate', N'@dtmEndPatchDate', @inbNullBitmap, 3, @intColumnCount, '<') + N'
)
SELECT ' + @nvcSelectStmt + N'
FROM PagingCTE
WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY RowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @intGMID int
			, @dtmStartPatchDate datetime
			, @dtmEndPatchDate datetime
			, @intPageNumber int
			, @intPageSize int
		'
		, @intGMID = @intGMID
		, @dtmStartPatchDate = @dtmStartPatchDate
		, @dtmEndPatchDate = @dtmEndPatchDate
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, 102109? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 102109;
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
