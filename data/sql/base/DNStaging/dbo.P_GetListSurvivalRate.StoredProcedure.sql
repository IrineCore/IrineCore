USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListSurvivalRate]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-10-14
description : <??? ??? ?????? ?????.>
result set : ?? ??
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104105 = ??? ?? ??? ????.
**/
CREATE PROCEDURE [dbo].[P_GetListSurvivalRate]
	  @intMainCategory TINYINT --// 1:??, 2:??
	, @dtBasisDate DATE --// ???
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END


DECLARE @Query nvarchar(4000)
DECLARE @Param nvarchar(1000)

SET @Param = N''

BEGIN TRY
	SELECT @Param = @Param + N'[' + CAST(CharacterLevel AS NVARCHAR(3)) + N'],'
	FROM
	(SELECT DISTINCT CharacterLevel FROM dbo.ST_SurvivalRate WHERE MainCategory = @intMainCategory AND BasisDate = @dtBasisDate) a  ORDER BY CharacterLevel
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 104105;
		GOTO ErrorHandler;
	END
	
	SET @Param = LEFT(@Param, LEN(@Param) -1)

	SET @Query = N'
	SELECT * FROM (
		SELECT a.SubCategory, a.JobCode, a.CharacterLevel, a.Cnt
		FROM 
			dbo.ST_SurvivalRate a LEFT OUTER JOIN dbo.DimJobs b ON a.JobCode = b.JobCode
		WHERE 
			MainCategory = @intMainCategory AND BasisDate = @dtBasisDate) a
	PIVOT(SUM(Cnt) for CharacterLevel In (' + @Param + ')) as PVT
	ORDER BY SubCategory, JobCode'

	EXEC sp_executesql @Query, N'@intMainCategory TINYINT, @dtBasisDate DATE', @intMainCategory, @dtBasisDate
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
