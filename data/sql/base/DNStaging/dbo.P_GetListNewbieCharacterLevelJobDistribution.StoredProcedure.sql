USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListNewbieCharacterLevelJobDistribution]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-11
description : ?? ??? ??/?? ??? ?????.
result set : CharacterLevel, Job1, Job2, Job3, Job4, Job5, Job6, Job7, Job11, Job12, Job13, Job14, Job15, Job16, Job17, Job18, Job19, Job20, Job21, Job22, Job46, Job49, Job54, Job57, Job62, Job67
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ??? ???? ?? ??     by 20110809.wgkim
3 = ?? ??? ???? ?? ??     by 20110809.wgkim
4 = ???? 1? ?? ?? ??		   by 20110914.ohtack
9 = ??? ?? by 2013-07-25 at ???
**/

CREATE PROCEDURE [dbo].[P_GetListNewbieCharacterLevelJobDistribution]
	  @datBasisDate date --// ?? ??
	, @intWorldID int --// (optional) ?? ID. NULL ??? ?? ?? ?? ??? ?????.
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
	SELECT CharacterLevel
		, ISNULL(SUM(CASE JobCode WHEN 1 THEN CharacterCount END), 0) AS Job1
		, ISNULL(SUM(CASE JobCode WHEN 2 THEN CharacterCount END), 0) AS Job2
		, ISNULL(SUM(CASE JobCode WHEN 3 THEN CharacterCount END), 0) AS Job3
		, ISNULL(SUM(CASE JobCode WHEN 4 THEN CharacterCount END), 0) AS Job4
		, ISNULL(SUM(CASE JobCode WHEN 5 THEN CharacterCount END), 0) AS Job5
		, ISNULL(SUM(CASE JobCode WHEN 6 THEN CharacterCount END), 0) AS Job6
		, ISNULL(SUM(CASE JobCode WHEN 7 THEN CharacterCount END), 0) AS Job7
		, ISNULL(SUM(CASE JobCode WHEN 11 THEN CharacterCount END), 0) AS Job11
		, ISNULL(SUM(CASE JobCode WHEN 12 THEN CharacterCount END), 0) AS Job12
		, ISNULL(SUM(CASE JobCode WHEN 13 THEN CharacterCount END), 0) AS Job13
		, ISNULL(SUM(CASE JobCode WHEN 14 THEN CharacterCount END), 0) AS Job14
		, ISNULL(SUM(CASE JobCode WHEN 15 THEN CharacterCount END), 0) AS Job15
		, ISNULL(SUM(CASE JobCode WHEN 16 THEN CharacterCount END), 0) AS Job16
		, ISNULL(SUM(CASE JobCode WHEN 17 THEN CharacterCount END), 0) AS Job17
		, ISNULL(SUM(CASE JobCode WHEN 18 THEN CharacterCount END), 0) AS Job18
		, ISNULL(SUM(CASE JobCode WHEN 19 THEN CharacterCount END), 0) AS Job19
		, ISNULL(SUM(CASE JobCode WHEN 20 THEN CharacterCount END), 0) AS Job20
		, ISNULL(SUM(CASE JobCode WHEN 21 THEN CharacterCount END), 0) AS Job21
		, ISNULL(SUM(CASE JobCode WHEN 22 THEN CharacterCount END), 0) AS Job22
		, ISNULL(SUM(CASE JobCode WHEN 46 THEN CharacterCount END), 0) AS Job46
		, ISNULL(SUM(CASE JobCode WHEN 49 THEN CharacterCount END), 0) AS Job49
		, ISNULL(SUM(CASE JobCode WHEN 54 THEN CharacterCount END), 0) AS Job54
		, ISNULL(SUM(CASE JobCode WHEN 57 THEN CharacterCount END), 0) AS Job57
		, ISNULL(SUM(CASE JobCode WHEN 62 THEN CharacterCount END), 0) AS Job62
		, ISNULL(SUM(CASE JobCode WHEN 67 THEN CharacterCount END), 0) AS Job67
	FROM dbo.ST_JobAndLevelType4
	WHERE BasisDate = @datBasisDate AND WorldID = COALESCE(@intWorldID, WorldID)
	GROUP BY CharacterLevel
	ORDER BY CharacterLevel;
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
