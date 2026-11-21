USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-02
description : PVPLog? ?????.
Result Set : PVPModeCode, PlayCount, KillCount, DeathCount, WinCount, LoseCount, DrawCount, GiveUpCount, OccupationCount, OccupationTimeLimitWinFlag
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPVPLog]
	  @inbCharacterID bigint --// ??? ID
	, @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @datBasisDate date = GETDATE() - 1;
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
SELECT PL.PVPModeCode 
     , COUNT(*) AS PlayCount
     , ISNULL(SUM(PEL.vsWarriorKOWin), 0) + ISNULL(SUM(PEL.vsArcherKOWin), 0) + ISNULL(SUM(PEL.vsSorceressKOWin), 0) + ISNULL(SUM(PEL.vsClericKOWin), 0) + ISNULL(SUM(PEL.vsAcademicKOWin), 0) + ISNULL(SUM(PEL.vsKaliKOWin), 0) AS KillCount
     , ISNULL(SUM(PEL.vsWarriorKOLose), 0) + ISNULL(SUM(PEL.vsArcherKOLose), 0) + ISNULL(SUM(PEL.vsSorceressKOLose), 0) + ISNULL(SUM(PEL.vsClericKOLose), 0) + ISNULL(SUM(PEL.vsAcademicKOLose), 0) + ISNULL(SUM(PEL.vsKaliKOLose), 0) AS DeathCount
     , ISNULL(SUM(CASE WHEN PEL.PVPResultCode = 1 THEN 1 END), 0) AS WinCount
     , ISNULL(SUM(CASE WHEN PEL.PVPResultCode = 2 THEN 1 END), 0) AS LoseCount
     , ISNULL(SUM(CASE WHEN PEL.PVPResultCode = 3 THEN 1 END), 0) AS DrawCount
     , ISNULL(SUM(CASE WHEN PEL.PVPResultCode = 4 THEN 1 END), 0) AS GiveUpCount
     , ISNULL(SUM(PEL.OccupationCount), 0) AS OccupationCount
     , ISNULL(SUM(CAST(PEL.OccupationTimeLimitWinFlag AS int)), 0) AS OccupationTimeLimitWinFlag
FROM (' + @nvcStmtOUT1 + N') PL
	INNER JOIN (' + @nvcStmtOUT2 + N') PEL ON PL.PVPStartID = PEL.PVPStartID
WHERE PL.PVPStartDate >= @dt2StartDate AND PL.PVPStartDate <= @dt2EndDate
  AND PEL.CharacterID = @inbCharacterID
GROUP BY PEL.CharacterID, PL.PVPModeCode;';

	EXEC sp_executesql @nvcStmt, N'
	  @dt2StartDate datetime2(0), @dt2EndDate datetime2(0), @inbCharacterID bigint'
	, @dt2StartDate = @dt2StartDate, @dt2EndDate = @dt2EndDate, @inbCharacterID= @inbCharacterID


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
