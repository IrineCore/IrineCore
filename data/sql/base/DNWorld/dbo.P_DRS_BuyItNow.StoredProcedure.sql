USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_BuyItNow]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 18
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-28
description : ?? ??? ???? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000030 = ?? ?? ??? ?????.
1030000032 = ??? ??? ?????.
1030000033 = ??? ???? 1??? ?????.
1030000025 = ?? ??? ???? ????.
1030000043 = ???? ???(???)? ?????.
1030000047 = DWC ???? ???? ??? ? ????.
1030000048 = ???? ?? ??? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_BuyItNow]
	  @inbCharacterID bigint --// ?? ???ID
	, @intAccountID int --// ?? ??? AccountID
	, @intAuctionID int --// ??ID
	, @nvcMailSubject nvarchar(50) --// ?? ?? ??		
	, @nvcMailContent nvarchar(300) --// ?? ?? ??
	, @nvcMailSenderName nvarchar(50) --// ?? ?? ????
	, @nvcRefundMailSubject nvarchar(50) --// ?? ?? ??		
	, @nvcRefundMailContent nvarchar(300) --// ?? ?? ??
	, @nvcRefundMailSenderName nvarchar(50) --// ?? ?? ????
	, @inyCharacterLevel tinyint = NULL OUTPUT -- // ?? ??? ??
	, @intCharacterExp int = NULL OUTPUT -- // ?? ??? ???
	, @inbCoin bigint = NULL OUTPUT -- // ?? ??? ??
	, @inbWarehouseCoin bigint = NULL OUTPUT -- // ?? ??? ????		
	, @inyMailTabCode tinyint = NULL --// ?? ?? ? ??
	, @inyRefundMailTabCode tinyint = NULL --// ?? ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @sdtBiddingEndDate smalldatetime
	  , @inbBuyitNowPrice bigint
	  , @inbLatestBiddingCharacterID bigint
	  , @inbLatestBiddingPrice bigint	  
	  , @inbCharacterCoin bigint
	  , @inbChangedCoin bigint
	  , @inbItemSerial bigint
	  , @intItemID int
	  , @insItemCount smallint
	  , @bitSoulBoundFlag bit
	  , @inyItemPotentialMoveCount tinyint
	  , @dt2OwnershipStartDate datetime2(0)
	  , @inySealCount tinyint
	  , @inyItemOption tinyint
	  , @inyItemPotential tinyint
	  , @inyItemLevel tinyint
	  , @inbSendMailID bigint
	  , @inbSellerCharacterID bigint
	  , @inbSellerAccountID bigint
	  , @inyAccountLevelCode tinyint
	  , @inyAuctionStatusCode tinyint
	  , @inyDragonJewelType tinyint
	  , @intAuctionItemID int
	  , @intLastBidderCount int
	  , @intLatestBiddingAccountID int;


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# DWC???? ??? ?????.*/	
	SELECT @inyAccountLevelCode = AccountLevelCode
	FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
	WHERE CharacterID = @inbCharacterID;
	
	IF @inyAccountLevelCode = 110
	BEGIN
		SET @intReturnValue = 1030000047;
		GOTO ErrorHandler;
	END

	/**_# ??? ????? ???? ?????.*/	
	SELECT @sdtBiddingEndDate = BiddingEndDate, @inbBuyitNowPrice = BuynowPrice
		 , @inbItemSerial = ItemSerial, @inbSellerCharacterID = A.CharacterID
		 , @inbSellerAccountID = CH.AccountID, @inyAuctionStatusCode = A.AuctionStatusCode
	     , @intAuctionItemID = ItemID
	FROM dbo.Auctions A WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK)
		 INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON A.CharacterID = CH.CharacterID
	WHERE AuctionID = @intAuctionID;

	IF @sdtBiddingEndDate < @dt2Now
	BEGIN
		SET @intReturnValue = 1030000030;
		GOTO ErrorHandler;
	END	

	IF @inyAuctionStatusCode = 4 
	BEGIN
		SET @intReturnValue = 1030000048;
		GOTO ErrorHandler;
	END
	
	/**_# ???? ???? ???? ?????.*/	
	IF @inbSellerCharacterID = @inbCharacterID OR @inbSellerAccountID = @intAccountID
	BEGIN
		SET @intReturnValue = 1030000043;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ???? ??? ?????.*/	
	SELECT @inbCharacterCoin = Coin
	FROM dbo.CharacterStatus WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK)
	WHERE CharacterID = @inbCharacterID;

	IF @inbBuyitNowPrice > @inbCharacterCoin
	BEGIN
		SET @intReturnValue = 1030000032;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;
	
	/**_# ?? ??? ???? ?? ?? ?? ???.*/	
	UPDATE dbo.AuctionBidding
	SET BiddingStatusCode = 1, @inbLatestBiddingCharacterID = CharacterID, @inbLatestBiddingPrice = BiddingPrice
	WHERE AuctionID = @intAuctionID AND BiddingStatusCode = 0;

	SET @intLastBidderCount = @@ROWCOUNT;

	IF @intLastBidderCount > 1
	BEGIN
		SET @intReturnValue = 1030000033;
		GOTO ErrorHandler;
	END 
	ELSE IF @intLastBidderCount = 1
	BEGIN
		/**_# ?? ??? ???? AccountID? ?????.*/
		SELECT @intLatestBiddingAccountID = AccountID
		FROM dbo.Characters WITH (NOLOCK) WHERE CharacterID = @inbLatestBiddingCharacterID;
		
		/**_# ?? ??? ???? ?? ??? ?? ??(????) ???.*/
		INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, DeleteDate, NewFlag, MailTabCode)
		SELECT NULL, @inbLatestBiddingCharacterID, 3, 1, @nvcRefundMailSubject, @nvcRefundMailContent, @dt2Now, 0, 0, @inbLatestBiddingPrice, 0, @nvcRefundMailSenderName, NULL, NULL, 0, @inyRefundMailTabCode;			
	
		/**_# ?? ?? ??? ?????.*/
		INSERT dbo.PushQueue (WishListID, CharacterID, RegistedDate, PushTypeCode, ItemID, AccountID)
		SELECT NULL, @inbLatestBiddingCharacterID, @dt2Now, 2, @intAuctionItemID, @intLatestBiddingAccountID;
	END

	/**_# ??????? ????? ?????.*/	
	UPDATE dbo.Auctions SET AuctionStatusCode = 2, ItemCalculateFlag = CAST (1 AS bit), UpdateDate = @dt2Now  WHERE AuctionID = @intAuctionID;

	/**_# ?? ????? ???? ??? Insert ???.*/	
	INSERT dbo.AuctionBidding (AuctionID, CharacterID, BiddingPrice, BiddingStatusCode, BiddingDate)
	SELECT @intAuctionID, @inbCharacterID, @inbBuyitNowPrice, 2, @dt2Now;
	/**_# ?? ??? ?????.*/
	UPDATE dbo.CharacterStatus
	SET Coin = Coin - @inbBuyitNowPrice,
	    @inyCharacterLevel = CharacterLevel,
		@intCharacterExp = CharacterExp,
		@inbWarehouseCoin = WarehouseCoin,
	    @inbCoin = Coin - @inbBuyitNowPrice
	WHERE CharacterID = @inbCharacterID;	

	/**_# ?? ?? ???? ???????.*/
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
	    , @inyDragonJewelType = DragonJewelType
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
	SELECT NULL, @inbCharacterID, 3, 1, @nvcMailSubject, @nvcMailContent, @dt2Now, 0, 0, 0, 0, @nvcMailSenderName, NULL, NULL, 0, @inyMailTabCode;

	SET @inbSendMailID = SCOPE_IDENTITY();

	INSERT dbo.MailAttachItems (MailID, ItemSerial, ItemID, FirstAttachItemFlag, ReceiveFlag, AttachItemCount)
	SELECT @inbSendMailID, @inbItemSerial, @intItemID, 1, 0, @insItemCount;		

	/**_# ???? ??? ?????.*/
	INSERT dbo.PushQueue (WishListID, CharacterID, RegistedDate, PushTypeCode, ItemID)
	SELECT NULL, @inbSellerCharacterID, @dt2Now, 3, @intAuctionItemID;
	

	COMMIT TRANSACTION;

	/**_# ??? ??? ????. */
	EXEC @intReturnValue = dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 59
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = @inbSellerCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = @intAuctionID
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = NULL
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemLevel = @inyItemLevel
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount
		, @inyDragonJewelType = @inyDragonJewelType;

	/**_# ???? ??? ????. */	
	SET @inbChangedCoin = (@inbBuyitNowPrice * -1);
	
	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dt2Now
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = 36
		, @inbCurrentCoin = @inbCoin
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
