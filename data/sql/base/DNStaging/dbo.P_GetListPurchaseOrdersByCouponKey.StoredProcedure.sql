USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPurchaseOrdersByCouponKey]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-16
description :
    ??? ?? ??? ?? ??? ?????. (????? ?? ?? ?? ? ?? ??? ???? ?? ??)
result set : 
    OrderDate       datetime2(0)
    CouponKey       nvarchar(128)
    AccountID       int
    AccountName     nvarchar(50)
    CharacterID     bigint
    CharacterName	nvarchar(30)
    ItemID          int
    ItemName        nvarchar(100)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPurchaseOrdersByCouponKey]
      @dt2StartOrderDate datetime2(0) --// ?? ?? (???)
    , @dt2EndOrderDate datetime2(0) --// ?? ?? (???)
WITH EXECUTE AS 'SPExecutor'
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

    /**_# ??? ?? ??? ?? ??? ?????. */
    SELECT PO.OrderDate         -- ?? ????
         , PO.CouponKey         -- ?? ??
         , DA.AccountID         -- ??ID
         , DA.AccountName       -- ???
         , DC.CharacterID       -- ???ID
         , DC.CharacterName     -- ????
         , DI.ItemID            -- ??ID
         , DI.ItemName          -- ???
      FROM dbo.PurchaseOrders PO WITH (NOLOCK)
           INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON (PO.PurchaseOrderID = POD.PurchaseOrderID)
           INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (PO.CharacterID = DC.CharacterID)
           INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DC.AccountID = DA.AccountID)
           INNER JOIN dbo.DimItems DI WITH (NOLOCK) ON (POD.ProductID = DI.ItemID)
     WHERE PO.OrderDate >= @dt2StartOrderDate
       AND PO.OrderDate <= @dt2EndOrderDate

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
