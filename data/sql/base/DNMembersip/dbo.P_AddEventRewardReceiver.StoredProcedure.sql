USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddEventRewardReceiver]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddEventRewardReceiver]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddEventRewardReceiver] AS' 
END
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-08-23

description :
	??? ??? ?? ??? ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101202 = ??? ??? ?? ??? ??? ???????.
	101203 = ?? ??? ??? ?? ??? ???.
**/

ALTER PROCEDURE [dbo].[P_AddEventRewardReceiver]
	  @intEventRewardID int --// ??? ?? ID
	, @intAccountID int --// ?? ?? ID
	, @inbCharacterID bigint --// ?? ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @sdtNow smalldatetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ??? ??? ?? ??? ?? ?? */
	INSERT dbo.EventRewardReceivers (CharacterID, EventRewardID, AccountID, ReceiveDate)
	VALUES (@inbCharacterID, @intEventRewardID, @intAccountID, @sdtNow);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101202;
		GOTO ErrorHandler;
	END

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_EventRewardReceivers%'
		SET @intReturnValue = 101203;
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
