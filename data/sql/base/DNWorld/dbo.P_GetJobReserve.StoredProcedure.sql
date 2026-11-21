USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetJobReserve]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-07-26

description :
	??? ?? ?? ??
result set :
	@inyJobStatus	tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103311 = Job ? ???? ????.
history :
	5 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetJobReserve]
	  @intSeqNo int --// ????
	, @inyStatusCode tinyint = null OUTPUT --// Job ??
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

	/**_# ??? ?? ?? ?? */
	SELECT @inyStatusCode = StatusCode
	  FROM dbo.JobReserve WITH (NOLOCK)
	 WHERE SeqNo = @intSeqNo;

	/**_## ??? ???? ?? ???? ??? 101194 ? ?????. */
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103311;
		GOTO ErrorHandler;
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
