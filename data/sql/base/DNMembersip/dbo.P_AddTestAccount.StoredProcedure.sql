USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTestAccount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddTestAccount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddTestAccount] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-22
description : ???? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101151 = ?? ??? ?????-????IP ?????.
**/

ALTER PROCEDURE [dbo].[P_AddTestAccount]
	  @nvcAccountName nvarchar(50) --// ?? ??
	, @vchIP varchar(15) --// ?? ??? IP ??
	, @intAuditLogID int --// GSM ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [TestAccounts] ???? INSERT???.*/
	INSERT dbo.TestAccounts (AccountID, IP, AuditLogID, RegisterDate)
	SELECT AccountID, @intIP, @intAuditLogID, GETDATE()
	FROM dbo.Accounts WITH (NOLOCK)
	WHERE AccountName = @nvcAccountName;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_TestAccounts%'
		SET @intReturnValue = 101151;

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
