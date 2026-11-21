USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCashIncomeOutgo]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListCashIncomeOutgo]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListCashIncomeOutgo] AS' 
END
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-08-22
description : ?? ?? ??? ?????.
result set :  InOutType, CashIncomeCode, IncomeCashAmount, CashOutgoCode, OutgoCashAmount, PurchaseOrderID, LogDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListCashIncomeOutgo]
	  @dt2StartDate datetime2(0) --// ?? ??
	, @dt2EndDate datetime2(0) --// ?? ??
    , @intAccountID int --// ??? ??ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT 1 AS InOutType, CashIncomeCode AS CashIncomeCode, CashAmount AS IncomeCashAmount, NULL AS CashOutgoCode, NULL AS OutgoCashAmount, NULL AS PurchaseOrderID, IncomeDate AS LogDate
	FROM dbo.CashIncome WITH(NOLOCK, INDEX(IX_NC_CashIncome_IncomeDate), FORCESEEK)
	WHERE IncomeDate BETWEEN @dt2StartDate AND @dt2EndDate
      AND AccountID = @intAccountID
	UNION ALL
	SELECT 2, NULL, NULL, CashOutgoCode, CashAmount, PurchaseOrderID, OutgoDate AS LogDate
	FROM dbo.CashOutgo WITH(NOLOCK, INDEX(IX_NC_CashOutgo_OutgoDate), FORCESEEK)
	WHERE OutgoDate BETWEEN @dt2StartDate AND @dt2EndDate
      AND AccountID = @intAccountID
	ORDER By LogDate;

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
