USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_PERF_GetDiskSize]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-08-02
description : ?? ?? ??? ??? ?????.(???? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_PERF_GetDiskSize]
	  @dtmBasisDate datetime --// ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @ObjectToken int,
        @chadrive char(1),
        @intodrive int,
        @varTotalSize varchar(20),
        @inbMB bigint = 1048576,
	    @i int = 1

CREATE TABLE #Driveslist
(
	Seq int identity (1,1),
    Drive         char(1) NOT NULL,
    FreeSpaceMB   int NULL,
    TotalSizeMB   int NULL
)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END
BEGIN TRY
	
	INSERT #Driveslist(Drive, FreeSpaceMB) 
	EXEC MASTER.dbo.xp_fixeddrives

	EXEC sp_OACreate 'Scripting.FileSystemObject', @ObjectToken OUT  --Creates an instance of the OLE object on an instance of SQL Server.

	WHILE 1=1
	BEGIN
		SELECT @chadrive = Drive 
		FROM #Driveslist
		WHERE Seq = @i;
		
		IF @@ROWCOUNT = 0
			BREAK;

		EXEC sp_OAMethod @ObjectToken, 'GetDrive', @intodrive OUT, @chadrive  

		EXEC sp_OAGetProperty @intodrive,'TotalSize', @varTotalSize OUT

		UPDATE #Driveslist
		SET TotalSizeMB = @varTotalSize / @inbMB
		WHERE drive = @chadrive
		        
		SET @i+=1;
	END

	EXEC sp_OADestroy @ObjectToken

	SELECT CAST(@dtmBasisDate as date) AS BasisDate, drive, @@SERVERNAME AS ServerName, DB_NAME() AS DatabaseName,
		   TotalSizeMB as 'Total(MB)',
		   FreeSpaceMB as 'Free(MB)',
		   CAST((FreeSpaceMB/(TotalSizeMB * 1.0)) * 100.0 as int) as 'Free(%)'
	FROM #Driveslist
	ORDER BY drive
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
