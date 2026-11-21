USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_GetListProcedureExecStats]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-03
result set :
description : (Timose) ???? DMV ?? ??
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_TMS_GetListProcedureExecStats]
	  @dtBasisDate date --// ?? ??
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
	SELECT A.BasisDate, A.ServerName, A.DatabaseName, A.ProcedureName
		, CASE WHEN B.CachedTime <= A.CachedTime THEN (B.ExecutionCount - A.ExecutionCount) ELSE B.ExecutionCount END AS ExecutionCount
		, CASE WHEN B.CachedTime <= A.CachedTime 
			THEN (B.TotalElapsedTime - A.TotalElapsedTime)/CASE WHEN (B.ExecutionCount - A.ExecutionCount) = 0 THEN 1 ELSE (B.ExecutionCount - A.ExecutionCount) END 
			ELSE B.TotalElapsedTime/CASE WHEN B.ExecutionCount = 0 THEN 1 ELSE B.ExecutionCount END 
		  END AS AvgElapsedTime
		, CASE WHEN B.CachedTime <= A.CachedTime 
			THEN (B.TotalWorkerTime - A.TotalWorkerTime)/CASE WHEN (B.ExecutionCount - A.ExecutionCount) = 0 THEN 1 ELSE (B.ExecutionCount - A.ExecutionCount) END 
			ELSE B.TotalWorkerTime/CASE WHEN B.ExecutionCount = 0 THEN 1 ELSE B.ExecutionCount END 
		  END AS AvgWorkerTime
		, CASE WHEN B.CachedTime <= A.CachedTime 
			THEN (B.TotalLogicalReads - A.TotalLogicalReads)/CASE WHEN (B.ExecutionCount - A.ExecutionCount) = 0 THEN 1 ELSE (B.ExecutionCount - A.ExecutionCount) END 
			ELSE B.TotalLogicalReads/CASE WHEN B.ExecutionCount = 0 THEN 1 ELSE B.ExecutionCount END 
		  END AS AvgLogicalReads
		, CASE WHEN B.CachedTime <= A.CachedTime 
			THEN (B.TotalLogicalWrites - A.TotalLogicalWrites)/CASE WHEN (B.ExecutionCount - A.ExecutionCount) = 0 THEN 1 ELSE (B.ExecutionCount - A.ExecutionCount) END 
			ELSE B.TotalLogicalWrites/CASE WHEN B.ExecutionCount = 0 THEN 1 ELSE B.ExecutionCount END 
		  END AS AvgLogicalWrites
		, B.QueryPlan
		, NULL AS PublisherID
	    , NULL AS NationalityCode
	    , NULL AS PartitionID
	    , NULL AS WorldID
	FROM (SELECT * FROM dbo.ProcedureExecStats WITH(NOLOCK) WHERE BasisDate = @dtBasisDate) A
		CROSS JOIN (SELECT * FROM dbo.ProcedureExecStats WITH(NOLOCK) WHERE BasisDate = DATEADD(DAY, 1, @dtBasisDate)) B
	WHERE A.BasisDate = DATEADD(DAY, -1, B.BasisDate) 
	  AND A.ServerName = B.ServerName 
	  AND A.DatabaseName = B.DatabaseName
	  AND A.ProcedureName = B.ProcedureName;

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
