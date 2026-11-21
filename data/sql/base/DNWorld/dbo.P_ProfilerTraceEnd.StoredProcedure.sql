USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ProfilerTraceEnd]    Script Date: 11/14/2026 8:54:39 AM ******/
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


CREATE PROCEDURE [dbo].[P_ProfilerTraceEnd]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @intTraceID int
	  , @nvcTraceFile nvarchar(245)

IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP 1 @intTraceID = TraceID, @nvcTraceFile = TraceFile FROM dbo.ProfilerTraceInfo 

	IF @@ROWCOUNT > 0
	BEGIN			
		EXEC sp_trace_setstatus 
		  @traceid = @intTraceID
		, @status = 0;

		EXEC sp_trace_setstatus 
		  @traceid = @intTraceID
		, @status = 2;

		INSERT dbo.ProfilerResult (StartTime, EndTime, TextData, HostName, LoginName, Duration, Reads, Writes, CPU, ObjectID, ServerName, EventClass, ObjectType, ObjectName, DatabaseName)
		SELECT StartTime, EndTime, TextData, HostName, LoginName, Duration, Reads, Writes, CPU, ObjectID, ServerName, EventClass, ObjectType, ObjectName, DatabaseName
		FROM fn_trace_gettable (@nvcTraceFile, default);
	END

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
