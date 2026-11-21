USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListAuctionItemsHistory]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-02-08

description :
	?? ??? ?? ? ?? ?? ??
result set : 
	@intReturnRowCount / int 

	AuctionItemsHistoryID / int
	AuctionItemsHistoryStatusCode / tinyint
	AuctionItemsHistoryStatusFKey / int
	Coin / int
	RegisterDate / datetime

	ItemSerial / bigint
	ItemID / int
	ItemRemainCount / smallint
	PayMethodCode / tinyint
	SenderCharacterID / bigint
	ItemMaterializeCode / tinyint
	ItemMaterializeFKey / bigint
	ItemDurability / smallint
	RandomSeed / int
	CoolTime / int
	ItemLevel / tinyint
	ItemPotential / tinyint
	SoulBoundFlag / bit
	SealCount / tinyint
	ItemOption / tinyint
	ItemMaterializeDate / datetime2(0)
	ItemLifespan / int
	EternityFlag / bit
	ItemExpireDate / smalldatetime
	OwnerCharacterID / bigint
	OwnershipStartDate / datetime2(0)
	ExpireCompleteFlag / bit
	TradeItemFlag / bit
	AdditiveItemID / int
	ItemLocationCode / tinyint
	ItemPotentialMoveCount / tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	8 = ?????? ?? ??     by ??? at 2012-04-12
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListAuctionItemsHistory]
	  @inyAuctionItemsHistoryStatusCode tinyint --// 0=?? (???? ??? ?? : ????? + ??????), 1=?? (???? ??? ?? : ?????), 2=?? (???? ??? ?? : ??????), 3=??:????? (???? ??? ?? : ??????+?????), 4=??:????? (???? ??? ?? : ?????), 5=??:????? (???? ??? ?? : ??????), 6=??:????? (???? ??, ???? ??? ?? : ??????), 7=0? ?? ??(??), 8=1? ?? ??(??), 9=2? ?? ??(??), 10=3? ?? ??(??), 11=4? ?? ??(??), 12=5? ?? ??(??), 13=6? ?? ??(??)
	, @intAuctionItemsHistoryStatusFKey int --// @inyAuctionItemsHistoryStatusCode=(0,1) : AuctionID, @inyAuctionItemsHistoryStatusCode=(2) : BiddingID, @inyAuctionItemsHistoryStatusCode=(3,4,5,6) : ContractID

	, @intReturnRowCount int OUTPUT --// ???? ?? ??

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

	-- ?? ??? ?? ? ?? ?? ??
	SELECT AuctionItemsHistoryID, AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate, ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount
	  FROM dbo.AuctionItemsHistory WITH (NOLOCK)
	 WHERE AuctionItemsHistoryStatusCode = @inyAuctionItemsHistoryStatusCode
	   AND AuctionItemsHistoryStatusFKey = @intAuctionItemsHistoryStatusFKey;

	SET @intReturnRowCount = @@ROWCOUNT;

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
