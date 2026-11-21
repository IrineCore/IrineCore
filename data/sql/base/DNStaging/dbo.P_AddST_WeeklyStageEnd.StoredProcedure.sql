USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_WeeklyStageEnd]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-05-03
description : ?? ???? ??? ??? ?????. (??? : ???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-05-03
3 = ?? ??? ?? ?? ?? ?? by ??? at 2013-05-06
4 = ? ?????? ?? by ??? at 2013-05-13
**/

CREATE PROCEDURE [dbo].[P_AddST_WeeklyStageEnd]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'StageEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DD, -6, @datBasisDate), 112)
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
	IF DATEPART(DW, @datBasisDate) = 1 --//????? ??
	BEGIN
		/** ????? ??? ?? by ??? at 2012-02-07 **/
		EXEC dbo.P_MakeTableString
			  @nvcDBName = @nvcDBName
			, @nvcTblName = @nvcTblName
			, @nvcStartDate = @nvcStartDate
			, @nvcEndDate = @nvcEndDate
			, @nvcHint = @nvcHint
			, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
		
		SET @nvcStmt = N'
		/**_# [ST_WeeklyStageEnd] ???? INSERT???.*/
		IF OBJECT_ID(''tempdb..#StageEndLogs'') IS NOT NULL DROP TABLE #StageEndLogs;
		SELECT CharacterID, WorldID 
			, CASE WHEN SUM(ClearTime) <= 3600 THEN 1
				WHEN SUM(ClearTime) > 3600 AND SUM(ClearTime) <= 10800 THEN 2
				WHEN SUM(ClearTime) > 10800 AND SUM(ClearTime) <= 18000 THEN 2
				WHEN SUM(ClearTime) > 18000 THEN 3 END AS PlayTimeCode
		INTO #StageEndLogs
		FROM (' + @nvcStmtOUT + N') V 
		WHERE V.StageStartDate >= DATEADD(DD, -6, @datBasisDate) AND V.StageStartDate < DATEADD(DD, 1, @datBasisDate)
		GROUP BY CharacterID, WorldID
		OPTION (MAXDOP 1);

		CREATE CLUSTERED INDEX IX_StageEndLogs_CharacterID_WorldID ON #StageEndLogs (CharacterID, WorldID);
		INSERT dbo.ST_WeeklyStageEnd (BasisDate, WorldID, MapID, ClearCount, EntranceCount, CharacterCount, PlayTimeCode)
		SELECT @datBasisDate AS BasisDate, V.WorldID, V.MapID
			, SUM(CASE WHEN V.ClearFlag = 1 THEN 1 ELSE 0 END) AS ClearCount
			, COUNT(*) AS EntranceCount
			, COUNT(DISTINCT b.CharacterID) AS CharacterCount
			, b.PlayTimeCode
		FROM (' + @nvcStmtOUT + N') V 
			INNER JOIN #StageEndLogs b ON b.CharacterID = V.CharacterID AND b.WorldID = V.WorldID
		WHERE V.StageStartDate >= DATEADD(DD, -6, @datBasisDate) AND V.StageStartDate < DATEADD(DD, 1, @datBasisDate)
		GROUP BY V.WorldID, V.MapID, b.PlayTimeCode
		OPTION (MAXDOP 1);

		DROP TABLE #StageEndLogs;
		';
		EXEC sp_executesql @nvcStmt, N'@datBasisDate date', @datBasisDate = @datBasisDate;
	END
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
