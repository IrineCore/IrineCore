USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[__NX__UpdateCashBalance]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[__NX__UpdateCashBalance]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[__NX__UpdateCashBalance] AS' 
END
GO
/**
0=?????
1=???????????????
100=???????????????dbo.ErrorLogs??
101197=??????
101,199=???????????????????????????????
**/

ALTER PROCEDURE [dbo].[__NX__UpdateCashBalance]
	  @nvcAccountName nvarchar(50) --// account name
	, @intUpdateCash int --// + or -
	, @intRemainCash int output
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intAccountID int
	, @intCashBalanceBefore int
	, @intCashBalanceAfter int
	, @intCashRemain int;

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

	SET @intCashRemain = @intCashBalanceBefore + @intUpdateCash;
	
	IF @intUpdateCash > 0
		INSERT dbo.CashIncome (AccountID, CashIncomeCode, PGCode, PGKey, CashAmount, IncomeDate)
		VALUES (@intAccountID, 2, NULL, NULL, @intUpdateCash, GETDATE());
	ELSE IF @intUpdateCash < 0
	BEGIN
        IF @intCashRemain > 0
        BEGIN
			INSERT dbo.CashOutgo (AccountID, CashOutgoCode, PurchaseOrderID, CashAmount, OutgoDate)
			VALUES (@intAccountID, 3, NULL, -@intUpdateCash, GETDATE());
		END
		ELSE 
		BEGIN
			INSERT dbo.CashOutgo (AccountID, CashOutgoCode, PurchaseOrderID, CashAmount, OutgoDate)
			VALUES (@intAccountID, 3, NULL, @intCashBalanceBefore, GETDATE());
		END
	END
	IF(@intCashRemain <0)
		SET @intCashRemain = 0;
		
	EXEC @intReturnValue = dbo.P_GetCashBalance
		  @intAccountID = @intAccountID
		, @nvcAccountName = NULL
		, @intCashBalance = @intCashBalanceAfter OUTPUT;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	IF @intCashBalanceAfter <> @intCashRemain
	BEGIN
		SET @intReturnValue = 101199;
		GOTO ErrorHandler;
	END
	
	COMMIT TRANSACTION;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

--OUTPUT
SET @intRemainCash = @intCashBalanceAfter
RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;
RETURN @intReturnValue;
GO
