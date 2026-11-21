USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_BuyTradeItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-12-27
description : ????? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000021 = ??? ???? ????.
1030000022 = ?? ???? ?? ???? ?? ?? ??? ? ????.	
1030000023 = ?? ??? ??????.
1030000024 = ??? ??? ?????.
1030000025 = ?? ??? ???? ????.
1030000047 = DWC ???? ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_BuyTradeItem]
	  @inbBuyerCharacterID bigint --// ?? ??? ID
	, @intTradeID int --// ?? ID	
	, @inbBuyCommission bigint --// ?? ???
	, @inbItemPrice bigint --// ??? ?? ??	
	, @nvcMailSubject nvarchar(50) --// ?? ?? 
	, @nvcMailContent nvarchar(300) --// ?? ??
	, @nvcMailSenderName nvarchar(50) --// ?? ??? ??	
	, @inyCharacterLevel tinyint = NULL OUTPUT -- // ??? ??
	, @intCharacterExp int = NULL OUTPUT -- // ??? ???
	, @inbCoin bigint = NULL OUTPUT -- // ??? ??
	, @inbWarehouseCoin bigint = NULL OUTPUT -- // ??? ????
	, @inyMailTabCode tinyint = NULL --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
      , @inbCharacterID bigint
      , @inyPayMethodCode tinyint
	  , @inyTradeStatusCode tinyint
      , @inbItemSerial bigint 
	  , @intItemID int 
	  , @insItemCount smallint 
	  , @insItemDurability smallint 
	  , @intRandomSeed int 
	  , @inyItemLevel tinyint 
	  , @inyItemPotential tinyint
	  , @inyItemOption tinyint
	  , @inySealCount tinyint
	  , @inbSellerCharacterID bigint
	  , @inbMaterializedItemID bigint
	  , @inbSendMailID bigint
	  , @bitSoulBoundFlag bit
	  , @inyItemPotentialMoveCount tinyint
	  , @dt2OwnershipStartDate datetime2(0)
	  ,	@inbChangedCoin bigint
	  ,	@inbCurrentCoin bigint
	  , @inyAccountLevelCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# DWC ????? ?????. */
	SELECT @inyAccountLevelCode = AccountLevelCode
	FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
	WHERE CharacterID = @inbBuyerCharacterID;

	IF @inyAccountLevelCode = 110
	BEGIN
		SET @intReturnValue = 1030000047;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;

	/**_# Trade ???? ???? ???? ???. */
	UPDATE dbo.Trade
	SET @inyTradeStatusCode = TradeStatusCode
		, @inbCharacterID = CharacterID
		, @inyPayMethodCode = ISNULL(PayMethodCode, 1)
		, @inbItemSerial = ItemSerial
		, @inbSellerCharacterID = CharacterID
	    , TradeStatusCode = 1
		, BuyerCharacterID = @inbBuyerCharacterID
		, TradeDate = @dt2Now
	WHERE TradeID = @intTradeID AND TradeStatusCode BETWEEN 0 AND 1;

	/**_# ?? ???? ?? ?? */
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000021;
		GOTO ErrorHandler;
	END

	/**_# ?? ???? ?? ???? ?? ?? */
	IF @inbCharacterID = @inbBuyerCharacterID
	BEGIN
		SET @intReturnValue = 1030000022;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ?? */
	IF @inyTradeStatusCode = 1
	BEGIN
		SET @intReturnValue = 1030000023;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ??? ?? */
	IF @inyPayMethodCode = 1 AND (
		SELECT Coin 
		FROM dbo.CharacterStatus WITH (NOLOCK) 
		WHERE CharacterID = @inbBuyerCharacterID
	) < (@inbBuyCommission + @inbItemPrice)
	BEGIN
		SET @intReturnValue = 1030000024;
		GOTO ErrorHandler;
	END

	/**_# ?? ????? ???? ?? ?????. */
	UPDATE dbo.MaterializedItems
	SET @intItemID = ItemID	
	    , @insItemCount = ItemRemainCount
		, @bitSoulBoundFlag = SoulBoundFlag
		, @inyItemPotentialMoveCount = ItemPotentialMoveCount
		, @dt2OwnershipStartDate = OwnershipStartDate
		, ItemLocationCode = 15
	    , ItemLocationIndex = NULL
	    , OwnerCharacterID = NULL	  
    WHERE ItemSerial = @inbItemSerial
		AND OwnerCharacterID = @inbSellerCharacterID
		AND ItemLocationCode = 12;  

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000025;
		GOTO ErrorHandler;
	END
	INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, DeleteDate, NewFlag, MailTabCode)
	SELECT NULL, @inbBuyerCharacterID, 3, 1, @nvcMailSubject, @nvcMailContent, @dt2Now, 0, 0, 0, 0, @nvcMailSenderName, NULL, NULL, 0, @inyMailTabCode;
	SET @inbSendMailID = SCOPE_IDENTITY();
	INSERT dbo.MailAttachItems (MailID, ItemSerial, ItemID, FirstAttachItemFlag, ReceiveFlag, AttachItemCount)
	SELECT @inbSendMailID, @inbItemSerial, @intItemID, 1, 0, @insItemCount;		

	/**_# ?? ???? ?????. */	
	UPDATE dbo.CharacterStatus
	SET @inbCurrentCoin = Coin
		, Coin = Coin - (@inbBuyCommission + @inbItemPrice)
		, @inyCharacterLevel = CharacterLevel
		, @intCharacterExp = CharacterExp
		, @inbCoin = Coin - (@inbBuyCommission + @inbItemPrice)
		, @inbWarehouseCoin = WarehouseCoin
	WHERE CharacterID = @inbBuyerCharacterID;	
	
	COMMIT TRANSACTION;

	/**_# ??? ??? ????. */
	EXEC @intReturnValue = dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 28
		, @inbOwnerCharacterID = @inbBuyerCharacterID
		, @inbOwnerCharacterIDBefore = @inbSellerCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = @intTradeID
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = NULL
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemLevel = @inyItemLevel
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount;

	/**_# ???? ??? ????. */

	SET @inbChangedCoin = (@inbBuyCommission + @inbItemPrice) * -1;
	SET @inbCurrentCoin = @inbCurrentCoin - (@inbBuyCommission + @inbItemPrice);

	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dt2Now
		, @inbCharacterID = @inbBuyerCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = 54
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @inbChangedCoin
		, @intChannelID = NULL
		, @intMapID = NULL
		, @inbCoinChangeFKey = NULL;

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
