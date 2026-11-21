USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_SetPurchaseOrderResult]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_SetPurchaseOrderResult]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_SetPurchaseOrderResult] AS' 
END
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ?? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101144 = @inyOrderStatusCode ? ??
101145 = ??? ?? ??? ???? ????.
history :
5 = ??? ?? ?? ??? ?? Condext_ID ?? input param. ?? ?? at 2010-08-04
9 = input param. @bitDeductCashFlag ?? by ??? at 2013-04-05
**/

ALTER PROCEDURE [dbo].[P_SetPurchaseOrderResult]
	  @inbPurchaseOrderID bigint --// ??? ?? ?? ID
	, @inyOrderStatusCode tinyint --// 1=???? & pick up ??, 2=?? ??, 3=?? ?? ??
	, @vchOrderKey varchar(32) --// ??? ??? ??, ????? ??? ?? ?? ?.
	, @intOrderResult int --// ????? ??? ?? ?? ?
	, @intPetalAmount int --// ??? ??? ??, ??? ?? ??
	, @vchContextKey varchar(32) = NULL --// ??? ??? ??, ????? ??? ???? ?? ?? ?.
	, @intPaidCashAmount int = NULL --// ??? ??? ?? ?? ????
	, @bitDeductCashFlag bit = 0 --// CashIncome, CashOoutgo ???? ???? ?? ????? ?? (???=1)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2OrderDate datetime2(0)
	, @inbCharacterID bigint
	, @intAccountID int
	, @intCashAmount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyOrderStatusCode NOT IN (1, 2, 3)
	BEGIN
		SET @intReturnValue = 101144;
		GOTO ErrorHandler;
	END

	IF @inyOrderStatusCode = 2 AND @bitDeductCashFlag = 1
	BEGIN
		SELECT @intCashAmount = Price, @inbCharacterID = CharacterID
		FROM dbo.PurchaseOrders WITH (NOLOCK)
		WHERE PurchaseOrderID = @inbPurchaseOrderID
			AND OrderStatusCode = 1;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101145;
			GOTO ErrorHandler;
		END

		SELECT @intAccountID = AccountID
		FROM dbo.Characters WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

		EXEC @intReturnValue = dbo.P_DeductCash
			  @intAccountID = @intAccountID
			, @inbPurchaseOrderID = @inbPurchaseOrderID
			, @intCashAmount = @intCashAmount;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	UPDATE dbo.PurchaseOrders
	SET OrderStatusCode = @inyOrderStatusCode
		, OrderKey = 
			CASE @inyOrderStatusCode
				WHEN 2 THEN 
					CASE WHEN OrderKey IS NULL THEN @vchOrderKey ELSE OrderKey END
				ELSE @vchOrderKey
			END
		, ContextKey = 
			CASE @inyOrderStatusCode
				WHEN 2 THEN 
					CASE WHEN ContextKey IS NULL THEN @vchContextKey ELSE ContextKey END
				ELSE @vchContextKey
			END
		, OrderResult = @intOrderResult
		, @dt2OrderDate = OrderDate
		, @inbCharacterID = CharacterID
		, PaidCashAmount = ISNULL(@intPaidCashAmount, Price)
	WHERE PurchaseOrderID = @inbPurchaseOrderID AND OrderStatusCode = 1;

	IF @inyOrderStatusCode = 2 AND @intPetalAmount > 0
		INSERT dbo.PetalIncome (AccountID, PetalIncomeCode, PurchaseOrderID, AuditLogID, PetalAmount, IncomeDate)
		SELECT AccountID, 1, @inbPurchaseOrderID, NULL, @intPetalAmount, @dt2OrderDate
		FROM dbo.Characters WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

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
