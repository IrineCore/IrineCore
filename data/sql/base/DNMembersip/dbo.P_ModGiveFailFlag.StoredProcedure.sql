USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGiveFailFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModGiveFailFlag]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModGiveFailFlag] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-10
description : ??? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101149 = ?? ?? ???? ????.
history :
2 = ? ?? ???? ?? at 2010-10-27
**/

ALTER PROCEDURE [dbo].[P_ModGiveFailFlag]
	  @inbPurchaseOrderID bigint --// ?? ?? ID
	, @intProductID int --// ?? ID
	, @intItemID int --// ??? ID]
	, @bitGiveFailFlag bit --// 0=??? ?? ??, 1=??? ?? ??
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
	UPDATE dbo.SelectedItems
	SET GiveFailFlag = @bitGiveFailFlag
	FROM dbo.SelectedItems SI
		INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON SI.PurchaseOrderDetailID = PD.PurchaseOrderDetailID
	WHERE PD.PurchaseOrderID = @inbPurchaseOrderID AND PD.ProductID = @intProductID AND SI.ItemID = @intItemID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101149;
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
