USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPointLogs]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-18

description :
	??? ?? ??
result set : 
	PointCode		tinyint
	LogDate			datetime2(0)
	PointBefore		int
	PointAfter		int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	3 = ?? ??
	5~6 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
	7 = ?? ??? ?? PL.LogDate >= @dt2LogDateTo --> PL.LogDate < @dt2LogDateTo by ??? at 2012-02-09
	8 = ?? ?? ?? by ??? at 2013-02-14
**/

CREATE PROCEDURE [dbo].[P_GetListPointLogs]
	  @dt2LogDateFrom datetime2(0) --// ?? ???
	, @dt2LogDateTo datetime2(0) --// ?? ???
	, @inbCharacterID bigint --// ??? ID
	, @inyPointCode tinyint = NULL --// 1=PVP ?? ???, 2=?? ?? ???, 3=?? ?? ???, 4=?? ?? ???, 5=??? ?? ???, 6=??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'PointLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2LogDateFrom, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2LogDateTo, 112)	
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
	SELECT PL.PointCode
		 , PL.LogDate
		 , PL.PointBefore
		 , PL.PointAfter
	  FROM (' + @nvcStmtOUT + N') PL
	 WHERE (PL.LogDate >= @dt2LogDateFrom)
	   AND (PL.LogDate < @dt2LogDateTo)
	   AND (PL.CharacterID = @inbCharacterID)
	   AND (@inyPointCode IS NULL OR PL.PointCode = @inyPointCode)
	 ORDER BY PL.PointCode, PL.LogDate
	';
	EXEC sp_executesql @nvcStmt, N'
		  @dt2LogDateFrom datetime2(0)
		, @dt2LogDateTo datetime2(0)
		, @inbCharacterID bigint
		, @inyPointCode tinyint'
		, @dt2LogDateFrom = @dt2LogDateFrom
		, @dt2LogDateTo = @dt2LogDateTo
		, @inbCharacterID = @inbCharacterID
		, @inyPointCode = @inyPointCode;
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
