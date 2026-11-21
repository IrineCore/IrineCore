USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_SpaceManagement]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_SpaceManagement]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_SpaceManagement] AS' 
END
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-12-07
description : Garbage Data ??? ???(?? ???? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_SpaceManagement]
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
	/**_# ?? ?? ?? */
	TRUNCATE TABLE dbo.DNAuth;

	/**_# ??? ?? ??? ?? ?? ?? ?? */
	DECLARE @Temp_PurchaseOrderDetails TABLE (PurchaseOrderDetailID bigint);
	DECLARE @Temp_PurchaseOrders TABLE (PurchaseOrderID bigint);

	DELETE dbo.SelectedItems
	OUTPUT deleted.PurchaseOrderDetailID
	  INTO @Temp_PurchaseOrderDetails (PurchaseOrderDetailID)
	  FROM dbo.SelectedItems C
		   INNER JOIN dbo.PurchaseOrderDetails B ON B.PurchaseOrderDetailID = C.PurchaseOrderDetailID
		   INNER JOIN dbo.PurchaseOrders A ON A.PurchaseOrderID = B.PurchaseOrderID
	 WHERE A.GiftFlag = CAST(1 AS bit)
	   AND (A.GiftReceiveFlag = CAST(1 AS bit) OR A.GiftExpireDate < GETDATE())
	   AND A.PayMethodCode IN (5, 6, 7, 8, 15, 17, 18, 19, 20, 21)
	   AND A.OrderDate < DATEADD(DAY, - 7, CAST(CAST(GETDATE() AS date) AS datetime2(0)))
	   AND NOT EXISTS (SELECT * FROM dbo.VIPPointLogs WHERE PurchaseOrderID = A.PurchaseOrderID)
	   AND NOT EXISTS (SELECT * FROM dbo.PetalIncome WHERE PurchaseOrderID = A.PurchaseOrderID);

	DELETE dbo.PurchaseOrderDetails
	OUTPUT deleted.PurchaseOrderID
	  INTO @Temp_PurchaseOrders (PurchaseOrderID)
	  FROM dbo.PurchaseOrderDetails P
		   INNER JOIN @Temp_PurchaseOrderDetails T ON (P.PurchaseOrderDetailID = T.PurchaseOrderDetailID);

	DELETE dbo.PurchaseOrders
	  FROM dbo.PurchaseOrders P
		   INNER JOIN @Temp_PurchaseOrders T ON (P.PurchaseOrderID = T.PurchaseOrderID);

	/**_# 30? ?? ?? ?? ?? ?? */
	-- ???? ?? ??
	DELETE dbo.GiftSendScheduleTarget
	  FROM dbo.GiftSendScheduleTarget A
		   INNER JOIN dbo.GiftSendSchedule B ON (A.GiftSendScheduleID = B.GiftSendScheduleID)
	 WHERE B.GiftSendDate < CONVERT(smalldatetime, DATEADD(day, -30, GETDATE()));

	-- ???? ?? ??
	DELETE dbo.GiftSendScheduleReceive
	  FROM dbo.GiftSendScheduleReceive A
		   INNER JOIN dbo.GiftSendSchedule B ON (A.GiftSendScheduleID = B.GiftSendScheduleID)
	 WHERE B.GiftSendDate < CONVERT(smalldatetime, DATEADD(day, -30, GETDATE()));

	-- ????
	DELETE dbo.GiftSendSchedule
	 WHERE GiftSendDate < CONVERT(smalldatetime, DATEADD(day, -30, GETDATE()));

	/**_# ?? ??? ??? ?? ? ?? ?? */
	DELETE dbo.MobileAuthentication WHERE AccountAuthenticationKey IS NOT NULL;

	/**_# ?????? ??? IP ? ?? ? ??? */
	DELETE dbo.ClientIPConnectCounts;


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
