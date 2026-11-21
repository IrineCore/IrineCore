USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGiftBoxForDOLIS]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetPageGiftBoxForDOLIS]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetPageGiftBoxForDOLIS] AS' 
END
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-09
description : ??/?? ???? ?? ??? ??????.
result set : PurchaseOrderID, SenderAccountName, SenderCharacterID, SenderCharacterName, SenderCharacterDeleteFlag, ReceiverCharacterID, ReceiverCharacterName, ReceiverCharacterDeleteFlag, Price, IP, OrderDate, PayMethodCode, GiftReceiveFlag, GiftMemo, GiftExpireDate, CouponKey, RefundFlag, RefundDate, GiftRejectFlag, GiftRejectDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101159 = ??? ?? ??? ????.
history :
5 = ?? ??? ????? ?? at 2010-08-25
7 = GiftRejectFlag, GiftRejectDate ??? ?? ????? ?? at 2010-10-14
9 = RefundFlag, RefundDate ??? ??? ?? ?? at 2010-11-02
**/

ALTER PROCEDURE [dbo].[P_GetPageGiftBoxForDOLIS]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @inbCharacterID bigint = NULL --// ?? ?? ??? ID
	, @inbReceiverCharacterID bigint = NULL --// ?? ?? ??? ID
	, @bitGiftReceiveFlag bit --// 0=??? ?? ?? ??, 1=??? ??
	, @nvcCouponKey nvarchar(128) = NULL --// ???
	, @dt2StartOrderDate datetime2(0) = NULL --// ?? ?? ?? - ?? ??
	, @dt2EndOrderDate datetime2(0) = NULL --// ?? ? ?? - ?? ??
	, @inyGiftExpireFlag bit = NULL --// 0=??????, 1=???
	, @inyPayMethodCode tinyint = NULL --// 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int,
	@dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @intColumnCount = 7;

	SET @inbNullBitmap = 
		CASE WHEN @inbCharacterID IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @inbReceiverCharacterID IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @bitGiftReceiveFlag IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @nvcCouponKey IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @dt2StartOrderDate IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @dt2EndOrderDate IS NOT NULL THEN POWER(2, 5) ELSE 0 END + 
		CASE WHEN @inyPayMethodCode IS NOT NULL THEN POWER(2, 6) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY PO.PurchaseOrderID DESC) AS RowNumber
		, PO.PurchaseOrderID, PO.CharacterID, PO.ReceiverCharacterID, PO.Price, dbo.FN_IPNumber2IP(PO.IP) AS IP, PO.OrderDate
		, PO.PayMethodCode, PO.GiftReceiveFlag, PO.GiftMemo, PO.GiftExpireDate, PO.GiftSenderName, PO.CouponKey, PD.RefundFlag, PD.RefundDate
		, PO.GiftRejectFlag, PO.GiftRejectDate
	FROM dbo.PurchaseOrders PO WITH (NOLOCK)
		INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
'		+ dbo.FN_DynamicWhereClause2(N'PO.CharacterID', N'@inbCharacterID', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.ReceiverCharacterID', N'@inbReceiverCharacterID', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.GiftReceiveFlag', N'@bitGiftReceiveFlag', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.CouponKey', N'@nvcCouponKey', @inbNullBitmap, 4, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.OrderDate', N'@dt2StartOrderDate', @inbNullBitmap, 5, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'PO.OrderDate', N'@dt2EndOrderDate', @inbNullBitmap, 6, @intColumnCount, '<=')
		+ dbo.FN_DynamicWhereClause2(N'PO.PayMethodCode', N'@inyPayMethodCode', @inbNullBitmap, 7, @intColumnCount, '=')
		+ CASE
			WHEN @inyGiftExpireFlag IS NULL THEN N''
			WHEN @inyGiftExpireFlag = 0 THEN N' AND (PO.GiftExpireDate IS NULL OR PO.GiftExpireDate > @dt2Now)'
			WHEN @inyGiftExpireFlag = 1 THEN N' AND PO.GiftExpireDate < @dt2Now'
		END + N'
		AND PO.GiftFlag = CAST(1 AS bit)
		AND PO.OrderStatusCode = 2
)
SELECT PC.PurchaseOrderID, AC1.AccountName AS SenderAccountName, CH1.CharacterID AS SenderCharacterID, COALESCE(CH1.CharacterName, DC1.CharacterName, PC.GiftSenderName) AS SenderCharacterName, CH1.DeleteFlag AS SenderCharacterDeleteFlag
	, CH2.CharacterID AS ReceiverCharacterID, COALESCE(CH2.CharacterName, DC2.CharacterName, PC.GiftSenderName) AS ReceiverCharacterName, CH2.DeleteFlag AS ReceiverCharacterDeleteFlag
	, PC.Price, PC.IP, PC.OrderDate, PC.PayMethodCode, PC.GiftReceiveFlag, PC.GiftMemo, PC.GiftExpireDate, PC.CouponKey, PC.RefundFlag, PC.RefundDate, PC.GiftRejectFlag, PC.GiftRejectDate
FROM PagingCTE PC
	LEFT OUTER JOIN dbo.Characters CH1 WITH (NOLOCK) ON PC.CharacterID = CH1.CharacterID
	LEFT OUTER JOIN dbo.DeletedCharacters DC1 WITH (NOLOCK) ON PC.CharacterID = DC1.CharacterID
	LEFT OUTER JOIN dbo.Accounts AC1 WITH (NOLOCK) ON CH1.AccountID = AC1.AccountID
	LEFT OUTER JOIN dbo.Characters CH2 WITH (NOLOCK) ON PC.ReceiverCharacterID = CH2.CharacterID
	LEFT OUTER JOIN dbo.DeletedCharacters DC2 WITH (NOLOCK) ON PC.ReceiverCharacterID = DC2.CharacterID
WHERE PC.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY PC.RowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @inbCharacterID bigint
			, @inbReceiverCharacterID bigint
			, @bitGiftReceiveFlag bit
			, @nvcCouponKey nvarchar(128)
			, @dt2StartOrderDate datetime2(0)
			, @dt2EndOrderDate datetime2(0)
			, @dt2Now datetime2(0)
			, @inyPayMethodCode tinyint
			, @intPageNumber int
			, @intPageSize int
		'
		, @inbCharacterID = @inbCharacterID
		, @inbReceiverCharacterID = @inbReceiverCharacterID
		, @bitGiftReceiveFlag = @bitGiftReceiveFlag
		, @nvcCouponKey = @nvcCouponKey
		, @dt2StartOrderDate = @dt2StartOrderDate
		, @dt2EndOrderDate = @dt2EndOrderDate
		, @dt2Now = @dt2Now
		, @inyPayMethodCode = @inyPayMethodCode
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, 101159? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101159;
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
