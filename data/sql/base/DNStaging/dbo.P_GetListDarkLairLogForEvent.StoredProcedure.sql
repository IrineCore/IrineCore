USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDarkLairLogForEvent]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-27
description : ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDarkLairLogForEvent]
	  @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ?? ??
	, @intStageID int --// ??? ???? ID
	, @intPlayRound int = NULL --// ??? ??? ?
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intMapID int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'DarkLairLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ??? StageID? MapID? ????.**/
	SELECT @intMapID = MIN(MapID)
	  FROM dbo.DimMaps WITH (NOLOCK)
	 WHERE StageID = @intStageID
	 GROUP BY StageID;
	
	-- DOLIS?? ?? ??? ??? ?? ??? ? ? ??? ???? ?, ????? ext ??? ???? ????? ?.
	IF @intPlayRound IS NULL
	BEGIN
		IF @intMapID = 12007
			SET @intPlayRound = 24;
		ELSE IF @intMapID IN (12011, 1000012011)
			SET @intPlayRound = 20;
		ELSE IF @intMapID = 12021
			SET @intPlayRound = 35;
		ELSE
			SET @intPlayRound = 0;
	END

	/** ????? ??? ??**/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	

	SET @nvcStmt = N'
	SELECT DISTINCT B.AccountName, A.CharacterID, A.CharacterName, C.JobName, D.JobName AS CharacterClassName
	FROM dbo.DimCharacters A WITH(NOLOCK) 
		INNER JOIN dbo.DimAccounts B WITH(NOLOCK) ON A.AccountID = B.AccountID
		INNER JOIN dbo.DimJobs C WITH(NOLOCK) ON A.JobCode = C.JobCode
		INNER JOIN dbo.DimJobs D WITH(NOLOCK) ON A.CharacterClassCode = D.JobCode
		INNER JOIN (' + @nvcStmtOUT + N') DL ON A.CharacterID = DL.CharacterID1
	WHERE A.DeleteFlag = 0
	  AND DL.MapID = @intMapID
	  AND DL.PlayRound >= @intPlayRound
	UNION
	SELECT DISTINCT B.AccountName, A.CharacterID, A.CharacterName, C.JobName, D.JobName AS CharacterClassName  
	FROM dbo.DimCharacters A WITH(NOLOCK) 
		INNER JOIN dbo.DimAccounts B WITH(NOLOCK) ON A.AccountID = B.AccountID
		INNER JOIN dbo.DimJobs C WITH(NOLOCK) ON A.JobCode = C.JobCode
		INNER JOIN dbo.DimJobs D WITH(NOLOCK) ON A.CharacterClassCode = D.JobCode
		INNER JOIN (' + @nvcStmtOUT + N') DL ON A.CharacterID = DL.CharacterID2
	WHERE A.DeleteFlag = 0
	  AND DL.MapID = @intMapID
	  AND DL.PlayRound >= @intPlayRound
	UNION 
	SELECT DISTINCT B.AccountName, A.CharacterID, A.CharacterName, C.JobName, D.JobName AS CharacterClassName  
	FROM dbo.DimCharacters A WITH(NOLOCK) 
		INNER JOIN dbo.DimAccounts B WITH(NOLOCK) ON A.AccountID = B.AccountID
		INNER JOIN dbo.DimJobs C WITH(NOLOCK) ON A.JobCode = C.JobCode
		INNER JOIN dbo.DimJobs D WITH(NOLOCK) ON A.CharacterClassCode = D.JobCode
		INNER JOIN (' + @nvcStmtOUT + N') DL ON A.CharacterID = DL.CharacterID3
	WHERE A.DeleteFlag = 0
	  AND DL.MapID = @intMapID
	  AND DL.PlayRound >= @intPlayRound
	UNION 
	SELECT DISTINCT B.AccountName, A.CharacterID, A.CharacterName, C.JobName, D.JobName AS CharacterClassName  
	FROM dbo.DimCharacters A WITH(NOLOCK) 
		INNER JOIN dbo.DimAccounts B WITH(NOLOCK) ON A.AccountID = B.AccountID
		INNER JOIN dbo.DimJobs C WITH(NOLOCK) ON A.JobCode = C.JobCode
		INNER JOIN dbo.DimJobs D WITH(NOLOCK) ON A.CharacterClassCode = D.JobCode
		INNER JOIN (' + @nvcStmtOUT + N') DL ON A.CharacterID = DL.CharacterID4
	WHERE A.DeleteFlag = 0
	  AND DL.MapID = @intMapID
	  AND DL.PlayRound >= @intPlayRound;'

	EXEC sp_executesql @nvcStmt, N'@intMapID int, @intPlayRound int', @intMapID = @intMapID, @intPlayRound = @intPlayRound;

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
