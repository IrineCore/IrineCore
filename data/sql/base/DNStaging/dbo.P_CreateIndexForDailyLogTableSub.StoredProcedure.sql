USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_CreateIndexForDailyLogTableSub]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-06
description : ? ?? ?? ???? ?? ??? ????? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_CreateIndexForDailyLogTableSub]
	  @dtmBasisDate datetime --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @datBasisDate date
	, @nvcTableSuffix nchar(8)
	, @i int
	, @j int
	, @nvcTableName nvarchar(128)
	, @nvcNewTableName nvarchar(128)
	, @nvcDateColumnName nvarchar(128)
	, @nvcStmt nvarchar(max);

DECLARE @tblLogTables table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	TableName nvarchar(128) NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @datBasisDate  = DATEADD(DAY, DATEDIFF(DAY, 0, @dtmBasisDate), 0);
	SET @nvcTableSuffix = CONVERT(nchar(8), @datBasisDate, 112);

	INSERT @tblLogTables (TableName)
	SELECT name
	FROM sys.tables
	WHERE name LIKE N'%Logs' AND name NOT IN (N'TableAuditLogs', N'ETLLogs', N'ErrorLogs', N'CharacterActionLogs', N'tmpLogs');

	SELECT @i = 1, @j = @@ROWCOUNT;

	WHILE @i <= @j
	BEGIN
		SELECT @nvcTableName = TableName FROM @tblLogTables WHERE Seq = @i;
		SELECT TOP (1) @nvcDateColumnName = name FROM sys.columns WHERE [object_id] = OBJECT_ID(@nvcTableName, N'U') AND system_type_id IN (40, 42, 43, 58, 61) ORDER BY column_id;

		SET @nvcNewTableName = @nvcTableName + N'_' + @nvcTableSuffix;

		SET @nvcStmt = N'
IF OBJECT_ID(N''' + @nvcNewTableName + N''', N''U'') IS NOT NULL
	CREATE CLUSTERED INDEX IX_NC_' + @nvcNewTableName + N'_' + @nvcDateColumnName + N' ON dbo.' + @nvcNewTableName + N' (' + @nvcDateColumnName + N');';

		EXEC (@nvcStmt);

		SET @i += 1;
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
