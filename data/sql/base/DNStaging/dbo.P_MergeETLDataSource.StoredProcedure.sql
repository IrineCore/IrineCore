USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_MergeETLDataSource]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-08
description : ETL?? ??? ??? ?? ?? ???? MERGE???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_MergeETLDataSource]
	  @nvcDataSourceNames nvarchar(max) --// |? ?????? DB ?? ???
	, @vchConnectionStrings varchar(max) --// |? ?????? DBMS ?? ???
WITH EXECUTE AS 'SPExecutor'
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
	BEGIN TRANSACTION;

	/**_# [ETLDataSources] ???? TRUNCATE???.*/
	TRUNCATE TABLE dbo.ETLDataSources;

	/**_# [ETLDataSources] ???? INSERT???.*/
	INSERT dbo.ETLDataSources (DataSourceName, ConnectionString)
	SELECT A.string, B.string
	FROM dbo.IF_NSplit(@nvcDataSourceNames, N'|') A
		INNER JOIN dbo.IF_Split(@vchConnectionStrings, '|') B ON A.seq = B.seq
	WHERE A.string > N'' AND B.string > '';

	COMMIT TRANSACTION;
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
