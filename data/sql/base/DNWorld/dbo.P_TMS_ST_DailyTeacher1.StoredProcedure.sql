USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_ST_DailyTeacher1]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-10
description : (TIMOSE) ?? ?? SSIS Snapshot (dbo.ST_DailyTeacher1)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~3 = ???? by ??? at 2013-04-10
**/

CREATE PROCEDURE [dbo].[P_TMS_ST_DailyTeacher1]
	@dtmBasisDate datetime = NULL 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dtmBasisDate = CAST(ISNULL(@dtmBasisDate, GETDATE()) AS date);
	--?? ??
	IF OBJECT_ID('tempdb..#t1') IS NOT NULL DROP TABLE #t1;
	SELECT c.WorldID, cs.JobCode, cs.CharacterLevel
		, COUNT(*) AS TotalTeacherCharacterCount
		, SUM(t.GraduateCount) AS TotalGraduateCount
	INTO #t1
	FROM dbo.TeacherCharacters t WITH (NOLOCK)
	INNER JOIN dbo.Characters c WITH (NOLOCK) ON c.CharacterID = t.TeacherCharacterID
	INNER JOIN dbo.CharacterStatus cs WITH (NOLOCK) ON cs.CharacterID = c.CharacterID
	GROUP BY c.WorldID, cs.JobCode, cs.CharacterLevel
	OPTION (MAXDOP 1);

	IF OBJECT_ID('tempdb..#t2') IS NOT NULL DROP TABLE #t2;
	SELECT c.WorldID, cs.JobCode, cs.CharacterLevel
		, COUNT(*) AS TodayRegisterTeacherCharacterCount
	INTO #t2
	FROM dbo.TeacherCharacters  t WITH (NOLOCK)
	INNER JOIN dbo.Characters c WITH (NOLOCK) ON c.CharacterID = t.TeacherCharacterID
	INNER JOIN dbo.CharacterStatus cs WITH (NOLOCK) ON cs.CharacterID = c.CharacterID
	WHERE t.RegisterDate >= DATEADD(DD, -1, @dtmBasisDate) AND t.RegisterDate < @dtmBasisDate
	GROUP BY c.WorldID, cs.JobCode, cs.CharacterLevel
	OPTION (MAXDOP 1);

	SELECT DATEADD(DD, -1, @dtmBasisDate) AS BasisDate
		, COALESCE(t1.WorldID, t2.WorldID) AS WorldID
		, COALESCE(t1.JobCode, t2.JobCode) AS JobCode
		, COALESCE(t1.CharacterLevel, t2.CharacterLevel) AS CharacterLevel
		, t1.TotalTeacherCharacterCount, t1.TotalGraduateCount, t2.TodayRegisterTeacherCharacterCount
	FROM #t1 t1 FULL OUTER JOIN #t2 t2 ON t2.WorldID = t1.WorldID AND t2.JobCode = t1.JobCode AND t2.CharacterLevel = t1.CharacterLevel;
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
