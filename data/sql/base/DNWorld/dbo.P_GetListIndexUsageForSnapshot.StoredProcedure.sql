USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListIndexUsageForSnapshot]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-04-30
description : ??? ????? ?????.(???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListIndexUsageForSnapshot]
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
		, CAST(DB_NAME() AS nvarchar(50)) AS DatabaseName 
		, OBJECT_NAME(I.object_id) AS TableName
		, ISNULL(I.name, 'Heap') AS IndexName
		, IUS.user_scans AS UserScans
		, IUS.user_seeks AS UserSeeks
		, IUS.user_lookups AS UserLookups
		, IUS.user_updates AS UserUpdates
	FROM sys.objects O WITH(NOLOCK)
		INNER JOIN sys.indexes I WITH(NOLOCK) ON O.object_id = I.object_id
		LEFT OUTER JOIN sys.dm_db_index_usage_stats IUS WITH(NOLOCK) ON I.object_id = IUS.object_id AND I.index_id = IUS.index_id AND IUS.database_id = DB_ID()
	WHERE O.type = 'U'

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
