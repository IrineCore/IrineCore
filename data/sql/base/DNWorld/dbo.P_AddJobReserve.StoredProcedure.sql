USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddJobReserve]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-07-26

description :
	?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history ;
	3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_AddJobReserve]
	  @nvcCommand nvarchar(max) --// ?? ??
	, @intSeqNo int = NULL OUTPUT --// ????
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

	/**_# [JobReserve] ???? INSERT???.*/
	INSERT dbo.JobReserve (Command, StatusCode, RegisterDate)
	VALUES (@nvcCommand, 0, GETDATE());

	SET @intSeqNo = SCOPE_IDENTITY();

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
