USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyPVP]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-11
description : ?? PVP ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ???? by ??? at 2013-04-11
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyPVP]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'PVPLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

DECLARE @nvcStmt2 nvarchar(max) = N''
	, @nvcDBName2 nvarchar(50) = N'DNStaging'
	, @nvcTblName2 nvarchar(50) = N'PVPEndLogs'
	, @nvcHint2 nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT2 nvarchar(max) = N'';


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @datBasisDate = CAST(ISNULL(@datBasisDate, GETDATE()) AS date);

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
		
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName2
		, @nvcTblName = @nvcTblName2
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint2
		, @nvcStmtOUT = @nvcStmtOUT2 OUTPUT;	
	
	SET @nvcStmt = N'
	/**_# [ST_DailyPVP] ???? INSERT???.*/
	INSERT dbo.ST_DailyPVP (BasisDate, WorldID, JobCode, CharacterLevel, PVPModeCode, MaxPlayerCount, CharacterCount, PVPPlayTime_SUM_SEC, WinCount, LoseCount, DrawCount, GiveupCount)
	SELECT CAST(a.PVPStartDate AS date) AS BasisDate, a.WorldID, dc.JobCode, dc.CharacterLevel, a.PVPModeCode, a.MaxPlayerCount
		, COUNT(*) AS CharacterCount
		, SUM(PVPPlayTime) AS PVPPlayTime_SUM_SEC
		, SUM(CASE WHEN PVPResultCode = 1 THEN 1 END) AS WinCount
		, SUM(CASE WHEN PVPResultCode = 2 THEN 1 END) AS LoseCount
		, SUM(CASE WHEN PVPResultCode = 3 THEN 1 END) AS DrawCount
		, SUM(CASE WHEN PVPResultCode = 4 THEN 1 END) AS GiveupCount
	FROM (' + @nvcStmtOUT + N') a
	INNER JOIN (' + @nvcStmtOUT2 + N') b ON b.PVPStartID = a.PVPStartID
	INNER JOIN dbo.DimCharacters dc WITH (NOLOCK) ON dc.CharacterID = b.CharacterID
	WHERE a.PVPStartDate >= @datBasisDate AND a.PVPStartDate < DATEADD(DD, 1, @datBasisDate)
	GROUP BY CAST(a.PVPStartDate AS date), a.WorldID, dc.CharacterLevel, a.PVPModeCode, a.MaxPlayerCount, dc.JobCode
	OPTION (MAXDOP 1);
	';
	EXEC sp_executesql @nvcStmt, N'@datBasisDate date', @datBasisDate = @datBasisDate;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
