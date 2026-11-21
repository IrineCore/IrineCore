USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPurchaseOrder]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListPurchaseOrder]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListPurchaseOrder] AS' 
END
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-09
description : ?? ??? ?????.
result set : 
	PurchaseOrderID, OrderDate, OrderKey, AccountID, AccountName, WorldID, CharacterID, CharacterName, GiftFlag, ProductID, Price, PetalIncomeCode, GiftReceiveFlag, ReceiverCharacterID, GiftRejectFlag, ReceiverCharacterName, IP
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = ?? ??? ????? ?? at 2010-08-25
4 = version 2? ?? at 2010-08-25
**/

ALTER PROCEDURE [dbo].[P_GetListPurchaseOrder]
	  @dt2StartOrderDate datetime2(0) --// ?? ?? ??.
	, @dt2EndOrderDate datetime2(0) --// ?? ? ??.
	, @inyPayMethodCode tinyint --// 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??
	, @inyPetalIncomeCode tinyint = NULL  --// (optional) ??? ?? ??? ??, ?? ??. 1=??, 2=??? ??, 3=??? ?? ??, 4=???, 5=??? ??? ?? ??
	, @nvcAccountName nvarchar(50) = NULL  --// (optional) ??? ?? ??
	, @nvcProductName nvarchar(100) = NULL  --// (optional) ?? ??
	, @intProductID int = NULL  --// (optional) ?? ID
	, @vchOrderKey varchar(32) = NULL --// (optional) ?? ? (????? ??? ?? ?? ID)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @intColumnCount = 8;

	SET @inbNullBitmap = 
		CASE WHEN @dt2StartOrderDate IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @dt2EndOrderDate IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @inyPayMethodCode IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @inyPetalIncomeCode IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @nvcAccountName IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @nvcProductName IS NOT NULL THEN POWER(2, 5) ELSE 0 END + 
		CASE WHEN @intProductID IS NOT NULL THEN POWER(2, 6) ELSE 0 END + 
		CASE WHEN @vchOrderKey IS NOT NULL THEN POWER(2, 7) ELSE 0 END;

	SET @nvcStmt = N'
WITH PTO AS (
	SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
	FROM dbo.PetalOutgo WITH (NOLOCK)
	GROUP BY PurchaseOrderID
)
SELECT PO.PurchaseOrderID, PO.OrderDate, PO.OrderKey, DA.AccountID, DA.AccountName, DC.WorldID, DC.CharacterID, DC.CharacterName, PO.GiftFlag, POD.ProductID
	, POD.ListPrice AS Price, PTI.PetalIncomeCode, PO.GiftReceiveFlag, PO.ReceiverCharacterID, PO.GiftRejectFlag, CH.CharacterName AS ReceiverCharacterName, dbo.FN_IPNumber2IP(PO.IP) AS IP
FROM dbo.PurchaseOrders PO WITH (NOLOCK)
	INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
	INNER JOIN dbo.Characters DC WITH (NOLOCK) ON PO.CharacterID = DC.CharacterID
	LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON PO.ReceiverCharacterID = CH.CharacterID
	INNER JOIN dbo.Accounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
	LEFT OUTER JOIN PTO WITH (NOLOCK) ON PO.PurchaseOrderID = PTO.PurchaseOrderID
	LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
'		+ dbo.FN_DynamicWhereClause2(N'PO.OrderDate', N'@dt2StartOrderDate', @inbNullBitmap, 1, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'PO.OrderDate', N'@dt2EndOrderDate', @inbNullBitmap, 2, @intColumnCount, '<=')
		+ dbo.FN_DynamicWhereClause2(N'PO.PayMethodCode', N'@inyPayMethodCode', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PTI.PetalIncomeCode', N'@inyPetalIncomeCode', @inbNullBitmap, 4, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'DA.AccountName', N'@nvcAccountName', @inbNullBitmap, 5, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'DP.ProductName', N'@nvcProductName', @inbNullBitmap, 6, @intColumnCount, '%A%')
		+ dbo.FN_DynamicWhereClause2(N'POD.ProductID', N'@intProductID', @inbNullBitmap, 7, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.OrderKey', N'@vchOrderKey', @inbNullBitmap, 8, @intColumnCount, '=')
 + N'
	AND PO.OrderStatusCode = 2
ORDER BY PO.PurchaseOrderID';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'@dt2StartOrderDate datetime2(0)
		, @dt2EndOrderDate datetime2(0)
		, @inyPayMethodCode tinyint
		, @inyPetalIncomeCode tinyint
		, @nvcAccountName nvarchar(50)
		, @nvcProductName nvarchar(100)
		, @intProductID int
		, @vchOrderKey varchar(32)'
		, @dt2StartOrderDate = @dt2StartOrderDate
		, @dt2EndOrderDate = @dt2EndOrderDate
		, @inyPayMethodCode = @inyPayMethodCode
		, @inyPetalIncomeCode = @inyPetalIncomeCode
		, @nvcAccountName = @nvcAccountName
		, @nvcProductName = @nvcProductName
		, @intProductID = @intProductID
		, @vchOrderKey = @vchOrderKey;
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
