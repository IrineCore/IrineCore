USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_TakeAuctionedOffCoin]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-28
description : ??? ??? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000037 = ?? ???? ?? ?????.
1030000038 = ?? ???? ???? ????.
1030000039 = ??? ??? ????.
**/
CREATE PROCEDURE [dbo].[P_DRS_TakeAuctionedOffCoin]
	  @inbCharacterID bigint --// ?? ???ID
	, @intAuctionID int --// ??ID
	, @nvcMailSubject nvarchar(50) --// ?? ?? ??		
	, @nvcMailContent nvarchar(300) --// ?? ?? ??
	, @nvcMailSenderName nvarchar(50) --// ?? ?? ????	
	, @inyMailTabCode tinyint = NULL --// ?? ?? ? ??		
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @sdtBiddingEndDate smalldatetime
	  , @inbSellerCharacterID bigint
	  , @dt2Now datetime2(0) = GETDATE()
	  , @inyAuctionStatusCode tinyint
	  , @inbLastBiddingPrice bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ????? ?? ???? ??? ?????.*/
	SELECT @sdtBiddingEndDate = A.BiddingEndDate, @inbSellerCharacterID = A.CharacterID, @inyAuctionStatusCode = AuctionStatusCode
	FROM dbo.Auctions A WITH (NOLOCK) 
	WHERE A.AuctionID = @intAuctionID 

	IF @inbCharacterID <> @inbSellerCharacterID 
	BEGIN
		SET @intReturnValue = 1030000038;
		GOTO ErrorHandler;
	END

	IF (@sdtBiddingEndDate > @dt2Now AND @inyAuctionStatusCode <> 2)
	BEGIN
		SET @intReturnValue = 1030000037;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;

	/**_# ?? ??? ????? ???? ???.*/
	SELECT @inbLastBiddingPrice = MaxBiddingPrice
	FROM dbo.VIX_AuctionBiddingCount WITH (NOLOCK)
	WHERE AuctionID = @intAuctionID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000039;
		GOTO ErrorHandler;
	END

	INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, DeleteDate, NewFlag, MailTabCode)
	SELECT NULL, @inbCharacterID, 3, 1, @nvcMailSubject, @nvcMailContent, @dt2Now, 0, 0, @inbLastBiddingPrice, 0, @nvcMailSenderName, NULL, NULL, 0, @inyMailTabCode;	

	/**_# ???? ?? ??? ???.*/
	UPDATE dbo.Auctions SET CoinCalculateFlag = CAST (1 AS bit) WHERE AuctionID = @intAuctionID;		
	
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
