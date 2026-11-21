USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCashBalance]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCashBalance]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCashBalance] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-07-11
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetCashBalance]
	  @intAccountID int --// ??? ?? ID
	, @nvcAccountName nvarchar(50) --// ??? ?? ??
	, @intCashBalance int = NULL OUTPUT --// ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbCashIncomeAmount bigint = 0
	, @inbCashOutgoAmount bigint = 0;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intAccountID <= 0 OR @intAccountID IS NULL
		SELECT @intAccountID = AccountID
		FROM dbo.Accounts WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK)
		WHERE AccountName = @nvcAccountName
		OPTION (KEEPFIXED PLAN);

	SELECT @inbCashIncomeAmount = CashAmount
	FROM dbo.VIX_CashIncomeTotals WITH (NOLOCK, NOEXPAND)
	WHERE AccountID = @intAccountID;

	SELECT @inbCashOutgoAmount = CashAmount
	FROM dbo.VIX_CashOutgoTotals WITH (NOLOCK, NOEXPAND)
	WHERE AccountID = @intAccountID;

	SET @intCashBalance = @inbCashIncomeAmount - @inbCashOutgoAmount;
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
