USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_IndexRebuild]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-12-07
description : ???? ??? ???.(???? ? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_IndexRebuild]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblTemp TABLE (Seq int identity, [object_id] int)
DECLARE @i int = 1, @j int, @object_id int;
DECLARE @Result table (TableName sysname NOT NULL, IndexName sysname NOT NULL, avg_fragmentation_in_percent decimal NOT NULL);
DECLARE @nvcStmt nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	INSERT @tblTemp ([object_id])
	SELECT [object_id]
	FROM sys.tables
	WHERE name NOT LIKE N'%Logs_'
	AND name NOT LIKE N'GuildWar%'
	AND name <> N'ErrorLogs';

	SET @j = @@ROWCOUNT;

	WHILE @i <= @j
	BEGIN
		SELECT @object_id = [object_id] FROM @tblTemp WHERE Seq = @i; 

		INSERT @Result (TableName, IndexName, avg_fragmentation_in_percent)
		SELECT OBJECT_NAME(@object_id), name, avg_fragmentation_in_percent
		FROM sys.dm_db_index_physical_stats (DB_ID(), @object_id, NULL, NULL, NULL) AS a
			JOIN sys.indexes AS b ON a.object_id = b.object_id AND a.index_id = b.index_id
		WHERE avg_fragmentation_in_percent >= 30 AND name IS NOT NULL

		SET @i += 1;
	END

	SELECT @nvcStmt += N'ALTER INDEX ' + IndexName + N' ON dbo.' + TableName + N' REBUILD' + CHAR(13) + CHAR(10)
	FROM @Result;

	EXEC (@nvcStmt);
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
