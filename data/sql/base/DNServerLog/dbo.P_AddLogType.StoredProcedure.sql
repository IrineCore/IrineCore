USE [dnserverlog]
GO
/****** Object:  StoredProcedure [dbo].[P_AddLogType]    Script Date: 11/14/2026 8:58:01 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-24
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
105101 = ?? ??? ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddLogType]
	  @inyLogType tinyint --// ?? ?? ?
	, @nvcLogTypeName nvarchar(50) --// ?? ?? ??
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
	/**_# [LogTypes] ???? INSERT???.*/
	INSERT dbo.LogTypes (LogType, LogTypeName)
	VALUES (@inyLogType, @nvcLogTypeName);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_LogTypes%'
		SET @intReturnValue = 105101;

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
