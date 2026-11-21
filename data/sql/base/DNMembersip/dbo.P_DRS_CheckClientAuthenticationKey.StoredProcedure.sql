USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_CheckClientAuthenticationKey]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_CheckClientAuthenticationKey]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_CheckClientAuthenticationKey] AS' 
END
GO
/**
version : 10
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-05-09
description : ??? ????? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101194 = ???? ???? ??? ????.
101195 = ????? ? ????(3?)? ???????.
**/

ALTER PROCEDURE [dbo].[P_DRS_CheckClientAuthenticationKey]	
	  @vchAccountAuthenticationKey varchar(5) --// ????? ???
	, @intAccountID int OUTPUT --// ??ID		
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @datIssueDate datetime
        , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [MobileAuthentication] ???? SELECT ???.*/
	SELECT @intAccountID = AccountID, @datIssueDate = AccountAuthenticationKeyIssueDate
	FROM dbo.MobileAuthentication WITH (NOLOCK)
	WHERE AccountAuthenticationKey = CAST (@vchAccountAuthenticationKey AS int);

	/**_# ?? ?? ???? ??? ?? ?? ?? ??*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101194;
		GOTO ErrorHandler;
	END

	/**_# ???????? 3???? ?? ?? ??? ?? ???? ?? ?? */
	IF DATEDIFF(SS,@datIssueDate,GETDATE()) > 180
	BEGIN
		DELETE dbo.MobileAuthentication WHERE AccountID = @intAccountID AND AccountAuthenticationKey = CAST (@vchAccountAuthenticationKey AS int);
		SET @intReturnValue = 101195;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ?? ? ??? ?? */
	UPDATE dbo.MobileAuthentication SET AccountAuthenticationKey = NULL WHERE AccountID = @intAccountID;

	/**_# ?? ?? ?? ? ?? ???? ?? ???? ?? ??? ???? */
	DELETE dbo.AccountAuthenticationDelQueue WHERE AccountID = @intAccountID;

	/**_# ?? ?????? 1???? ??? ?? ???? */
	DELETE dbo.MobileAuthentication
	WHERE DATEDIFF(MI,AccountAuthenticationKeyIssueDate,GETDATE()) > 60 AND AccountAuthenticationKey IS NOT NULL;

	
	/**_# ??? ?? ?? ?? */
	EXEC dbo.P_AddMobileAuthenticationLog
			@dt2LogDate = @dt2Now
		  , @intAccountID = @intAccountID
		  , @inyAuthenticationLogCode = 1
		  , @intAccountAuthenticationKey = NULL;

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
