USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetMobileAuthenticationFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_GetMobileAuthenticationFlag]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_GetMobileAuthenticationFlag] AS' 
END
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-06-27
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ??? ????.
**/

ALTER PROCEDURE [dbo].[P_DRS_GetMobileAuthenticationFlag]
	  @intAccountID int --// ??? ?? ID
	, @bitMobileAuthenticationFlag bit = NULL OUTPUT --// ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intAuthenticationKey int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [MobileAuthentication] ???? SELECT???.*/
	SELECT @intAuthenticationKey = AccountAuthenticationKey
	FROM dbo.MobileAuthentication WITH (NOLOCK)
	WHERE AccountID = @intAccountID AND AccountAuthenticationKey IS NULL;

	IF @@ROWCOUNT = 0
	BEGIN	
		SET @intReturnValue = 101115;
		SET @bitMobileAuthenticationFlag = 0;
		GOTO ErrorHandler;
	END

	IF @intAuthenticationKey IS NULL
		SET @bitMobileAuthenticationFlag = 1;
	ELSE
		SET @bitMobileAuthenticationFlag = 0;

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
