USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_DelDNEvent]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_DelDNEvent]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_DelDNEvent] AS' 
END
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-11-08

description :
	??? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101217 = ??? ??? ???????.
history :
	1 = ?? ??
**/

ALTER PROCEDURE [dbo].[P_DLS_DelDNEvent]
	  @intDNEventID int --// ??? ID
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

	BEGIN TRANSACTION

		/**_# ??? ?? ?? */
		DELETE dbo.DNEventSchedules
		 WHERE DNEventID = @intDNEventID;

		/**_# ??? ?? */
		DELETE dbo.DNEvents
		 WHERE DNEventID = @intDNEventID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101217;
			GOTO ErrorHandler;
		END

	COMMIT TRANSACTION

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
