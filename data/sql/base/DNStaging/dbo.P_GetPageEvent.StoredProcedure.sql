USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageEvent]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ??? ??? ????? ?????? ?????.
result set : EventID, EventName, EventMemo, EventStartDate, EventEndDate, RegisterDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104102 = ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPageEvent]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @nvcEventName nvarchar(50) = NULL --// ??? ??
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
	SET @nvcSelectStmt = N'EventID, EventName, EventMemo, EventStartDate, EventEndDate, RegisterDate'

	SET @intColumnCount = 1;

	SET @inbNullBitmap = 
		CASE WHEN @nvcEventName IS NOT NULL THEN POWER(2, 0) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY EventID DESC) AS rowNumber, ' + @nvcSelectStmt + N'
	FROM dbo.Events WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'EventName', N'@nvcEventName', @inbNullBitmap, 1, @intColumnCount, '%A%') + N'
)
SELECT ' + @nvcSelectStmt + N'
FROM PagingCTE
WHERE rowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY rowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcEventName nvarchar(50)
			, @intPageNumber int
			, @intPageSize int
		'
		, @nvcEventName = @nvcEventName
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, 104102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 104102;
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
