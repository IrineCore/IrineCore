USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAllVisitCharacterLevelJobDistribution]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-09
description : ?? ??? ??/?? ??? ?????.
result set : CharacterLevel, Job1, Job2, Job3, Job4, Job5, Job6, Job7, Job11, Job12, Job13, Job14, Job15, Job16, Job17, Job18, Job19, Job20, Job21, Job22, Job23, Job24, Job25, Job26, Job27, Job28, Job29, Job30, Job31, Job32, Job33, Job34, Job35, Job36, Job37, Job38, Job39, Job40, Job41, Job42, Job43, Job44, Job45, Job46, Job47, Job48, Job49, Job50, Job51, Job54, Job55, Job56, Job57, Job58, Job59, Job62, Job63, Job64, Job67, Job68, Job69
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ???? 2? ?? ?? ??		   by 20110914.ohtack
5 = ?? 2? ?? ?? by 2012-12-21 at ???
6 = ??? ?? by 2013-07-25 at ???
**/

CREATE PROCEDURE [dbo].[P_GetListAllVisitCharacterLevelJobDistribution]
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
		, ISNULL(SUM(CASE JobCode WHEN 23 THEN CharacterCount END), 0) AS Job23
		, ISNULL(SUM(CASE JobCode WHEN 24 THEN CharacterCount END), 0) AS Job24
		, ISNULL(SUM(CASE JobCode WHEN 25 THEN CharacterCount END), 0) AS Job25
		, ISNULL(SUM(CASE JobCode WHEN 26 THEN CharacterCount END), 0) AS Job26
		, ISNULL(SUM(CASE JobCode WHEN 27 THEN CharacterCount END), 0) AS Job27
		, ISNULL(SUM(CASE JobCode WHEN 28 THEN CharacterCount END), 0) AS Job28
		, ISNULL(SUM(CASE JobCode WHEN 29 THEN CharacterCount END), 0) AS Job29
		, ISNULL(SUM(CASE JobCode WHEN 30 THEN CharacterCount END), 0) AS Job30
		, ISNULL(SUM(CASE JobCode WHEN 31 THEN CharacterCount END), 0) AS Job31
		, ISNULL(SUM(CASE JobCode WHEN 32 THEN CharacterCount END), 0) AS Job32
		, ISNULL(SUM(CASE JobCode WHEN 33 THEN CharacterCount END), 0) AS Job33
		, ISNULL(SUM(CASE JobCode WHEN 34 THEN CharacterCount END), 0) AS Job34
		, ISNULL(SUM(CASE JobCode WHEN 35 THEN CharacterCount END), 0) AS Job35
		, ISNULL(SUM(CASE JobCode WHEN 36 THEN CharacterCount END), 0) AS Job36
		, ISNULL(SUM(CASE JobCode WHEN 37 THEN CharacterCount END), 0) AS Job37
		, ISNULL(SUM(CASE JobCode WHEN 38 THEN CharacterCount END), 0) AS Job38
		, ISNULL(SUM(CASE JobCode WHEN 39 THEN CharacterCount END), 0) AS Job39
		, ISNULL(SUM(CASE JobCode WHEN 40 THEN CharacterCount END), 0) AS Job40
		, ISNULL(SUM(CASE JobCode WHEN 41 THEN CharacterCount END), 0) AS Job41
		, ISNULL(SUM(CASE JobCode WHEN 42 THEN CharacterCount END), 0) AS Job42
		, ISNULL(SUM(CASE JobCode WHEN 43 THEN CharacterCount END), 0) AS Job43
		, ISNULL(SUM(CASE JobCode WHEN 44 THEN CharacterCount END), 0) AS Job44
		, ISNULL(SUM(CASE JobCode WHEN 45 THEN CharacterCount END), 0) AS Job45
		, ISNULL(SUM(CASE JobCode WHEN 46 THEN CharacterCount END), 0) AS Job46
		, ISNULL(SUM(CASE JobCode WHEN 47 THEN CharacterCount END), 0) AS Job47
		, ISNULL(SUM(CASE JobCode WHEN 48 THEN CharacterCount END), 0) AS Job48
		, ISNULL(SUM(CASE JobCode WHEN 49 THEN CharacterCount END), 0) AS Job49
		, ISNULL(SUM(CASE JobCode WHEN 50 THEN CharacterCount END), 0) AS Job50
		, ISNULL(SUM(CASE JobCode WHEN 51 THEN CharacterCount END), 0) AS Job51
		, ISNULL(SUM(CASE JobCode WHEN 54 THEN CharacterCount END), 0) AS Job54
		, ISNULL(SUM(CASE JobCode WHEN 55 THEN CharacterCount END), 0) AS Job55
		, ISNULL(SUM(CASE JobCode WHEN 56 THEN CharacterCount END), 0) AS Job56
		, ISNULL(SUM(CASE JobCode WHEN 57 THEN CharacterCount END), 0) AS Job57
		, ISNULL(SUM(CASE JobCode WHEN 58 THEN CharacterCount END), 0) AS Job58
		, ISNULL(SUM(CASE JobCode WHEN 59 THEN CharacterCount END), 0) AS Job59
		, ISNULL(SUM(CASE JobCode WHEN 62 THEN CharacterCount END), 0) AS Job62
		, ISNULL(SUM(CASE JobCode WHEN 63 THEN CharacterCount END), 0) AS Job63
		, ISNULL(SUM(CASE JobCode WHEN 64 THEN CharacterCount END), 0) AS Job64
		, ISNULL(SUM(CASE JobCode WHEN 67 THEN CharacterCount END), 0) AS Job67
		, ISNULL(SUM(CASE JobCode WHEN 68 THEN CharacterCount END), 0) AS Job68
		, ISNULL(SUM(CASE JobCode WHEN 69 THEN CharacterCount END), 0) AS Job69
	FROM dbo.ST_JobAndLevelType6
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
