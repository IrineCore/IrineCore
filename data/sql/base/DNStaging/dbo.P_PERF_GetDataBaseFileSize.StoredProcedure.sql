USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_PERF_GetDataBaseFileSize]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-08-02
description : ?? ?????? ?? ??? ?????.(???? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_PERF_GetDataBaseFileSize]
	  @dtmBasisDate datetime --// ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intWorldID int

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
		 , name AS FileName
		 , size*8 AS FileSize 
	from sys.database_files WITH(NOLOCK)
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
