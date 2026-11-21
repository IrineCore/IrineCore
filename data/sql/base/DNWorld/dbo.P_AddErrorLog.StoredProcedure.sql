USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddErrorLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2008-05-01
description : Log the error that occured.
return value :
0 = There is no error.
1 = The transaction is in an uncommittable state. Rolling back transaction.
100 = System error occurred.
**/

CREATE PROCEDURE [dbo].[P_AddErrorLog]
	  @intPROCID int = NULL
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE
	@nvcErrorMessage nvarchar(4000),
	@intErrorNumber int,
	@intErrorSeverity int,
	@intErrorState int,
	@intErrorLine int,
	@nvcErrorProcedure nvarchar(128);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

/**_# If there is no error information to log, return 0.*/
IF ERROR_NUMBER() IS NULL
BEGIN
	SET @intReturnValue = 0;
	GOTO ErrorHandler;
END

/**_# Log the error.*/
/**_## Assign variables to error-handling functions that capture information for RAISERROR.*/
SET @intErrorNumber = ERROR_NUMBER();
SET @intErrorSeverity = ERROR_SEVERITY();
SET @intErrorState = ERROR_STATE();
SET @intErrorLine = ERROR_LINE();
SET @nvcErrorProcedure = ERROR_PROCEDURE();
SET @nvcErrorMessage = ERROR_MESSAGE();

/**_## Log the error that occurred.*/
IF @nvcErrorProcedure = OBJECT_NAME(@@PROCID)
	INSERT dbo.ErrorLogs (loginName, hostName, errorNumber, errorSeverity, errorState, errorProcedure, errorLine, errorMessage, occurredDate)
	VALUES (CAST(ORIGINAL_LOGIN() AS nvarchar(128)), CAST(HOST_NAME() AS nvarchar(128)), @intErrorNumber, @intErrorSeverity, @intErrorState, ISNULL(OBJECT_NAME(@intPROCID), @nvcErrorProcedure), @intErrorLine, @nvcErrorMessage, GETDATE());

ELSE IF @@NESTLEVEL <= 2
	INSERT dbo.ErrorLogs (loginName, hostName, errorNumber, errorSeverity, errorState, errorProcedure, errorLine, errorMessage, occurredDate)
	VALUES (CAST(ORIGINAL_LOGIN() AS nvarchar(128)), CAST(HOST_NAME() AS nvarchar(128)), @intErrorNumber, @intErrorSeverity, @intErrorState, @nvcErrorProcedure, @intErrorLine, @nvcErrorMessage, GETDATE());

/**_# Rethrow the error.*/
SET @nvcErrorMessage = N'Error %d, Level %d, State %d, Procedure %s, Line %d, Message: ' + @nvcErrorMessage;
RAISERROR (@nvcErrorMessage, @intErrorSeverity, 1, @intErrorNumber, @intErrorSeverity, @intErrorState, @nvcErrorProcedure, @intErrorLine);

RETURN 100;
ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN 0;
GO
