USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_MoveAnotherAccountCash]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_MoveAnotherAccountCash]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_MoveAnotherAccountCash] AS' 
END
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-08-22
description : ? ???? ?? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101197 = ??? ???? ????.
101198 = ?? ??? ?????.
**/

ALTER PROCEDURE [dbo].[P_MoveAnotherAccountCash]
	  @nvcSourceAccountName nvarchar(50) --// ?? ??? ?? ??
	, @nvcTargetAccountName nvarchar(50) --// ?? ??? ?? ??
	, @intCashAmount int --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intSourceAccountID int
	, @intTargetAccountID int
	, @intCashBalanceBefore int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	SELECT @intSourceAccountID = AccountID
	FROM dbo.Accounts WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK)
	WHERE AccountName = @nvcSourceAccountName
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101197;
		GOTO ErrorHandler;
	END

	SELECT @intTargetAccountID = AccountID
	FROM dbo.Accounts WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK)
	WHERE AccountName = @nvcTargetAccountName
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101197;
		GOTO ErrorHandler;
	END

	EXEC @intReturnValue = dbo.P_GetCashBalance
		  @intAccountID = @intSourceAccountID
		, @nvcAccountName = NULL
		, @intCashBalance = @intCashBalanceBefore OUTPUT;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	IF @intCashAmount > @intCashBalanceBefore
	BEGIN
		SET @intReturnValue = 101198;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;

	INSERT dbo.CashOutgo (AccountID, CashOutgoCode, PurchaseOrderID, CashAmount, OutgoDate)
	VALUES (@intSourceAccountID, 4, NULL, @intCashAmount, GETDATE());

	INSERT dbo.CashIncome (AccountID, CashIncomeCode, PGCode, PGKey, CashAmount, IncomeDate)
	VALUES (@nvcTargetAccountName, 3, NULL, NULL, @intCashAmount, GETDATE());

	COMMIT TRANSACTION;
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
