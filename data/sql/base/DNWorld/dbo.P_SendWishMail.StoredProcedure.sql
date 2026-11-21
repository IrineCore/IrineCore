USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SendWishMail]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-06-04

description :
	??? ?? ??
result :
	@inbReceiverCharacterID / bigint
	@intReceiverAccountID / int
	@intTotalMailCount / int
	@intNotReadMailCount / int
	@int7DaysLeftMailCount / int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103338 = ??ID, ???ID, ?????? ??? ???? ????.
	103102 = ???? ???? ????.
	103380 = DWC ????? ??? ??? ? ????.
history :
	1~7 = ?? ??
	9 = DWC ???? ?? ?? ? ??? ?? by ??? at 2013-02-19
**/

CREATE PROCEDURE [dbo].[P_SendWishMail]
	  @inbSenderCharacterID bigint --// ?????? ID
	, @inbReceiverCharacterName nvarchar(30) --// ?? ??? ??
	, @nvcSubject nvarchar(50) --// ??
	, @nvcContent nvarchar(300) --// ??

	, @vchProductIDs varchar(max) --// ??? ??ID (S/N)
	, @vchItemProductIDs varchar(max) --// ?? ??ID (S/N)
	, @vchItemIDs varchar(max) --// ???ID
	, @vchItemOptions varchar(max) --// ??? ??

	, @intWishLimitCount int --// 1? ?? ??? ??
	, @bitNewFlag bit --// 0=?? ??? ?? ?, 1=?? ??? ?? ?

	, @inbReceiverCharacterID bigint OUTPUT --// ?? ??? ID
	, @intReceiverAccountID int OUTPUT --// ?? ???? ?? ID
	, @intTotalMailCount int OUTPUT --// ? ?? ?? ?
	, @intNotReadMailCount int OUTPUT --// ?? ?? ?? ?
	, @int7DaysLeftMailCount int OUTPUT --// ?? ??? 7? ?? ?? ?

--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @inbMailID bigint
      , @intRowCount int
      , @dt2Now datetime2(0) = GETDATE()
	  , @inyAccountLevelCode tinyint;

DECLARE @tblProductID TABLE (Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, ProductID int NOT NULL);
DECLARE @tblItemProductID TABLE (Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, ProductID int NOT NULL);
DECLARE @tblItemID TABLE (Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemID int NOT NULL);
DECLARE @tblItemOption TABLE (Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemOption int NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ?? ??? ?? ???? */
	SELECT @inbReceiverCharacterID = CharacterID
	     , @intReceiverAccountID = AccountID
		 , @inyAccountLevelCode = AccountLevelCode
	  FROM dbo.Characters WITH (NOLOCK)
	 WHERE CharacterName = @inbReceiverCharacterName
	   AND DeleteFlag = CONVERT(bit, 0)

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102; -- ???? ???? ????.
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ???? DWC ????? 103380? ?????.*/
	IF @inyAccountLevelCode = 110
	BEGIN
		SET @intReturnValue = 103380;
		GOTO ErrorHandler;
	END

	/**_# 1? ?? ??? ?? ?? */
	IF @intWishLimitCount <= (SELECT COUNT(*) FROM dbo.Mails WITH (NOLOCK) WHERE MailTypeCode = 18 AND SenderCharacterID = @inbSenderCharacterID AND SendDate > CONVERT(datetime2(0), CONVERT(date, GETDATE())))
	BEGIN
		SET @intReturnValue = 103339; -- 1? ?? ??? ??? ???????.
		GOTO ErrorHandler;
	END

	/**_# ??? ??ID (S/N) */
	INSERT @tblProductID (ProductID) SELECT CAST(string AS int) FROM dbo.TF_Split(@vchProductIDs, ',') WHERE string > '';

	SET @intRowCount = @@ROWCOUNT;

	/**_# ?? ??ID (S/N) */
	INSERT @tblItemProductID (ProductID) SELECT CAST(string AS int) FROM dbo.TF_Split(@vchItemProductIDs, ',') WHERE string > '';

	IF @@ROWCOUNT <> @intRowCount
	BEGIN
		SET @intReturnValue = 103338; -- ??ID, ???ID, ?????? ??? ???? ????.
		GOTO ErrorHandler;
	END

	/**_# ???ID */
	INSERT @tblItemID (ItemID) SELECT CAST(string AS int) FROM dbo.TF_Split(@vchItemIDs, ',') WHERE string > '';

	IF @@ROWCOUNT <> @intRowCount
	BEGIN
		SET @intReturnValue = 103338; -- ??ID, ???ID, ?????? ??? ???? ????.
		GOTO ErrorHandler;
	END

	/**_# ????? */
	INSERT @tblItemOption (ItemOption) SELECT CAST(string AS int) FROM dbo.TF_Split(@vchItemOptions, ',') WHERE string > '';

	IF @@ROWCOUNT <> @intRowCount
	BEGIN
		SET @intReturnValue = 103338; -- ??ID, ???ID, ?????? ??? ???? ????.
		GOTO ErrorHandler;
	END
	BEGIN TRANSACTION
		/**_# ??? ??*/
		INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, NewFlag)
		VALUES (@inbSenderCharacterID, @inbReceiverCharacterID, 18, 4, @nvcSubject, @nvcContent, @dt2Now, CAST(0 AS bit), CAST(0 AS bit), 0, CAST(0 AS bit), @bitNewFlag);

		SET @inbMailID = SCOPE_IDENTITY();

		/**_# ??? ??*/
		INSERT dbo.WishProducts (MailID, ProductID)
		SELECT DISTINCT @inbMailID, ProductID FROM @tblProductID;

		/**_# ??? ???*/
		INSERT dbo.WishItems (WishProductID, MailID, ProductID, ItemID, ItemOption)
		SELECT W.WishProductID
		     , @inbMailID
		     , IP.ProductID
		     , I.ItemID
		     , O.ItemOption
		  FROM dbo.WishProducts W WITH (NOLOCK)
		       INNER JOIN @tblProductID P ON (W.ProductID = P.ProductID)
		       INNER JOIN @tblItemProductID IP ON (P.Seq = IP.Seq)
		       INNER JOIN @tblItemID I ON (P.Seq = I.Seq)
		       INNER JOIN @tblItemOption O ON (P.Seq = O.Seq)
		 WHERE W.MailID = @inbMailID;

	COMMIT TRANSACTION

	EXEC dbo.P_GetCountReceiveMail
		  @inbReceiverCharacterID = @inbReceiverCharacterID
		, @intTotalMailCount = @intTotalMailCount OUTPUT
		, @intNotReadMailCount = @intNotReadMailCount OUTPUT
		, @int7DaysLeftMailCount = @int7DaysLeftMailCount OUTPUT;

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
