USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListNestClearRankTime]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-14
description : ??? ??? ?? ??? ?????.
result set : TotalRanking/int, ClearTime/int, CharacterID1/bigint, CharacterName1/nvarchar(50), CharacterLevel1/tinyint, JobCode1/tinyint, CharacterID2/bigint, CharacterName2/nvarchar(50), CharacterLevel2/tinyint, JobCode2/tinyint, CharacterID3/bigint, CharacterName3/nvarchar(50), CharacterLevel3/tinyint, JobCode3/tinyint, CharacterID4/bigint, CharacterName4/nvarchar(50), CharacterLevel4/tinyint, JobCode4/tinyint, CharacterID5/bigint, CharacterName5/nvarchar(50), CharacterLevel5/tinyint, JobCode5/tinyint, CharacterID6/bigint, CharacterName6/nvarchar(50), CharacterLevel6/tinyint, JobCode6/tinyint, CharacterID7/bigint, CharacterName7/nvarchar(50), CharacterLevel7/tinyint, JobCode7/tinyint, CharacterID8/bigint, CharacterName8/nvarchar(50), CharacterLevel8/tinyint, JobCode8/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListNestClearRankTime]
		@dt2StartDate datetime2(0) -- ?? ?? ??
      , @dt2EndDate datetime2(0) -- ?? ?? ??
      , @intMapID int -- ?ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

SET DATEFIRST 7;

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
EXEC dbo.P_MakeTableString
	  @nvcDBName = @nvcDBName
	, @nvcTblName = @nvcTblName
	, @nvcStartDate = @nvcStartDate
	, @nvcEndDate = @nvcEndDate
	, @nvcHint = @nvcHint
	, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	

	SET @nvcStmt = N'
	WITH C1 AS
	(
		SELECT ROW_NUMBER() OVER(PARTITION BY StageStartID ORDER BY V.CharacterID) AS Seq, V.StageStartID, DS.StageID, V.CharacterID, DC.CharacterName, V.CharacterLevel, V.JobCode, V.PartySize, V.ClearTime
		FROM (' + @nvcStmtOUT + N') V
			INNER JOIN dbo.DimMaps DM WITH(NOLOCK) ON V.MapID = DM.MapID
			INNER JOIN dbo.DimStages DS WITH(NOLOCK) ON DM.StageID = DS.StageID
			INNER JOIN dbo.DimCharacters DC WITH(NOLOCK) ON V.CharacterID = DC.CharacterID
		WHERE V.StageStartDate BETWEEN @dt2StartDate AND @dt2EndDate
          AND V.MapID = @intMapID
          AND V.ClearFlag = CAST(1 AS bit) 
		  AND DS.NestFlag = CAST(1 AS bit)  
	), 
	C2 AS(
		SELECT A.StageStartID, StageID, ClearTime
			, MAX(CASE WHEN Seq = 1 THEN CharacterID END) AS CharacterID1
			, MAX(CASE WHEN Seq = 1 THEN CharacterName END) AS CharacterName1
			, MAX(CASE WHEN Seq = 1 THEN CharacterLevel END) AS CharacterLevel1
			, MAX(CASE WHEN Seq = 1 THEN JobCode END) AS JobCode1
			, MAX(CASE WHEN Seq = 2 THEN CharacterID END) AS CharacterID2
			, MAX(CASE WHEN Seq = 2 THEN CharacterName END) AS CharacterName2
			, MAX(CASE WHEN Seq = 2 THEN CharacterLevel END) AS CharacterLevel2
			, MAX(CASE WHEN Seq = 2 THEN JobCode END) AS JobCode2
			, MAX(CASE WHEN Seq = 3 THEN CharacterID END) AS CharacterID3
			, MAX(CASE WHEN Seq = 3 THEN CharacterName END) AS CharacterName3
			, MAX(CASE WHEN Seq = 3 THEN CharacterLevel END) AS CharacterLevel3
			, MAX(CASE WHEN Seq = 3 THEN JobCode END) AS JobCode3
			, MAX(CASE WHEN Seq = 4 THEN CharacterID END) AS CharacterID4
			, MAX(CASE WHEN Seq = 4 THEN CharacterName END) AS CharacterName4
			, MAX(CASE WHEN Seq = 4 THEN CharacterLevel END) AS CharacterLevel4
			, MAX(CASE WHEN Seq = 4 THEN JobCode END) AS JobCode4
			, MAX(CASE WHEN Seq = 5 THEN CharacterID END) AS CharacterID5
			, MAX(CASE WHEN Seq = 5 THEN CharacterName END) AS CharacterName5
			, MAX(CASE WHEN Seq = 5 THEN CharacterLevel END) AS CharacterLevel5
			, MAX(CASE WHEN Seq = 5 THEN JobCode END) AS JobCode5
			, MAX(CASE WHEN Seq = 6 THEN CharacterID END) AS CharacterID6
			, MAX(CASE WHEN Seq = 6 THEN CharacterName END) AS CharacterName6
			, MAX(CASE WHEN Seq = 6 THEN CharacterLevel END) AS CharacterLevel6
			, MAX(CASE WHEN Seq = 6 THEN JobCode END) AS JobCode6
			, MAX(CASE WHEN Seq = 7 THEN CharacterID END) AS CharacterID7
			, MAX(CASE WHEN Seq = 7 THEN CharacterName END) AS CharacterName7
			, MAX(CASE WHEN Seq = 7 THEN CharacterLevel END) AS CharacterLevel7
			, MAX(CASE WHEN Seq = 7 THEN JobCode END) AS JobCode7
			, MAX(CASE WHEN Seq = 8 THEN CharacterID END) AS CharacterID8
			, MAX(CASE WHEN Seq = 8 THEN CharacterName END) AS CharacterName8
			, MAX(CASE WHEN Seq = 8 THEN CharacterLevel END) AS CharacterLevel8
			, MAX(CASE WHEN Seq = 8 THEN JobCode END) AS JobCode8
		FROM C1 A
		GROUP BY A.StageStartID, StageID, ClearTime
	)
	SELECT ROW_NUMBER() OVER(PARTITION BY StageID ORDER BY ClearTime, StageStartID) AS TotalRanking 
        , ClearTime
		, CharacterID1
		, CharacterName1
		, CharacterLevel1
		, JobCode1
		, CharacterID2
		, CharacterName2
		, CharacterLevel2
		, JobCode2
		, CharacterID3
		, CharacterName3
		, CharacterLevel3
		, JobCode3
		, CharacterID4
		, CharacterName4
		, CharacterLevel4
		, JobCode4
		, CharacterID5
		, CharacterName5
		, CharacterLevel5
		, JobCode5
		, CharacterID6
		, CharacterName6
		, CharacterLevel6
		, JobCode6
		, CharacterID7
		, CharacterName7
		, CharacterLevel7
		, JobCode7
		, CharacterID8
		, CharacterName8
		, CharacterLevel8
		, JobCode8
	FROM C2
	ORDER BY TotalRanking;'

	EXEC sp_executesql @nvcStmt, N'
		  @dt2StartDate datetime2(0)
		, @dt2EndDate datetime2(0)
		, @intMapID int'
		, @dt2StartDate = @dt2StartDate
		, @dt2EndDate = @dt2EndDate
		, @intMapID = @intMapID;
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
