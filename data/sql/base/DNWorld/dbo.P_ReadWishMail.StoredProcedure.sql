USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ReadWishMail]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-06-08

description :
	??? ?? ??
result set :
	-- OUTPUT Parameter
		@inbSenderCharacterID / bigint
		@inbReceiverCharacterID / bigint
		@nvcContent / nvarchar(300)
		@dt2SendDate / datetime2(0)
		@inbPurchaseOrderID / bigint
		@intNotReadMailCount / int
		@bitNewFlag / bit
	-- Record Set
		ProductID / int
		ItemProductID / int
		ItemID / int
		ItemOption / tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103166 = ???? ?? ?????.
	103340 = ??? ???? ????.
history :
	1~4 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_ReadWishMail]
	  @inbMailID bigint --// ?? ID
	, @inbSenderCharacterID bigint OUTPUT --// ?? ??? ID  ?, MailTypeCode = 2? ??, NULL ?????.
	, @inbReceiverCharacterID bigint OUTPUT --// ?? ??? ID
	, @nvcContent nvarchar(300) OUTPUT --// ??
	, @dt2SendDate datetime2(0) OUTPUT --// ?? ??
	, @inbPurchaseOrderID bigint OUTPUT --// ??? ?? ?? ID
	, @intNotReadMailCount int OUTPUT --// ?? ?? ?? ?
	, @bitNewFlag bit OUTPUT --// 0=?? ??? ?? ?, 1=?? ??? ?? ?
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

	SET @intNotReadMailCount = 0;

	/**_# ??? ?? ??? ???? ??? ?????.*/
	UPDATE dbo.Mails
	   SET @inbSenderCharacterID = M.SenderCharacterID
	     , @inbReceiverCharacterID = M.ReceiverCharacterID
	     , @nvcContent = M.Content
	     , @dt2SendDate = M.SendDate
	     , @inbPurchaseOrderID = WP.PurchaseOrderID
	     , @bitNewFlag = ISNULL(M.NewFlag, 0)
	     , ReadFlag = CAST(1 AS bit)
	  FROM dbo.Mails M
	       INNER JOIN dbo.WishProducts WP WITH (NOLOCK) ON (M.MailID = WP.MailID)
	 WHERE M.MailID = @inbMailID
	   AND M.DeleteFlag = CAST(0 AS bit);

	IF @@ROWCOUNT = 0
	BEGIN
		SELECT WP.ProductID, WI.ProductID AS ItemProductID, WI.ItemID, WI.ItemOption
		  FROM dbo.WishProducts WP WITH (NOLOCK)
			   INNER JOIN dbo.WishItems WI WITH (NOLOCK) ON (WP.WishProductID = WI.WishProductID)
		 WHERE (1 = 0);

		SET @intReturnValue = 103166; -- ???? ?? ?????.
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ??? ?? ?????.*/
	SELECT @intNotReadMailCount = COUNT(*)
	  FROM dbo.Mails WITH (NOLOCK)
	 WHERE ReceiverCharacterID = @inbReceiverCharacterID
	   AND SendDate >= CAST(CAST(DATEADD(DAY, -29, GETDATE()) AS date) AS datetime2(0))
	   AND DeleteFlag = CAST(0 AS bit)
	   AND ReadFlag = CAST(0 AS bit);

	/**_# ??? ???? ?????.*/
	SELECT WP.ProductID, WI.ProductID AS ItemProductID, WI.ItemID, WI.ItemOption
	  FROM dbo.Mails M WITH (NOLOCK)
	       INNER JOIN dbo.WishProducts WP WITH (NOLOCK) ON (M.MailID = WP.MailID)
	       INNER JOIN dbo.WishItems WI WITH (NOLOCK) ON (WP.WishProductID = WI.WishProductID)
	 WHERE M.MailID = @inbMailID
	 ORDER BY WP.ProductID, WI.ItemID;

	IF @@ROWCOUNT = 0
	BEGIN
		SELECT WP.ProductID, WI.ProductID AS ItemProductID, WI.ItemID, WI.ItemOption
		  FROM dbo.WishProducts WP WITH (NOLOCK)
			   INNER JOIN dbo.WishItems WI WITH (NOLOCK) ON (WP.WishProductID = WI.WishProductID)
		 WHERE (1 = 0);

		SET @intReturnValue = 103340; -- ??? ???? ????.
		GOTO ErrorHandler;
	END

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
