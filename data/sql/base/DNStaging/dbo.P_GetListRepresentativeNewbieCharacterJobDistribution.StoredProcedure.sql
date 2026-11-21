USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListRepresentativeNewbieCharacterJobDistribution]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-10
description : ?? ??? ?? ??? ?? ??? ?????.
result set : JobCode, JobName, CharacterCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListRepresentativeNewbieCharacterJobDistribution]
	  @datBasisDate date --// ?? ??
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
	SELECT DJ.JobCode, DJ.JobName, SUM(CharacterCount) AS CharacterCount
	FROM dbo.ST_JobAndLevelType5 ST WITH (NOLOCK)
		INNER JOIN dbo.DimJobs DJ WITH (NOLOCK) ON ST.JobCode = DJ.JobCode
	WHERE ST.BasisDate = @datBasisDate
	GROUP BY DJ.JobCode, DJ.JobName
	ORDER BY DJ.JobCode;
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
