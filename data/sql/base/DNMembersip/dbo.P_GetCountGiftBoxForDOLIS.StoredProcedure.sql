USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountGiftBoxForDOLIS]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCountGiftBoxForDOLIS]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCountGiftBoxForDOLIS] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-09
description : ??/?? ???? ?? ??? ??????? ??????.
result set : RecordCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetCountGiftBoxForDOLIS]
	  @inbCharacterID bigint = NULL --// ?? ?? ??? ID
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
	SET @nvcStmt = N'
SELECT COUNT(*) AS RecordCount
FROM dbo.PurchaseOrders PO WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'PO.CharacterID', N'@inbCharacterID', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.ReceiverCharacterID', N'@inbReceiverCharacterID', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.GiftReceiveFlag', N'@bitGiftReceiveFlag', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.CouponKey', N'@nvcCouponKey', @inbNullBitmap, 4, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PO.OrderDate', N'@dt2StartOrderDate', @inbNullBitmap, 5, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'PO.OrderDate', N'@dt2EndOrderDate', @inbNullBitmap, 6, @intColumnCount, '<=')
		+ dbo.FN_DynamicWhereClause2(N'PO.PayMethodCode', N'@@inyPayMethodCode', @inbNullBitmap, 7, @intColumnCount, '=')
		+ CASE
			WHEN @inyGiftExpireFlag IS NULL THEN N''
			WHEN @inyGiftExpireFlag = 0 THEN N' AND (PO.GiftExpireDate IS NULL OR PO.GiftExpireDate > @dt2Now)'
			WHEN @inyGiftExpireFlag = 1 THEN N' AND PO.GiftExpireDate < @dt2Now'
		END + N'
	AND PO.GiftFlag = CAST(1 AS bit)
	AND PO.OrderStatusCode = 2
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
		'
		, @inbCharacterID = @inbCharacterID
		, @inbReceiverCharacterID = @inbReceiverCharacterID
		, @bitGiftReceiveFlag = @bitGiftReceiveFlag
		, @nvcCouponKey = @nvcCouponKey
		, @dt2StartOrderDate = @dt2StartOrderDate
		, @dt2EndOrderDate = @dt2EndOrderDate
		, @dt2Now = @dt2Now
		, @inyPayMethodCode = @inyPayMethodCode;
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
