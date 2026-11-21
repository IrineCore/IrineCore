USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetMarketPriceItemInfo]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-27
description : ???? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetMarketPriceItemInfo]
	  @inbItemSerial bigint --// ??? ???
	, @intItemID int OUTPUT --// ??? ID
	, @intItemDurability int OUTPUT --// ??????
	, @inyItemLevel tinyint OUTPUT --// ??? ????
	, @inyItemPotential tinyint OUTPUT --// ??? ???
	, @inyItemOption tinyint OUTPUT --// ??? ??
	, @inySealCount tinyint OUTPUT --// ??? ????
	, @bitSoulBoundFlag bit OUTPUT --// ??? ????
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
	/**_# [MarketPrices] ???? SELECT???.*/
	SELECT @intItemID = ItemID, @intItemDurability = RepresentativeItemDurability, @inyItemLevel = ItemLevel, @inyItemOption = ItemOption
		 , @inySealCount = RepresentativeItemSealCount, @bitSoulBoundFlag = RepresentativeItemSoulBoundFlag, @inyItemPotential = RepresentativeItemPotential
	FROM dbo.MarketPrices WITH (NOLOCK, INDEX(IX_NNI_MarketPrices_RepresentativeItemSerial))
	WHERE RepresentativeItemSerial = @inbItemSerial;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
