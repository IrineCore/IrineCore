USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddAuctionBidding]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 20
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-27
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000030 = ?? ?? ??? ?????.
1030000031 = ?? ???? ????? ????.
1030000032 = ??? ??? ?????.
1030000033 = ??? ???? 1??? ?????.
1030000041 = ???? ?????? ?????.
1030000042 = ???? ??????? ????.
1030000043 = ???? ???(???)? ?????.
1030000047 = DWC ???? ???? ??? ? ????.
1030000048 = ???? ?? ??? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_AddAuctionBidding]
	  @intAuctionID int --// ??ID
	, @inbCharacterID bigint --// ?? ??? ID
	, @intAccountID int --// ?? ??? AccountID
	, @inbBiddingPrice bigint --// ????	
	, @nvcRefundMailSubject nvarchar(50) --// ?? ?? ??		
	, @nvcRefundMailContent nvarchar(300) --// ?? ?? ??
	, @nvcRefundMailSenderName nvarchar(50) --// ?? ?? ????
	, @inyCharacterLevel tinyint = NULL OUTPUT -- // ?? ??? ??
	, @intCharacterExp int = NULL OUTPUT -- // ?? ??? ???
	, @inbCoin bigint = NULL OUTPUT -- // ?? ??? ??
	, @inbWarehouseCoin bigint = NULL OUTPUT -- // ?? ??? ????
	, @inyMailTabCode tinyint = NULL --// ?? ?? ? ??
	, @inbCommission bigint = NULL --// ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @sdtBiddingEndDate smalldatetime
	  , @inbCurrentPrice bigint
	  , @inbLatestBiddingCharacterID bigint
	  , @inbLatestBiddingPrice bigint	  
	  , @inbCharacterCoin bigint
	  , @inbChangedCoin bigint
	  , @inbSellerCharacterID bigint
	  , @inbBuyNowPrice bigint
	  , @intSellerCharacterAccountID int
	  , @intBiddingCount int
	  , @inyAccountLevelCode tinyint
	  , @inyAuctionStatusCode tinyint
	  , @intAuctionItemID int
	  , @inbTotalBiddingAmount bigint
	  , @intLatestBiddingAccountID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @inbTotalBiddingAmount = @inbBiddingPrice + ISNULL(@inbCommission, 0);

	/**_# DWC???? ??? ?????.*/	
	SELECT @inyAccountLevelCode = AccountLevelCode
	FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
	WHERE CharacterID = @inbCharacterID;
	
	IF @inyAccountLevelCode = 110
	BEGIN
		SET @intReturnValue = 1030000047;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ????? ???? ?????.*/	
	SELECT @sdtBiddingEndDate = BiddingEndDate, @inbBuyNowPrice = BuynowPrice, @inbSellerCharacterID = CharacterID, @inyAuctionStatusCode = AuctionStatusCode, @intAuctionItemID = ItemID
	FROM dbo.Auctions WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK)
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
	

	/**_# ??? ??? ????? ??? ???? ??? ?????.*/	
	SELECT @inbCurrentPrice = BiddingPrice
	FROM dbo.AuctionBidding WITH (UPDLOCK, INDEX(PK_AuctionBidding))
	WHERE AuctionID = @intAuctionID AND BiddingStatusCode = 0;

	IF @inbCurrentPrice >= @inbBiddingPrice
	BEGIN
		SET @intReturnValue = 1030000031;
		GOTO ErrorHandler;
	END	

	/**_# ???? ???(???)? ???? ?????.*/	
	SELECT @intSellerCharacterAccountID = AccountID 
	FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) 
	WHERE CharacterID = @inbSellerCharacterID;

	IF @inbSellerCharacterID = @inbCharacterID OR @intAccountID = @intSellerCharacterAccountID
	BEGIN
		SET @intReturnValue = 1030000043;
		GOTO ErrorHandler;
	END

	/**_# ???? ?????? ?????.*/	
	IF @inbBiddingPrice = @inbBuyNowPrice
	BEGIN
		SET @intReturnValue = 1030000041;
		GOTO ErrorHandler;
	END

	IF @inbBiddingPrice > @inbBuyNowPrice
	BEGIN
		SET @intReturnValue = 1030000042;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ???? ??? ?????.*/	
	SELECT @inbCharacterCoin = Coin
	FROM dbo.CharacterStatus WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK)
	WHERE CharacterID = @inbCharacterID;

	IF @inbTotalBiddingAmount > @inbCharacterCoin
	BEGIN
		SET @intReturnValue = 1030000032;
		GOTO ErrorHandler;
	END
	
	BEGIN TRANSACTION;
	
	/**_# ?? ??? ???? ?? ?? ?? ???.*/	
	UPDATE dbo.AuctionBidding
	SET BiddingStatusCode = 1, @inbLatestBiddingCharacterID = CharacterID, @inbLatestBiddingPrice = BiddingPrice
	WHERE AuctionID = @intAuctionID AND BiddingStatusCode = 0;

	SET @intBiddingCount = @@ROWCOUNT;

	IF @intBiddingCount > 1
	BEGIN
		SET @intReturnValue = 1030000033;
		GOTO ErrorHandler;
	END 
	ELSE IF @intBiddingCount = 1
	BEGIN
		/**_# ?? ??? ???? AccountID? ?????.*/
		SELECT @intLatestBiddingAccountID = AccountID
		FROM dbo.Characters WITH (NOLOCK) WHERE CharacterID = @inbLatestBiddingCharacterID;
		
		/**_# ?? ??? ???? ?? ??? ?? ??(????) ???.*/
		INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, DeleteDate, NewFlag, MailTabCode)
		SELECT NULL, @inbLatestBiddingCharacterID, 3, 1, @nvcRefundMailSubject, @nvcRefundMailContent, @dt2Now, 0, 0, @inbLatestBiddingPrice, 0, @nvcRefundMailSenderName, NULL, NULL, 0, @inyMailTabCode;			

		/**_# ?? ?? ??? ?????.*/
		INSERT dbo.PushQueue (WishListID, CharacterID, RegistedDate, PushTypeCode, ItemID, AccountID)
		SELECT NULL, @inbLatestBiddingCharacterID, @dt2Now, 2, @intAuctionItemID, @intLatestBiddingAccountID;
	END

	/**_# ?? ??? ?????.*/
	UPDATE dbo.CharacterStatus
	SET Coin = Coin - @inbTotalBiddingAmount,
	    @inyCharacterLevel = CharacterLevel,
		@intCharacterExp = CharacterExp,
		@inbWarehouseCoin = WarehouseCoin,
	    @inbCoin = Coin - @inbTotalBiddingAmount
	WHERE CharacterID = @inbCharacterID;	

	/**_# ??? ???? ?? ???.*/
	INSERT dbo.AuctionBidding (AuctionID, CharacterID, BiddingPrice, BiddingStatusCode, BiddingDate)
	SELECT @intAuctionID, @inbCharacterID, @inbBiddingPrice, 0, @dt2Now;		
	
	COMMIT TRANSACTION;
	/**_# ???? ??? ????. */	
	SET @inbChangedCoin = (@inbTotalBiddingAmount * -1);
	
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
