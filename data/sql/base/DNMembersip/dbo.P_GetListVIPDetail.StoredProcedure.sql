USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListVIPDetail]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListVIPDetail]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListVIPDetail] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-07
description : VIP ?? ?? ??? ?????.
result set : OrderDate, ProductID, ListPrice, BasicPoint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101166 = VIP ?? ??? ????.
**/

ALTER PROCEDURE [dbo].[P_GetListVIPDetail]
	  @inbCharacterID bigint --// ??? ID
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
	SELECT PO.OrderDate, POD.ProductID, POD.ListPrice, VL.BasicPoint
	FROM dbo.VIPPointLogs VL WITH (NOLOCK)
		INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON VL.PurchaseOrderID = PO.PurchaseOrderID
		INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
	WHERE VL.CharacterID = @inbCharacterID
	ORDER BY POD.PurchaseOrderDetailID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101166;
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
