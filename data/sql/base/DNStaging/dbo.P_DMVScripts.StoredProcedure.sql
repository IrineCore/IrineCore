USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_DMVScripts]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-07-26
description : Script ??
**/

CREATE PROCEDURE [dbo].[P_DMVScripts]
AS

SET NOCOUNT ON;

print N'

--====================================================================================================
-- ?? ???? ?? ??

select @@servername as [server_name]
	 , db_name(est.dbid) as [db_name]
	 , object_name(est.objectid) as [object_name]
	 , es.is_user_process as up
	 , es.session_id
	 , er.blocking_session_id as blk_id
	 , case when object_name(est.objectid) is null then ''txt : '' + est.text else ''sp : '' + object_name(est.objectid) end as [object_name]
	 , er.status as [req_status]
	 , er.wait_type
	 , er.cpu_time as [cpu(ms)]
	 , er.total_elapsed_time [tot_elpd(ms)]
	 , er.reads
	 , er.writes
	 , er.logical_reads
	 , es.host_name
	 , es.last_request_start_time
	 , es.last_request_end_time
	 , er.wait_time
	 , er.wait_resource
	 , er.open_transaction_count
	 , er.percent_complete
	 , es.login_name
	 , ec.client_net_address as [ip] -- ? ??? ???? ?????? ??? ?????.
	 , ec.client_tcp_port as [port] --? ??? ??? ????? ???? ?? ?????.
	 , es.lock_timeout
	 , er.command
	 , db_name(er.database_id) as [db]
	 , substring(est.text, er.statement_start_offset/2
	 , (case when er.statement_end_offset = -1 then len(convert(nvarchar(max), est.text)) * 2 else er.statement_end_offset end - er.statement_start_offset) / 2) as query_text
	 , es.program_name
	 , er.nest_level
	 , er.granted_query_memory
  from sys.dm_exec_sessions es
	   join sys.dm_exec_connections ec on es.session_id = ec.session_id
	   join sys.dm_exec_requests er on es.session_id = er.session_id
	   cross apply sys.dm_exec_sql_text(sql_handle) est
 order by blk_id desc, er.cpu_time desc, session_id asc;

--====================================================================================================
-- ?? SP ?? ??

DECLARE @plan_handle varbinary(max), @sp_name sysname;

SET @sp_name = ''''

-- ????
SELECT @plan_handle = plan_handle
  FROM sys.dm_exec_query_stats                      AS A   WITH (NOLOCK)
       OUTER APPLY sys.dm_exec_sql_text(sql_handle) AS B
 WHERE (B.objectid = OBJECT_ID(@sp_name));
SELECT query_plan FROM sys.dm_exec_query_plan(@plan_handle);

-- ??
SELECT @@SERVERNAME AS [server_name]
	 , db_name(QT.dbid) AS [db_name]
	 , OBJECT_NAME(QT.objectid) AS [object_name]
	 , QS.plan_generation_num
	 , QS.creation_time
	 , QS.last_execution_time
	 , QS.execution_count
	 , QS.total_worker_time / QS.execution_count AS avg_worker_time
	 , QS.total_worker_time
	 , QS.last_worker_time
	 , QS.min_worker_time
	 , QS.max_worker_time
	 , QS.total_physical_reads / QS.execution_count AS avg_physical_reads
	 , QS.total_physical_reads
	 , QS.last_physical_reads
	 , QS.min_physical_reads
	 , QS.max_physical_reads
	 , QS.total_logical_writes / QS.execution_count AS avg_logical_writes
	 , QS.total_logical_writes
	 , QS.last_logical_writes
	 , QS.min_logical_writes
	 , QS.max_logical_writes
	 , QS.total_logical_reads / QS.execution_count AS avg_logical_reads
	 , QS.total_logical_reads
	 , QS.last_logical_reads
	 , QS.min_logical_reads
	 , QS.max_logical_reads
	 , QS.total_elapsed_time / QS.execution_count AS avg_elapsed_time
	 , QS.total_elapsed_time
	 , QS.last_elapsed_time
	 , QS.min_elapsed_time
	 , QS.max_elapsed_time
	 , SUBSTRING(QT.text, (QS.statement_start_offset / 2) + 1, ((CASE QS.statement_end_offset WHEN -1 THEN DATALENGTH(QT.text) ELSE QS.statement_end_offset END - QS.statement_start_offset) / 2) + 1) AS query_text
	 , CONVERT(SMALLDATETIME, GETDATE()) AS RegisterDate
  FROM sys.dm_exec_query_stats QS
	   CROSS APPLY sys.dm_exec_sql_text(QS.sql_handle) QT
 WHERE OBJECT_NAME(QT.objectid) = @sp_name;

--====================================================================================================
-- ?? ?? ??

SELECT @@SERVERNAME AS [server_name]
     , db_name(QT.dbid) AS [db_name]
     , OBJECT_NAME(QT.objectid) AS [object_name]

     , QS.plan_generation_num
     , QS.creation_time
     , QS.last_execution_time
     , QS.execution_count

     , QS.total_worker_time / QS.execution_count AS avg_worker_time
     , QS.total_worker_time
     , QS.last_worker_time
     , QS.min_worker_time
     , QS.max_worker_time

     , QS.total_physical_reads / QS.execution_count AS avg_physical_reads
     , QS.total_physical_reads
     , QS.last_physical_reads
     , QS.min_physical_reads
     , QS.max_physical_reads

     , QS.total_logical_writes / QS.execution_count AS avg_logical_writes
     , QS.total_logical_writes
     , QS.last_logical_writes
     , QS.min_logical_writes
     , QS.max_logical_writes

     , QS.total_logical_reads / QS.execution_count AS avg_logical_reads
     , QS.total_logical_reads
     , QS.last_logical_reads
     , QS.min_logical_reads
     , QS.max_logical_reads

     , QS.total_elapsed_time / QS.execution_count AS avg_elapsed_time
     , QS.total_elapsed_time
     , QS.last_elapsed_time
     , QS.min_elapsed_time
     , QS.max_elapsed_time

     , SUBSTRING(QT.text, (QS.statement_start_offset / 2) + 1, ((CASE QS.statement_end_offset WHEN -1 THEN DATALENGTH(QT.text) ELSE QS.statement_end_offset END - QS.statement_start_offset) / 2) + 1) AS query_text

  FROM sys.dm_exec_query_stats QS
       CROSS APPLY sys.dm_exec_sql_text(QS.sql_handle) QT;
GO

--====================================================================================================
-- ?? CPU ?? 100? Plan

SELECT TOP 100
       @@SERVERNAME AS [server_name]
     , db_name(QP.dbid) AS [db_name]
     , OBJECT_NAME(QP.objectid) AS [object_name]
     , QS.total_worker_time / QS.execution_count AS avg_worker_time
     , QP.query_plan

  FROM sys.dm_exec_query_stats QS
       CROSS APPLY sys.dm_exec_query_plan(QS.plan_handle) QP
 ORDER BY QS.total_worker_time / QS.execution_count DESC;
GO

--====================================================================================================
-- ??? ??

SELECT @@SERVERNAME AS [server_name]
     , db_name(US.database_id) AS [db_name]
     , OBJECT_NAME(US.object_id) AS [object_name]
     , I.name AS [index_name]
     , US.user_seeks
     , US.user_scans
     , US.user_lookups
     , US.user_updates
     , US.last_user_seek
     , US.last_user_scan
     , US.last_user_lookup
     , US.last_user_update
     , US.system_seeks
     , US.system_scans
     , US.system_lookups
     , US.system_updates
     , US.last_system_seek
     , US.last_system_scan
     , US.last_system_lookup
     , US.last_system_update

  FROM sys.dm_db_index_usage_stats US
       INNER JOIN sys.indexes I ON (US.object_id = I.object_id AND US.index_id = I.index_id);
GO

'

RETURN
GO
