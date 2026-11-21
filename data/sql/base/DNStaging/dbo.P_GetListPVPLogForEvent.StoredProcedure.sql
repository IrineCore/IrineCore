USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPLogForEvent]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-27
description : PVP???? PVP ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPVPLogForEvent]
	  @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ?? ??
	, @inyPVPModeCode tinyint --// 1=???, 2=???, 3=??, 4=???, 5=??, 6=1:1 PVP??, 7=2:2 PVP??, 8=3:3 PVP??, 9=4:4 PVP??, 10=???, 11=???, 12=???
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName1 nvarchar(50) = N'PVPLogs'
	, @nvcTblName2 nvarchar(50) = N'PVPEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112)		
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT1 nvarchar(max) = N''
	, @nvcStmtOUT2 nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ??**/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName1
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT1 OUTPUT;	
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName2
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT2 OUTPUT;	

	SET @nvcStmt = N'
	SELECT DISTINCT B.AccountName, A.CharacterID, A.CharacterName, C.JobName, D.JobName AS CharacterClassName 
	FROM dbo.DimCharacters A WITH(NOLOCK) 
		INNER JOIN dbo.DimAccounts B WITH(NOLOCK) ON A.AccountID = B.AccountID
		INNER JOIN dbo.DimJobs C WITH(NOLOCK) ON A.JobCode = C.JobCode
		INNER JOIN dbo.DimJobs D WITH(NOLOCK) ON A.CharacterClassCode = D.JobCode
		INNER JOIN (' + @nvcStmtOUT2 + N') PEL ON A.CharacterID = PEL.CharacterID
		INNER JOIN (' + @nvcStmtOUT1 + N') PL ON PL.PVPStartID = PEL.PVPStartID
	WHERE A.DeleteFlag = 0
	  AND PL.PVPModeCode = @inyPVPModeCode;'

	EXEC sp_executesql @nvcStmt, N'@inyPVPModeCode tinyint', @inyPVPModeCode = @inyPVPModeCode;

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
