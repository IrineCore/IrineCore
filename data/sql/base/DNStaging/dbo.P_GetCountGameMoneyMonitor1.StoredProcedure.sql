USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountGameMoneyMonitor1]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-08
description : 1? 10?? ?? ??? ??? ??? ????? ?? ????? ??????.
result set : RecordCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetCountGameMoneyMonitor1]
	  @dt2StartLogDate datetime2(0) = NULL --// ?? ?? ?? - ?? ??
	, @dt2EndLogDate datetime2(0) = NULL --// ?? ? ?? - ?? ??
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
	SET @intColumnCount = 2;

	SET @inbNullBitmap = 
		CASE WHEN @dt2StartLogDate IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @dt2EndLogDate IS NOT NULL THEN POWER(2, 1) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @nvcStmt = N'
	SELECT COUNT(*) AS RecordCount
	FROM dbo.Mon_GameMoney1 WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dt2StartLogDate', @inbNullBitmap, 1, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dt2EndLogDate', @inbNullBitmap, 2, @intColumnCount, '<=');

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @dt2StartLogDate datetime2(0)
			, @dt2EndLogDate datetime2(0)
		'
		, @dt2StartLogDate = @dt2StartLogDate
		, @dt2EndLogDate = @dt2EndLogDate;

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
