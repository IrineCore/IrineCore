USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_SlideWindow]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-11
description : Log ???? SATA HDD? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = Log Data? SAS HDD? ???? ??? ?? ????? ????? ?? (??? = 30?) at 2010-12-07
**/

CREATE PROCEDURE [dbo].[P_SlideWindow]
	  @dtmBasisDate datetime --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intLifespan int, @nvcSuffix nvarchar(9), @i int, @j int, @k int, @l int, @nvcTableName nvarchar(128), @nvcStmt nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intLifespan = Lifespan FROM dbo.SlideWindowConfig WITH (NOLOCK);

	IF @@ROWCOUNT = 0
		SET @intLifespan = 30;

	SET @dtmBasisDate = DATEADD(DAY, -@intLifespan, @dtmBasisDate);
	SET @nvcSuffix = N'_' + CONVERT(nvarchar(8), @dtmBasisDate, 112);

	CREATE TABLE #TargetLogs (
		Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
		TableName nvarchar(128) NOT NULL
	);

	CREATE TABLE #Indexes (
		Seq int NOT NULL,
		name nvarchar(128) NOT NULL,
		[type] int NOT NULL,
		is_unique bit NOT NULL,
		columnNames nvarchar(2000) NOT NULL
	);

	INSERT #TargetLogs (TableName)
	SELECT name
	FROM sys.tables
	WHERE name LIKE N'%Logs\' + @nvcSuffix ESCAPE N'\'

	SELECT @i = 1, @j = @@ROWCOUNT;

	WHILE @i <= @j
	BEGIN
		TRUNCATE TABLE #Indexes;

		SET @nvcStmt = N''

		SELECT @nvcTableName = TableName FROM #TargetLogs WHERE Seq = @i;

		WITH C1 AS (
			SELECT IXC2.[object_id], IXC2.index_id, STUFF((
				SELECT N',' + COL.name
				FROM sys.index_columns IXC 
					INNER JOIN sys.columns COL ON IXC.[object_id] = COL.[object_id] AND IXC.column_id = COL.column_id
				WHERE IXC.[object_id] = IXC2.[object_id] AND IXC.index_id = IXC2.index_id
				ORDER BY IXC.key_ordinal
				FOR XML PATH('')
			), 1, 1, N'') AS columnNames
			FROM sys.index_columns IXC2
			GROUP BY IXC2.[object_id], IXC2.index_id
		)
		INSERT #Indexes (Seq, name, [type], is_unique, columnNames)
		SELECT ROW_NUMBER() OVER(PARTITION BY I.[type] ORDER BY I.index_id), I.name, I.[type], I.is_unique, C1.columnNames
		FROM sys.indexes I
			INNER JOIN C1 ON I.[object_id] = C1.[object_id] AND I.index_id = C1.index_id
		WHERE I.[object_id] = OBJECT_ID(@nvcTableName); 

		SELECT @k = 1, @l = COUNT(*) FROM #Indexes WHERE [type] = 2;

		SELECT @nvcStmt += N'DROP INDEX ' + name + N' ON dbo.' + @nvcTableName + N';' + CHAR(13) + CHAR(10)
		FROM #Indexes
		WHERE [type] = 2;

		IF @@ROWCOUNT > 0
			EXEC (@nvcStmt);

		SELECT @nvcStmt = N'CREATE CLUSTERED INDEX ' + name + N' ON dbo.' + @nvcTableName + N' (' + columnNames + N') WITH DROP_EXISTING ON [OldData];'
		FROM #Indexes
		WHERE [type] = 1;

		IF @@ROWCOUNT > 0
			EXEC (@nvcStmt);

		SET @nvcStmt = N'';

		SELECT @nvcStmt += N'CREATE ' + CASE WHEN is_unique = 1 THEN N'UNIQUE ' ELSE N'' END + N'INDEX ' + name + N' ON dbo.' + @nvcTableName + N' (' + columnNames + N') ON [OldData];' + CHAR(13) + CHAR(10)
		FROM #Indexes
		WHERE [type] = 2;

		IF @@ROWCOUNT > 0
			EXEC (@nvcStmt);

		SET @i += 1;
	END

	DROP TABLE #TargetLogs;
	DROP TABLE #Indexes;
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
