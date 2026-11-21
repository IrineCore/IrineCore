USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPurchaseOrderDetail]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListPurchaseOrderDetail]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListPurchaseOrderDetail] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-29
description : ?? ??? ?? ?? ?? ?? (??, ?? ???)? ?????.
result set : 
	version = 1 : ProductID/int, ItemID/int
	version = 2 : ProductID/int, ItemID/int, ItemOption/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101145 = ??? ?? ??? ???? ????.
history :
2 = input param. @intVersion ?? & ?? ?? ItemOption ?? at 2011-10-04
**/

ALTER PROCEDURE [dbo].[P_GetListPurchaseOrderDetail]
	  @inbPurchaseOrderID bigint --// ??? ?? ?? ID
	, @intVersion int = 1 --// ??? SP? ??
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
	IF @intVersion = 1
		SELECT POD.ProductID, SI.ItemID
		FROM dbo.PurchaseOrderDetails POD WITH (NOLOCK)
			INNER JOIN dbo.SelectedItems SI WITH (NOLOCK) ON POD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
		WHERE POD.PurchaseOrderID = @inbPurchaseOrderID
		ORDER BY SI.PurchaseOrderDetailID;

	ELSE IF @intVersion = 2
		SELECT POD.ProductID, SI.ItemID, SI.ItemOption
		FROM dbo.PurchaseOrderDetails POD WITH (NOLOCK)
			INNER JOIN dbo.SelectedItems SI WITH (NOLOCK) ON POD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
		WHERE POD.PurchaseOrderID = @inbPurchaseOrderID
		ORDER BY SI.PurchaseOrderDetailID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101145;
		GOTO ErrorHandler;
	END
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
