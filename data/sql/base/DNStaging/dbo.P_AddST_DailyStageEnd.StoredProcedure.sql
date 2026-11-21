USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyStageEnd]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-10
description : ?? ???? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-04-10
5 = ??? ?? by ??? at 2013-06-28
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyStageEnd]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'StageEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

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
	
	SET @nvcStmt = N'
	/**_# [ST_DailyStageEnd] ???? INSERT???.*/
	INSERT dbo.ST_DailyStageEnd (BasisDate, WorldID, JobCode, CharacterLevel, PartySize, MapID, ClearTime_SUM_SEC, ClearTime_AVG_SEC, ClearTime_MIN_SEC, ClearTime_MAX_SEC, ClearCount, EntranceCount, ClearPoint_AVG, DifficultyCode, ClearPoint_TOTAL)
	SELECT CAST(StageStartDate AS date) AS BasisDate, WorldID, JobCode, CharacterLevel, PartySize, MapID
		, SUM(ClearTime) AS ClearTime_SUM_SEC
		, AVG(ClearTime) AS ClearTime_AVG_SEC
		, MIN(ClearTime) AS ClearTime_MIN_SEC
		, MAX(ClearTime) AS ClearTime_MAX_SEC
		, SUM(CASE WHEN ClearFlag = 1 THEN 1 ELSE 0 END) AS ClearCount
		, COUNT(*) AS EntranceCount
		, AVG(ClearPoint) AS ClearPoint_AVG
		, DifficultyCode
		, SUM(ClearPoint) AS ClearPoint_TOTAL
	FROM (' + @nvcStmtOUT + N') V 
	WHERE V.StageStartDate >= @datBasisDate AND V.StageStartDate < DATEADD(DD, 1, @datBasisDate)
	GROUP BY CAST(StageStartDate AS date), WorldID, JobCode, CharacterLevel, PartySize, MapID, DifficultyCode
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
