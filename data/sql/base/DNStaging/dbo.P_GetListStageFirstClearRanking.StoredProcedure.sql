USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListStageFirstClearRanking]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-10-29
description : ?????? ???? ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListStageFirstClearRanking]
	  @dt2StageStartDate datetime2(0) --// ???? ?? ??
	, @dt2StageEndDate datetime2(0) --// ???? ?? ??
	, @intStageID int --// ???? ID
	, @vchDifficultyCode varchar(max) --// ??? ex('1,2,3') 1=??, 2=??, 3=???, 4=???, 5=???
	, @intRanking int --//????? ?? ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intMapID int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'StageEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StageStartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StageEndDate, 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

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
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
	SET @nvcStmt = N'
DECLARE @tblDifficultyCodes TABLE
(	
	DifficultyCode tinyint primary key
);

INSERT @tblDifficultyCodes (DifficultyCode)
SELECT CAST(string AS tinyint) FROM dbo.IF_Split(@vchDifficultyCode, '','') WHERE string > '''';

/**_# ? ???? ??? ??? ?????. */
WITH CTE AS(
	SELECT DENSE_RANK() OVER(PARTITION BY DC.WorldID ORDER BY V.StageEndDate) AS Ranking, V.CharacterID, DC.CharacterName, DW.WorldName, DC.CharacterClassCode, DC.JobCode, ClearTime, V.StageStartID 
	FROM (' + @nvcStmtOUT + N') V 
		 INNER JOIN @tblDifficultyCodes A ON (V.DifficultyCode = A.DifficultyCode)
		 INNER JOIN dbo.DimCharacters DC WITH(NOLOCK) ON (V.CharacterID = DC.CharacterID)
		 INNER JOIN dbo.DimWorlds DW WITH(NOLOCK) ON (DC.WorldID = DW.WorldID)
		 INNER JOIN dbo.DimMaps DM WITH(NOLOCK) ON (V.MapID = DM.MapID)
	WHERE V.StageStartDate >= @dt2StageStartDate AND V.StageStartDate < @dt2StageEndDate
	  AND DM.StageID = @intStageID
	  AND V.ClearFlag = 1
	  AND DC.DeleteFlag = 0
)
SELECT Ranking, CharacterID, CharacterName, ClearTime, WorldName, CharacterClassCode, JobCode, StageStartID
FROM CTE 
WHERE Ranking <= @intRanking
ORDER BY WorldName, Ranking ;
	'
	EXEC sp_executesql @nvcStmt, N'
		  @dt2StageStartDate datetime2(0)
		, @dt2StageEndDate datetime2(0)
		, @intStageID int	
		, @vchDifficultyCode varchar(max)
		, @intRanking int'
		, @dt2StageStartDate = @dt2StageStartDate
		, @dt2StageEndDate = @dt2StageEndDate
		, @intStageID = @intStageID
	    , @vchDifficultyCode = @vchDifficultyCode		
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
