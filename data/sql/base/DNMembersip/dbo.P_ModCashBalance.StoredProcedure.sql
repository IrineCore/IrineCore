USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModCashBalance]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModCashBalance]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModCashBalance] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-07-11
description : ???? ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101197 = ??? ???? ????.
101199 = ?? ??? ???? ?? ??? ?? ?? ?? ??? ??????. ?? ? ?? ??? ???.
**/

ALTER PROCEDURE [dbo].[P_ModCashBalance]
	  @nvcAccountName nvarchar(50) --// ??? ?? ??
	, @intCashBalance int --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intAccountID int
	, @intCashBalanceBefore int
	, @intCashBalanceAfter int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intAccountID = AccountID
	FROM dbo.Accounts WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK)
	WHERE AccountName = @nvcAccountName
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101197;
		GOTO ErrorHandler;
	END

	EXEC @intReturnValue = dbo.P_GetCashBalance
		  @intAccountID = @intAccountID
		, @nvcAccountName = NULL
		, @intCashBalance = @intCashBalanceBefore OUTPUT;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	BEGIN TRANSACTION;

	IF @intCashBalance > @intCashBalanceBefore
		INSERT dbo.CashIncome (AccountID, CashIncomeCode, PGCode, PGKey, CashAmount, IncomeDate)
		VALUES (@intAccountID, 2, NULL, NULL, @intCashBalance - @intCashBalanceBefore, GETDATE());

	ELSE IF @intCashBalance < @intCashBalanceBefore
		INSERT dbo.CashOutgo (AccountID, CashOutgoCode, PurchaseOrderID, CashAmount, OutgoDate)
		VALUES (@intAccountID, 3, NULL, @intCashBalanceBefore - @intCashBalance, GETDATE());

	EXEC @intReturnValue = dbo.P_GetCashBalance
		  @intAccountID = @intAccountID
		, @nvcAccountName = NULL
		, @intCashBalance = @intCashBalanceAfter OUTPUT;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	IF @intCashBalanceAfter <> @intCashBalance
	BEGIN
		SET @intReturnValue = 101199;
		GOTO ErrorHandler;
	END

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
