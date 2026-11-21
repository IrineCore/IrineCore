USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ProfilerTraceStart]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-08-01
description : ?? ????? ?????? ?????.
history :
1 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_ProfilerTraceStart]
	  @inbduration bigint --//????
	, @nvcProcedureName nvarchar(256) --//????? ????? ????
	, @nvcTraceFileLocation nvarchar(100) --// Trace File? ??? Local ??(*??? ?? ??? ????? ???.*) ex)f:\Trace\ 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
DECLARE @trace_id int
	  , @options int = 2
	  , @tracefile nvarchar(245) 
	  , @maxfilesize bigint = 10
	  , @stoptime datetime = dateadd(hh, 1, getdate())
	  , @filecount int = 10

DECLARE @event TABLE (
	event_id int, 
	column_id int, 
	column_desc nvarchar(200), 
	logical_operator int, 
	comparison_operator int, 
	value sql_variant
)

IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	TRUNCATE TABLE dbo.ProfilerTraceInfo

	IF RIGHT(@nvcTraceFileLocation, 1) <> N'\'
		SET @nvcTraceFileLocation += N'\';

	SET @tracefile = @nvcTraceFileLocation + db_name() + '-' + convert(nvarchar(8), getdate(), 112) + '-' + replace(convert(nvarchar(8), getdate(), 108), N':', N'')

	EXEC sp_trace_create
	  @traceid =  @trace_id OUTPUT 
	, @options = @options
	, @tracefile = @tracefile
	, @maxfilesize = @maxfilesize
	, @stoptime = null
	, @filecount = @filecount;

	
	INSERT dbo.ProfilerTraceInfo (TraceID, TraceFile)
	SELECT @trace_id, @tracefile+N'.trc';
		
	--trace ??? ??? ??
	insert @event (event_id, column_id, column_desc, logical_operator, comparison_operator, value)
	select *
	from
	(
		select [event_id] = 10 --RPC:Completed
		union all select 12 --SQL:BatchCompleted
	) x
	cross join 
	(
		select [column_id] = 1, [column_desc] = N'TextData', [logical_operator] = null, [comparison_operator] = null, [value] = null
		union all select 6, N'NTUserName' , null, null, null
		union all select 8, N'HostName' , null, null, null
		union all select 9, N'ClientProcessID' , null, null, null
		union all select 10, N'ApplicationName' , null, null, null
		union all select 11, N'LoginName' , null, null, null
		union all select 12, N'SPID' , null, null, null
		union all select 13, N'Duration' , 0, 3, 50
		union all select 14, N'StartTime' , null, null, null
		union all select 15, N'EndTime' , null, null, null
		union all select 16, N'Reads' , null, null, null
		union all select 17, N'Writes' , null, null, null
		union all select 18, N'CPU' , null, null, null
		union all select 22, N'ObjectID' , null, null, null
		union all select 26, N'ServerName' , null, null, null
		union all select 27, N'EventClass' , null, null, null
		union all select 28, N'ObjectType' , null, null, null
		union all select 34, N'ObjectName' , null, null, null
		union all select 35, N'DatabaseName' , null, null, null
	) y
	select * from @event

	declare @sql nvarchar(max) = '';
	select @sql = @sql + 
	'exec sp_trace_setevent 
		@traceid = ' + replace(str(@trace_id), ' ', '') + '
		, @eventid = ' + replace(str(event_id), ' ', '')  + '
		, @columnid = ' + replace(str(column_id), ' ', '') + '
		, @on = 1
	' + CHAR(10) 
	from @event

	exec (@sql)
	
	SET @nvcProcedureName = N'%'+@nvcProcedureName+N'%'

	exec sp_trace_setfilter @trace_id, 13, 0, 2, @inbduration
	exec sp_trace_setfilter @trace_id, 1, 0, 6, @nvcProcedureName
	
	exec sp_trace_setstatus 
	  @traceid = @trace_id
	, @status = 1;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 19062
	BEGIN
		PRINT N'File path is invalid.';
		SET @intReturnValue = 1;
	END
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
