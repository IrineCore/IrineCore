USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListProcedureStatForSnapshot]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-04-30
description : ???? dmv ??? ?????.(???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListProcedureStatForSnapshot]
	  @dtmBasisDate datetime
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

	SELECT CAST(@dtmBasisDate AS date) AS BasisDate
	     , @@SERVERNAME AS ServerName
	     , DB_NAME() AS DatabaseName
         , P.name AS ProcedureName
		 , B.cached_time AS CachedTime, ISNULL(B.execution_count, 0) AS ExecutionCount
		 , ISNULL(B.AvgElapsedTime, 0) AS AvgElapsedTime, ISNULL(B.AvgWorkerTime, 0) AS AvgWorkerTime, ISNULL(B.AvgLogicalReads, 0) AS AvgLogicalReads, ISNULL(B.AvgLogicalWrites, 0) AS AvgLogicalWrites
         , ISNULL(B.total_elapsed_time, 0) AS TotalElapsedTime, ISNULL(B.min_elapsed_time, 0) AS MinElapsedTime, ISNULL(B.max_elapsed_time, 0) AS MaxElapsedTime
		 , ISNULL(B.total_worker_time, 0) AS TotalWorkerTime, ISNULL(B.min_worker_time, 0) AS MinWorkerTime, ISNULL(B.max_worker_time, 0) AS MaxWorkerTime
		 , ISNULL(B.total_logical_reads, 0) AS TotalLogicalReads, ISNULL(B.min_logical_reads, 0) AS MinLogicalReads, ISNULL(B.max_logical_reads, 0) AS MaxLogicalReads
		 , ISNULL(B.total_logical_writes, 0) AS TotalLogicalWrites, ISNULL(B.min_logical_writes, 0) AS MinLogicalWrites, ISNULL(B.max_logical_writes, 0) AS MaxLogicalWrites
		 , B.QueryPlan
	FROM sys.procedures P WITH(NOLOCK)
		LEFT OUTER JOIN (
		SELECT QS.object_id
			 , CAST(QS.cached_time AS datetime2(0)) AS cached_time
			 , QS.execution_count
			 , QS.total_elapsed_time/QS.execution_count AS AvgElapsedTime
			 , QS.total_worker_time/QS.execution_count AS AvgWorkerTime
			 , QS.total_logical_reads/QS.execution_count AS AvgLogicalReads
			 , QS.total_logical_writes/QS.execution_count AS AvgLogicalWrites
			 , QS.total_elapsed_time
			 , QS.min_elapsed_time
			 , QS.max_elapsed_time
			 , QS.total_worker_time
			 , QS.min_worker_time
			 , QS.max_worker_time
			 , QS.total_logical_reads
			 , QS.min_logical_reads
			 , QS.max_logical_reads
			 , QS.total_logical_writes
			 , QS.min_logical_writes
			 , QS.max_logical_writes
			 , QP.query_plan AS QueryPlan
			 , ROW_NUMBER() OVER(PARTITION BY QS.object_id ORDER BY QS.execution_count DESC) AS Seq
		FROM sys.procedures A
			LEFT OUTER JOIN sys.dm_exec_procedure_stats QS WITH(NOLOCK) ON A.object_id = QS.object_id
			CROSS APPLY sys.dm_exec_query_plan(QS.plan_handle) AS QP 
		WHERE QS.database_id = DB_ID()
		  AND QS.execution_count >= 100
	) B ON P.object_id = B.object_id
	WHERE B.Seq IS NULL OR B.Seq = 1	

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
