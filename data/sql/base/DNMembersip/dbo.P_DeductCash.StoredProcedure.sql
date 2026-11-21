USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DeductCash]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DeductCash]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DeductCash] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-07-11
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101198 = ?? ??? ?????.
**/

ALTER PROCEDURE [dbo].[P_DeductCash]
	  @intAccountID int --// ??? ?? ID
	, @inbPurchaseOrderID bigint = NULL--// ??? ?? ?? ID(???? ???? ??? ??? ?????.)
	, @intCashAmount int --// ?? ??
	, @intCashOutgoID int = NULL OUTPUT --// ?? ?? ID
	, @intCashBalance int = NULL OUTPUT --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	INSERT dbo.CashOutgo (AccountID, CashOutgoCode, PurchaseOrderID, CashAmount, OutgoDate)
	VALUES (@intAccountID, CASE WHEN @inbPurchaseOrderID IS NOT NULL THEN 1 ELSE 3 END, @inbPurchaseOrderID, @intCashAmount, GETDATE());

	EXEC @intReturnValue = dbo.P_GetCashBalance
		  @intAccountID = @intAccountID
		, @nvcAccountName = NULL
		, @intCashBalance = @intCashBalance OUTPUT;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	IF @intCashBalance < 0
	BEGIN
		SET @intReturnValue = 101198;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	SET @intCashOutgoID = SCOPE_IDENTITY();
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
