USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListItemOfGift]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListItemOfGift]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListItemOfGift] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-13
description : ?? ?? ??? ?????. (??? ??? ??? ???? ?? ???? ?????.)
result set : 
	version = 1 : ProductID/int, ListPrice/int, ItemID/int
	version = 2 : ProductID/int, ListPrice/int, ItemID/int, ItemOption/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101154 = ?? ??? ???? ????.
history :
2 = input param. @intVersion ?? & ?? ?? ItemOption ?? at 2011-10-04
**/

ALTER PROCEDURE [dbo].[P_GetListItemOfGift]
	  @inbPurchaseOrderID bigint --// ??? ?? ?? ID
	, @intVersioin int = 1 --// ??? SP? ??
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
	/**_# ?? ??? ?????.*/
	IF @intVersioin = 1
		SELECT PD.ProductID, PD.ListPrice, SI.ItemID
		FROM dbo.PurchaseOrderDetails PD WITH (NOLOCK)
			INNER JOIN dbo.SelectedItems SI WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
		WHERE PD.PurchaseOrderID = @inbPurchaseOrderID;

	ELSE IF @intVersioin = 2
		SELECT PD.ProductID, PD.ListPrice, SI.ItemID, SI.ItemOption
		FROM dbo.PurchaseOrderDetails PD WITH (NOLOCK)
			INNER JOIN dbo.SelectedItems SI WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
		WHERE PD.PurchaseOrderID = @inbPurchaseOrderID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101154;
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
