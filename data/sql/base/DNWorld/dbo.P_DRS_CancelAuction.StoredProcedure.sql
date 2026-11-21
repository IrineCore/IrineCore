USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_CancelAuction]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-08
description : ??? ?? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000025 = ?? ??? ???? ????.
1030000045 = ???? ?? ??? ? ????.
1030000046 = ???? ? ????? ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_CancelAuction]
	  @intAuctionID int --// ??ID	
	, @nvcMailSubject nvarchar(50) --// ?? ?? ??		
	, @nvcMailContent nvarchar(300) --// ?? ?? ??
	, @nvcMailSenderName nvarchar(50) --// ?? ?? ????		
	, @inyMailTabCode tinyint = NULL --// ?? ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inyAuctionStatusCode tinyint
	  , @dtsBiddingEndDate smalldatetime
	  , @dt2Now datetime2(0) = GETDATE()
	  , @inbItemSerial bigint
	  , @intItemID int
	  , @intBiddingCount int
	  , @inbSellerCharacterID bigint
	  , @inbSendMailID bigint
      , @insItemCount smallint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ??? ?? ??? ?????.*/	
	SELECT @inyAuctionStatusCode = A.AuctionStatusCode, @dtsBiddingEndDate = A.BiddingEndDate, @inbItemSerial = A.ItemSerial, @intItemID = A.ItemID
		 , @intBiddingCount = AB.BiddingCount, @inbSellerCharacterID = A.CharacterID, @insItemCount = A.ItemCount
	FROM dbo.Auctions A WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK) 
	     LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount AB WITH (NOLOCK) ON A.AuctionID = AB.AuctionID
	WHERE A.AuctionID = @intAuctionID;
	
	/**_# ????? ??? ?????.*/	
	IF @intBiddingCount > 0
	BEGIN
		SET @intReturnValue = 1030000045;
		GOTO ErrorHandler;
	END

	/**_# ???? ????? ??? ???? ?????.*/	
	IF @inyAuctionStatusCode = 2 OR (@dtsBiddingEndDate > @dt2Now AND @intBiddingCount = 0)
	BEGIN
		SET @intReturnValue = 1030000046;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;

	/**_# [MaterializedItems] ???? UPDATE ???.*/
	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = 15
	    , ItemLocationIndex = NULL
	    , OwnerCharacterID = NULL	
	WHERE ItemSerial = @inbItemSerial AND ItemID = @intItemID AND ItemLocationCode = 22;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000025;
		GOTO ErrorHandler;
	END 

	/**_# ???? ??? ?????.*/
	INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, DeleteDate, NewFlag, MailTabCode)
	SELECT NULL, @inbSellerCharacterID, 3, 1, @nvcMailSubject, @nvcMailContent, @dt2Now, 0, 0, 0, 0, @nvcMailSenderName, NULL, NULL, 0, @inyMailTabCode;

	SET @inbSendMailID = SCOPE_IDENTITY();

	INSERT dbo.MailAttachItems (MailID, ItemSerial, ItemID, FirstAttachItemFlag, ReceiveFlag, AttachItemCount)
	SELECT @inbSendMailID, @inbItemSerial, @intItemID, 1, 0, @insItemCount;	

	/**_# ??? ?? ?????.*/
	DELETE dbo.Auctions	WHERE AuctionID = @intAuctionID;

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
