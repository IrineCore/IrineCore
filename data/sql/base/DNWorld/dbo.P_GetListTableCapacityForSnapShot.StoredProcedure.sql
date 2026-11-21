USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTableCapacityForSnapShot]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-06
description : ?? ??? ??? ?????.(DailySnapshot)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListTableCapacityForSnapShot]
	  @dtmBasisDate datetime
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
	;WITH C1 (object_id, ReservedSize, DataSize, IndexSize, UnusedSize, [Rows]) AS (
		SELECT object_id
			, SUM(reserved_page_count) * 8
			, SUM(CASE WHEN index_id <= 1 THEN used_page_count END) * 8
			, ISNULL(SUM(CASE WHEN index_id > 1 THEN used_page_count END) * 8, 0)
			, SUM(reserved_page_count - used_page_count) * 8
			, MAX(CASE WHEN index_id <= 1 THEN row_count END)
		FROM sys.dm_db_partition_stats
		GROUP BY object_id
	)
	SELECT CAST(@dtmBasisDate AS date) AS BasisDate, @@SERVERNAME AS ServerName, DB_NAME() AS DatabaseName, T.name AS TableName, C1.ReservedSize, C1.DataSize, C1.IndexSize, C1.UnusedSize, C1.[Rows]
	FROM sys.tables T
		INNER JOIN C1 ON T.object_id = C1.object_id
	ORDER BY T.name
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
