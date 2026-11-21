USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetSaleAmount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetSaleAmount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetSaleAmount] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-29
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetSaleAmount]
	  @dt2StartDate datetime2(0) --// ?? ?? ?? (UTC ??)
	, @dt2EndDate datetime2(0) --// ?? ? ?? (UTC ??)
	, @inbSaleAmount bigint OUTPUT --// ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intOffset int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intOffset = DATEDIFF(MINUTE, GETUTCDATE(), GETDATE());

	SELECT @inbSaleAmount = SUM(Price)
	FROM dbo.PurchaseOrders
	WHERE PayMethodCode = 2 AND OrderStatusCode = 2
		AND OrderDate > DATEADD(MINUTE, @intOffset, @dt2StartDate) AND OrderDate <= DATEADD(MINUTE, @intOffset, @dt2EndDate);
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
