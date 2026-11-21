USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_IssueAccountAuthenticationKey]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_IssueAccountAuthenticationKey]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_IssueAccountAuthenticationKey] AS' 
END
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-05-08
description : ???? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DRS_IssueAccountAuthenticationKey]
	  @intAccountID int --// ??ID
	, @vchAccountAuthenticationKey varchar(5) OUTPUT --// 	??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intNewAuthenticationKey int
	  , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	/**_# ??? ? ?? */
	SELECT @intNewAuthenticationKey =  ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint)) % 100000;
 
	/**_# ?? ??? Account?? ??*/
	
	IF EXISTS (SELECT * FROM dbo.MobileAuthentication WITH (NOLOCK) WHERE AccountID = @intAccountID)
	BEGIN
		/**_# ?? ??? Account? ?? ??? ?? ???? ???? */
		UPDATE dbo.MobileAuthentication
		SET AccountAuthenticationKey = @intNewAuthenticationKey,
			AccountAuthenticationKeyIssueDate = GETDATE()
		WHERE AccountID = @intAccountID;		
	END
	ELSE
	BEGIN
		/**_# ??? Account? ?? ??? ?? ???? ?? */
		INSERT dbo.MobileAuthentication (AccountID, AccountAuthenticationKey, AccountAuthenticationKeyIssueDate) 
		VALUES (@intAccountID, @intNewAuthenticationKey, GETDATE());
	END		
	
	SET @vchAccountAuthenticationKey = CASE WHEN @intNewAuthenticationKey < 10000 THEN RIGHT('00000' + CAST (@intNewAuthenticationKey AS varchar(5)),5) ELSE CAST (@intNewAuthenticationKey AS varchar(5)) END;

	/**_# ??? ?? ??? ??*/
	EXEC dbo.P_AddMobileAuthenticationLog
			@dt2LogDate = @dt2Now
		  , @intAccountID = @intAccountID
		  , @inyAuthenticationLogCode = 1
		  , @intAccountAuthenticationKey = @intNewAuthenticationKey;

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
