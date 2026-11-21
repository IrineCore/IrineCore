USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModAccountName]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModAccountName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModAccountName] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-10-12
description : AccountName? ?????. (??? ?? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_ModAccountName]
	  @nvcOldAccountName nvarchar(50) --// ?? ??? ?? ??
	, @nvcNewAccountName nvarchar(50) --// ? ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @intAccountID int
	  , @dt2Now datetime2(0) = GETDATE();
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @nvcOldAccountName = @nvcOldAccountName + N'@nx'

	UPDATE dbo.Accounts
	SET AccountName = @nvcNewAccountName
      , @intAccountID = AccountID
	FROM dbo.Accounts WITH (INDEX(IX_UN_Accounts_AccountName), FORCESEEK)
	WHERE AccountName = @nvcOldAccountName
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT > 0
		INSERT dbo.TransferAccounts (TransferDate, AccountID, OldAccountName, NewAccountName)
		VALUES (@dt2Now, @intAccountID, @nvcOldAccountName, @nvcNewAccountName);
		
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
