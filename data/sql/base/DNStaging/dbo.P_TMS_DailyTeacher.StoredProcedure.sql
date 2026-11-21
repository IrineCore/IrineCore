USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_DailyTeacher]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-11
description : (TIMOSE) ?? ??/?? ?? (dbo.ST_DailyTeacher)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ???? by ??? at 2013-04-11
**/

CREATE PROCEDURE [dbo].[P_TMS_DailyTeacher]
	@datBasisDate date
WITH EXECUTE AS 'SPExecutor'
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
	SELECT COALESCE(a.BasisDate, b.BasisDate) AS BasisDate
		, COALESCE(a.WorldID, b.WorldID) AS WorldID
		, COALESCE(a.JobCode, b.JobCode) AS JobCode
		, COALESCE(a.CharacterLevel, b.CharacterLevel) AS CharacterLevel
		, a.TotalTeacherCharacterCount
		, a.TotalGraduateCount
		, a.TodayRegisterTeacherCharacterCount
		, b.TodayGraduateCount
		, b.TodayTeacherAndPupilCount
		, NULL AS PublisherID, NULL AS NationalityCode, NULL AS PartitionID
	FROM dbo.ST_DailyTeacher1 a WITH (NOLOCK)
	FULL OUTER JOIN dbo.ST_DailyTeacher2 b WITH (NOLOCK) ON b.BasisDate = a.BasisDate AND b.WorldID = a.WorldID AND b.JobCode = a.JobCode AND b.CharacterLevel = a.CharacterLevel
	WHERE a.BasisDate >= @datBasisDate AND a.BasisDate < DATEADD(DD, 1, @datBasisDate)
	OPTION (MAXDOP 1); 
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
