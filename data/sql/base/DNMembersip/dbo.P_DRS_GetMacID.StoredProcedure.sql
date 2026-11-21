USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetMacID]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_GetMacID]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_GetMacID] AS' 
END
GO
/**
version : 1
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-09-04
description : ??? MacID? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000064 = ??? ???? ?? ?????.
1030000065 = ??? MacID? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_DRS_GetMacID]
	  @intAccountID int --// ??? ?? ID
	, @vchMobileSerial varchar(max) --// ??? ?????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @binEncryptedSerial binary(20);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [MobileAuthentication] ???? SELECT???.*/
	SELECT @binEncryptedSerial = EncryptedSerial
	FROM dbo.MobileAuthentication WITH (NOLOCK)
	WHERE AccountID = @intAccountID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000064;
		GOTO ErrorHandler;
	END
	
	IF dbo.FN_HashPassphrase(@vchMobileSerial,@intAccountID) <> @binEncryptedSerial
	BEGIN
		SET @intReturnValue = 1030000065;
		GOTO ErrorHandler;
	END
	
END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
