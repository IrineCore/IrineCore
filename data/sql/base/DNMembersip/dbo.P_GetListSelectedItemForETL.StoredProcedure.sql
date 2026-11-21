USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListSelectedItemForETL]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListSelectedItemForETL]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListSelectedItemForETL] AS' 
END
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-07
description : ?? ??? ??? ??? ?????.
result set : SelectedItemID, PurchaseOrderDetailID, ItemID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4~5 = ItemOption ??     by 20111018.wgkim
**/

ALTER PROCEDURE [dbo].[P_GetListSelectedItemForETL]
	  @dtmBasisDate datetime --// ?? ??
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
	SELECT SI.SelectedItemID, SI.PurchaseOrderDetailID, SI.ItemID, SI.ItemOption
	FROM dbo.PurchaseOrders PO WITH (NOLOCK)
		INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		INNER JOIN dbo.SelectedItems SI WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
	WHERE PO.OrderDate >= CAST(@dtmBasisDate AS datetime2(0)) AND PO.OrderDate < CAST(DATEADD(DAY, 1, @dtmBasisDate) AS datetime2(0))
		AND PO.PayMethodCode IN (2, 3, 4, 9)
	OPTION (MAXDOP 1)	
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
