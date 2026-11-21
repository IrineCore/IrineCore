USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_ModMobileSerial]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_ModMobileSerial]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_ModMobileSerial] AS' 
END
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-06-26
description : ??? ?????? ??/?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ??? ????.
**/

ALTER PROCEDURE [dbo].[P_DRS_ModMobileSerial]
	  @intAccountID int --// ??ID	
	, @vchMobileSerial varchar(max) --// ??? ?????	
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
	/**_# ??? ???? ?????. */
	SELECT AccountID
	FROM dbo.MobileAuthentication WITH (NOLOCK)
	WHERE AccountID = @intAccountID AND AccountAuthenticationKey IS NULL;

	/**_# ??? ??? ???? ???? */
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ?? ??? ????? ?? */
	UPDATE dbo.MobileAuthentication
	SET EncryptedSerial = dbo.FN_HashPassphrase(@vchMobileSerial, @intAccountID)
	WHERE AccountID = @intAccountID;
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
