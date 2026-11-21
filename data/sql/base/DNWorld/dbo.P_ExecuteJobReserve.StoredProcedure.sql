USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ExecuteJobReserve]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-07-26

description :
	??? ?? ?? (DB Schedule ? ?? ?? ??)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	8 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_ExecuteJobReserve]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intSeqNo int, @nvcCommand nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	-- ??? ??? ?? ????.
	SELECT TOP 1
		   @intSeqNo = SeqNo
		 , @nvcCommand = Command
	  FROM dbo.JobReserve WITH (NOLOCK, INDEX(IX_NN_JobReserve_StatusCode))
	 WHERE StatusCode = 0
	OPTION (KEEPFIXED PLAN);

	-- ??? ??? ??? ?????.
	IF @@ROWCOUNT = 0 RETURN 0;

	-- ??? ??? ???? '???'?? ??? ??.
	UPDATE dbo.JobReserve
	   SET StatusCode = 2
	 WHERE SeqNo = @intSeqNo;

	-- ??? ?? ??
	EXEC (@nvcCommand);

	-- ??? ??? ???? '??'? ??? ??.
	UPDATE dbo.JobReserve
	   SET StatusCode = 1
	 WHERE SeqNo = @intSeqNo;

END TRY
BEGIN CATCH

	-- ??? ??? ???? '??'? ??? ??.
	UPDATE dbo.JobReserve
	   SET StatusCode = 3
	 WHERE SeqNo = @intSeqNo;

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
