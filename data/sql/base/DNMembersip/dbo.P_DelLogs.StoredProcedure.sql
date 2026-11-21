USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DelLogs]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DelLogs]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DelLogs] AS' 
END
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-14
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DelLogs]
	@dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @i int
	, @j int
	, @nvcStmt nvarchar(4000)
	, @nvcLogTableName nvarchar(128)
	, @nvcDTColumnName nvarchar(128)
	, @nchLogTableSuffix nchar(1)
	, @dt2BasisDate datetime2(0) = DATEADD(DAY, DATEDIFF(DAY, 0, @dtmBasisDate), 0);

DECLARE @tblTables table (Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, LogTableName sysname NOT NULL, DTColumnName sysname NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblTables (LogTableName, DTColumnName)
	VALUES (N'AccountLoginLogs', N'LoginDate')
		, (N'AccountLogoutLogs', N'LogoutDate')
		, (N'MobileAuthenticationLogs', N'LogDate');

	SELECT @i = 1, @j = @@ROWCOUNT, @nchLogTableSuffix = CAST(dbo.FN_LogTableSuffix(DATEADD(DAY, -1, @dt2BasisDate)) AS nchar(1));

	WHILE @i <= @j
	BEGIN
		SELECT @nvcLogTableName = LogTableName + @nchLogTableSuffix, @nvcDTColumnName = DTColumnName
		FROM @tblTables
		WHERE Seq = @i;

		SET @nvcStmt = N'
IF NOT EXISTS (SELECT *  FROM dbo.' + @nvcLogTableName + N' WITH (NOLOCK) WHERE ' + @nvcDTColumnName + N' >= @dt2BasisDate)
	TRUNCATE TABLE dbo.' + @nvcLogTableName + N';';

		EXEC sp_executesql @nvcStmt, N'@dt2BasisDate datetime2(0)', @dt2BasisDate = @dt2BasisDate;

		SET @i += 1;
	END

	SET @i = 1;

	WHILE @i > 0
	BEGIN
		DELETE TOP (50) dbo.CUCount WHERE SnapshotDate < @dt2BasisDate;
		SET @i = @@ROWCOUNT;
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
