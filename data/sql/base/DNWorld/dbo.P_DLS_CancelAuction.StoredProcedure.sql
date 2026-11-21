USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_CancelAuction]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-14
description : ??? ?????. (???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000025 = ?? ??? ???? ????.
1030000049 = ?? ??? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_CancelAuction]
	  @intAuctionID int --// ??ID
	, @nvcCancelReason nvarchar(300) --// ?? ?? ??
	, @nvcMailSubject nvarchar(50) --// ?? ?? ??(???)		
	, @nvcMailContent nvarchar(300) --// ?? ?? ??(???)
	, @nvcMailSenderName nvarchar(50) --// ?? ?? ????(???)
	, @nvcBidderMailSubject nvarchar(50) --// ?? ?? ??(???)		
	, @nvcBidderMailContent nvarchar(300) --// ?? ?? ??(???)
	, @nvcBidderMailSenderName nvarchar(50) --// ?? ?? ????(???)
	, @inyMailTabCode tinyint = NULL --// ?? ? ??	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inbLatestBiddingCharacterID bigint
	, @inbLatestBiddingPrice bigint
	, @inbItemSerial bigint
	, @inbSendMailID bigint
	, @intItemID int
	, @insItemCount smallint
	, @bitSoulBoundFlag bit
	, @inyItemPotentialMoveCount tinyint
	, @dt2OwnershipStartDate datetime2(0)
	, @inySealCount tinyint
	, @inyItemOption tinyint
	, @inyItemPotential tinyint
	, @inyItemLevel tinyint
	, @inbSellerCharacterID bigint
	, @inyAuctionStatusCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT @inyAuctionStatusCode = AuctionStatusCode
	FROM dbo.Auctions WITH (NOLOCK)
	WHERE AuctionID = @intAuctionID;

	IF @inyAuctionStatusCode = 4
	BEGIN
		SET @intReturnValue = 1030000049;
		GOTO ErrorHandler;
	END
	
	BEGIN TRANSACTION;

	/**_# [Auctions] ???? UPDATE???.*/
	UPDATE dbo.Auctions
	SET AuctionStatusCode = 4, @inbSellerCharacterID = CharacterID, @inbItemSerial = ItemSerial
	WHERE AuctionID = @intAuctionID;

	/**_# ?? ??? ??? ?????.*/
	SELECT @inbLatestBiddingCharacterID = CharacterID, @inbLatestBiddingPrice = BiddingPrice
	FROM dbo.AuctionBidding	
	WHERE AuctionID = @intAuctionID AND BiddingStatusCode = 0;
	IF @@ROWCOUNT = 1	
	BEGIN
		/**_# ?? ??? ???? ?? ??? ?? ??(????) ???.*/
		INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, DeleteDate, NewFlag, MailTabCode)
		SELECT NULL, @inbLatestBiddingCharacterID, 3, 1, @nvcBidderMailSubject, @nvcBidderMailContent, @dt2Now, 0, 0, @inbLatestBiddingPrice, 0, @nvcBidderMailSenderName, NULL, NULL, 0, @inyMailTabCode;			
	END

	/**_# ????? ??? ?? ???? ???? ?????.*/
	UPDATE dbo.MaterializedItems
	SET @intItemID = ItemID	
	    , @insItemCount = ItemRemainCount
		, @bitSoulBoundFlag = SoulBoundFlag
		, @inyItemPotentialMoveCount = ItemPotentialMoveCount
		, @dt2OwnershipStartDate = OwnershipStartDate
	    , @inySealCount = SealCount
	    , @inyItemOption = ItemOption
	    , @inyItemPotential = ItemPotential
	    , @inyItemLevel = ItemLevel
		, ItemLocationCode = 15
	    , ItemLocationIndex = NULL
	    , OwnerCharacterID = NULL	  
    WHERE ItemSerial = @inbItemSerial		
		AND ItemLocationCode = 22; 

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000025;
		GOTO ErrorHandler;
	END 

	INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, DeleteDate, NewFlag, MailTabCode)
	SELECT NULL, @inbSellerCharacterID, 3, 1, @nvcMailSubject, @nvcMailContent, @dt2Now, 0, 0, 0, 0, @nvcMailSenderName, NULL, NULL, 0, @inyMailTabCode;

	SET @inbSendMailID = SCOPE_IDENTITY();

	INSERT dbo.MailAttachItems (MailID, ItemSerial, ItemID, FirstAttachItemFlag, ReceiveFlag, AttachItemCount)
	SELECT @inbSendMailID, @inbItemSerial, @intItemID, 1, 0, @insItemCount;		

	/**_# ??? ?? ??? ???.*/
	UPDATE dbo.Auctions SET ItemCalculateFlag = CAST (1 AS bit) WHERE AuctionID = @intAuctionID;

	/**_# ????? ?????.*/
	IF NOT EXISTS (SELECT * FROM dbo.AuctionCancelReason WITH (NOLOCK) WHERE AuctionID = @intAuctionID)
		INSERT dbo.AuctionCancelReason (AuctionID, CancelReason, CancelDate)
		SELECT @intAuctionID, @nvcCancelReason, @dt2Now;
	ELSE
		UPDATE dbo.AuctionCancelReason
		SET CancelReason = @nvcCancelReason, CancelDate = @dt2Now
		WHERE AuctionID = @intAuctionID;

	COMMIT TRANSACTION;

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
