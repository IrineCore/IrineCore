USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountEvent]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ??? ??? ????? ????? ??????.
result set : RecordCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetCountEvent]
	  @nvcEventName nvarchar(50) = NULL --// ??? ??
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
	SET @intColumnCount = 1;

	SET @inbNullBitmap = 
		CASE WHEN @nvcEventName IS NOT NULL THEN POWER(2, 0) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @nvcStmt = N'
SELECT COUNT(*) AS RecordCount
FROM dbo.Events WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'EventName', N'@nvcEventName', @inbNullBitmap, 1, @intColumnCount, '%A%') + N'
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcEventName nvarchar(50)
		'
		, @nvcEventName = @nvcEventName;
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
