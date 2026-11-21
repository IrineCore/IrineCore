USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_DropExpiredLogTable]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 15
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-02
description : ?? ??? ??? ?? ???? ?????. (SP? ??? ??? ???? ?? ??? ?????)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history:
6 = Drop ?? DBName? ?? Drop? ?? ?? ?? ?? 20111010.ohtack
7 = Drop ?? DBName? ?? Drop? ?? ?? ?? ?? 20111010.ohtack
8 = Drop ?? DBName? ?? Drop? ?? ?? ?? ?? 20111010.ohtack
9 = Drop ?? DBName? ?? Drop? ?? ?? ?? ?? 20111010.ohtack
11~12 = WITH EXECUTE AS 'SPExecutor' ????     by ??? at 2011-12-07
13 = WITH EXECUTE AS 'SPExecutor' ???? ??     by ??? at 2011-12-08
14 = ??? ?????? ??? ??? ?? ????? ?? ??     by ??? at 2013-06-03
**/

CREATE PROCEDURE [dbo].[P_DropExpiredLogTable]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @datBasisDate date = GETDATE() - 1
DECLARE @nvcStmt nvarchar(max) = N''
	, @i int
	, @j int
	, @nvcDBName sysname;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	CREATE TABLE #Databases (
		Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
		DBName sysname NOT NULL
	);

	INSERT #Databases (DBName)
	SELECT name
	FROM sys.databases
	WHERE name LIKE N'DNStagingLog%'
	UNION ALL
	SELECT N'DNStaging';

	SELECT @i = 1, @j = @@ROWCOUNT;

	WHILE @i <= @j
	BEGIN
		SELECT @nvcDBName = DBName FROM #Databases WHERE Seq = @i;

		SET @nvcStmt = N'
DECLARE @nvcStmt nvarchar(max) = N'''';

SELECT @nvcStmt += N''DROP TABLE ' + @nvcDBName + N'.dbo.'' + TB.name + N'';'' + CHAR(13) + CHAR(10)
FROM ' + @nvcDBName + N'.sys.tables TB 
	LEFT OUTER JOIN dbo.LogLifespan LS ON SUBSTRING(name, 1, LEN(TB.name)-9) = LS.LogTableName
WHERE TB.name LIKE N''%Logs_%'' and name NOT IN (N''ErrorLogs'', N''ETLLogs'', N''TableAuditLogs'', ''tmpLogs'')
  AND RIGHT(TB.name, 8) < CONVERT(char(8), DATEADD(DAY, - CASE WHEN LS.Lifespan IS NULL THEN 90 WHEN LS.Lifespan < 30 THEN 30 ELSE LS.Lifespan END, @datBasisDate), 112);

EXEC (@nvcStmt);'
		EXEC sys.sp_executesql @nvcStmt, N'@datBasisDate date', @datBasisDate = @datBasisDate;

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
