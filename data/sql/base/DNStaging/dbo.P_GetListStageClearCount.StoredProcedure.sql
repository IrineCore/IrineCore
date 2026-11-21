USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListStageClearCount]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-04-01

description :
	(DOLIS) ????? ??? ?? ??
result set : 
	Ranking			int
	StageName		nvarchar(50)
	DifficultyCode	tinyint
	ClearCount		int
	CharacterName	nvarchar(30)
	CharacterLevel	tinyint
	JobCode			tinyint
	PartySize		tinyint
	WorldID			int
	AccountID		int
	CharacterID		bigint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1 = ?? ??
	2~3 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
	4 = StageID ???? ?? ? ???? ?? ? ? ??? ?? by ??? at 2012-05-15
	5~6 = WorldID ??? DimCharacters ???? ?? by ??? at 2012-05-15
**/

CREATE PROCEDURE [dbo].[P_GetListStageClearCount]
	  @dt2StartDate		datetime2(0)	--// ???
	, @dt2EndDate		datetime2(0)	--// ???
	, @intStageID		int				--// ????ID (0=????)
	, @intClearCount	int				--// ??? ??
	, @intRanking		int				--// ?? (0=????)
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'StageEndLogs'
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
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
	/**_# ????? ??? ??? ?????. */
	SET @nvcStmt = N'
	SELECT ROW_NUMBER() OVER(PARTITION BY DS.StageName, SEL.DifficultyCode ORDER BY COUNT(*) DESC) AS Ranking
		 , DS.StageName
		 , SEL.DifficultyCode	-- 10=SSS, 11=SS, 12=S, 13=A, 14=B, 15=C, 16=D
		 , COUNT(*) AS ClearCount
		 , DC.CharacterName
		 , DC.CharacterLevel
		 , DC.JobCode
		 , SEL.PartySize
		 , DC.WorldID
		 , DC.AccountID
		 , DC.CharacterID
	  INTO #Temp_Result
	  FROM (' + @nvcStmtOUT + N') SEL 
		   INNER JOIN dbo.DimMaps DM WITH (NOLOCK) ON (SEL.MapID = DM.MapID)
		   INNER JOIN dbo.DimStages DS WITH (NOLOCK) ON (DM.StageID = DS.StageID)
		   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (SEL.CharacterID = DC.CharacterID)
	 WHERE (SEL.ClearFlag = 1)
	   AND (SEL.StageEndDate >= @dt2StartDate AND SEL.StageEndDate <= @dt2EndDate)
       AND DM.StageID = @intStageID
	 GROUP BY DS.StageName, SEL.DifficultyCode, DC.CharacterName, DC.CharacterLevel, DC.JobCode, SEL.PartySize, DC.WorldID, DC.AccountID, DC.CharacterID
	HAVING COUNT(*) >= @intClearCount;

	SELECT Ranking
		 , StageName
		 , DifficultyCode
		 , ClearCount
		 , CharacterName
		 , CharacterLevel
		 , JobCode
		 , PartySize
		 , WorldID
		 , AccountID
		 , CharacterID
	  FROM #Temp_Result
	 WHERE (@intRanking = 0 OR Ranking <= @intRanking)
	 ORDER BY StageName, DifficultyCode, Ranking, CharacterName;

	DROP TABLE #Temp_Result;
	';
	EXEC sp_executesql @nvcStmt, N'
		  @dt2StartDate datetime2(0)
		, @dt2EndDate datetime2(0)
		, @intStageID int			
		, @intClearCount int			
		, @intRanking int'
		, @dt2StartDate = @dt2StartDate
		, @dt2EndDate = @dt2EndDate
		, @intStageID = @intStageID
		, @intClearCount = @intClearCount
		, @intRanking = @intRanking;		
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
