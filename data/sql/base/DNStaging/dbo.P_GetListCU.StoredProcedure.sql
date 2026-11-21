USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCU]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-19
description : ?? ??? ?????.
result set : LogDate/datetime, WorldID/int, WorldName/nvarchar, CUCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4~6 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetListCU]
	  @dtmStartDate datetime --// ?? ?? ??
	, @dtmEndDate datetime --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CUCountLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtmStartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtmEndDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';
	
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
		
	SET @nvcStmt = N'
	SELECT L.LogDate, L.WorldID, W.WorldName, L.CUCount
	FROM (' + @nvcStmtOUT + N') L
		LEFT OUTER JOIN dbo.DimWorlds W ON L.WorldID = W.WorldID
	WHERE LogDate BETWEEN @dtmStartDate AND @dtmEndDate
	UNION ALL
	SELECT L.LogDate, NULL, NULL, SUM(L.CUCount)
	FROM dbo.V_CUCountLogs L WITH (NOLOCK)
	WHERE LogDate BETWEEN @dtmStartDate AND @dtmEndDate
	GROUP BY LogDate
	ORDER BY WorldID, LogDate;
	';
	EXEC sp_executesql @nvcStmt, N'@dtmStartDate datetime, @dtmEndDate datetime', @dtmStartDate = @dtmStartDate, @dtmEndDate = @dtmEndDate;
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
