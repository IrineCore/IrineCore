USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyTeacher2]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-11
description : ?? ??? ??/???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-04-11
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyTeacher2]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'TeacherAndPupilLogs'
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
	/**_# [ST_DailyTeacher2] ???? INSERT???.*/
	WITH t1 AS
	(
		SELECT CAST(V.LogDate AS date) AS BasisDate, dc.WorldID, dc.JobCode, dc.CharacterLevel
			, SUM(CASE WHEN TeacherAndPupilLogCode = 4 THEN 1 ELSE 0 END) AS TodayGraduateCount
		FROM (' + @nvcStmtOUT + N') V
		INNER JOIN dbo.DimCharacters dc WITH (NOLOCK) ON dc.CharacterID = V.PupilCharacterID
		WHERE V.LogDate >= @datBasisDate AND V.LogDate < DATEADD(DD, 1, @datBasisDate)
		GROUP BY CAST(V.LogDate AS date), dc.WorldID, dc.JobCode, dc.CharacterLevel
	), t2 AS
	(
		SELECT CAST(V.LogDate AS date) AS BasisDate, dc.WorldID, dc.JobCode, dc.CharacterLevel
			, SUM(CASE WHEN TeacherAndPupilLogCode IN (1,5) THEN 1 ELSE 0 END) AS TodayTeacherAndPupilCount
		FROM (' + @nvcStmtOUT + N') V
		INNER JOIN dbo.DimCharacters dc WITH (NOLOCK) ON dc.CharacterID = V.TeacherCharacterID
		WHERE V.LogDate >= @datBasisDate AND V.LogDate < DATEADD(DD, 1, @datBasisDate)
		GROUP BY CAST(V.LogDate AS date), dc.WorldID, dc.JobCode, dc.CharacterLevel
	)
	INSERT dbo.ST_DailyTeacher2 (BasisDate, WorldID, JobCode, CharacterLevel, TodayGraduateCount, TodayTeacherAndPupilCount)
	SELECT COALESCE(t1.BasisDate, t2.BasisDate) AS BasisDate
		, COALESCE(t1.WorldID, t2.WorldID) AS WorldID
		, COALESCE(t1.JobCode, t2.JobCode) AS JobCode
		, COALESCE(t1.CharacterLevel, t2.CharacterLevel) AS CharacterLevel
		, t1.TodayGraduateCount, t2.TodayTeacherAndPupilCount
	FROM t1 FULL OUTER JOIN t2 ON t2.BasisDate = t1.BasisDate AND t2.WorldID = t1.WorldID AND t2.JobCode = t1.JobCode AND t2.CharacterLevel = t1.CharacterLevel 
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
