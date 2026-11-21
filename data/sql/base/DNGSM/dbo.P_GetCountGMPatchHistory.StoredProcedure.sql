USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountGMPatchHistory]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-12
description : ?? ??? ??????? ???? ??????.
result set : RecordCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetCountGMPatchHistory]
	  @intGMID int = NULL --// ??? ID
	, @dtmStartPatchDate datetime = NULL --// ?? ?? ?? - ?? ??
	, @dtmEndPatchDate datetime = NULL --// ?? ? ?? - ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intColumnCount = 3;

	SET @inbNullBitmap = 
		CASE WHEN @intGMID IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @dtmStartPatchDate IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @dtmEndPatchDate IS NOT NULL THEN POWER(2, 2) ELSE 0 END;

	/**_# ?? ??? ?????.*/
	SET @dtmEndPatchDate = @dtmEndPatchDate + 1;
	SET @nvcStmt = N'
SELECT COUNT(*) AS RecordCount
FROM dbo.GMPatchHistory WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'GMID', N'@intGMID', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PatchDate', N'@dtmStartPatchDate', @inbNullBitmap, 2, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'PatchDate', N'@dtmEndPatchDate', @inbNullBitmap, 3, @intColumnCount, '<');

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @intGMID int
			, @dtmStartPatchDate datetime
			, @dtmEndPatchDate datetime
		'
		, @intGMID = @intGMID
		, @dtmStartPatchDate = @dtmStartPatchDate
		, @dtmEndPatchDate = @dtmEndPatchDate;
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
