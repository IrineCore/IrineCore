USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_TakeMiscarriedAuctionItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-28
description : ??? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000038 = ?? ???? ???? ????.
1030000037 = ?? ???? ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_TakeMiscarriedAuctionItem]
	  @intAuctionID int
	, @inbCharacterID bigint --// ?? ???ID
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
	  , @inyItemLevel tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ????? ?? ???? ??? ?????.*/
	SELECT @sdtBiddingEndDate = BiddingEndDate, @inbSellerCharacterID = CharacterID, @inbItemSerial = ItemSerial
	FROM dbo.Auctions WITH (NOLOCK)
	WHERE AuctionID = @intAuctionID

	IF @inbCharacterID <> @inbSellerCharacterID 
	BEGIN
		SET @intReturnValue = 1030000038;
		GOTO ErrorHandler;
	END

	IF @sdtBiddingEndDate > @dt2Now
	BEGIN
		SET @intReturnValue = 1030000037;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;
	
	/**_# ??? ???? ???????.*/
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
	SELECT NULL, @inbCharacterID, 3, 1, @nvcMailSubject, @nvcMailContent, @dt2Now, 0, 0, 0, 0, @nvcMailSenderName, NULL, NULL, 0, @inyMailTabCode;

	SET @inbSendMailID = SCOPE_IDENTITY();

	INSERT dbo.MailAttachItems (MailID, ItemSerial, ItemID, FirstAttachItemFlag, ReceiveFlag, AttachItemCount)
	SELECT @inbSendMailID, @inbItemSerial, @intItemID, 1, 0, @insItemCount;		

	/**_# ??? ?? ??? ???.*/
	UPDATE dbo.Auctions SET ItemCalculateFlag = CAST (1 AS bit) WHERE AuctionID = @intAuctionID;		

	COMMIT TRANSACTION;

	/**_# ??? ??? ????. */
	EXEC @intReturnValue = dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 60
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = @inbCharacterID
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
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount;
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
