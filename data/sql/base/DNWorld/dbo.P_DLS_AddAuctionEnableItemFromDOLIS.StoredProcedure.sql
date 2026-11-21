USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddAuctionEnableItemFromDOLIS]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-15
description : ????? ?? ?????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000050 = ?? DNT ??? ???? ?? ??????.
1030000051 = ?? DOLIS?? ??? ??????.
**/

CREATE PROCEDURE [dbo].[P_DLS_AddAuctionEnableItemFromDOLIS]
	  @intItemID int --// ???ID	
	, @inyItemCategoryID tinyint --// ??? ???? ID
	, @nvcItemName nvarchar(50) --// ??? ???? ID
	, @bitActivateFlag bit = 1--// ??? ?? (0=?????, 1=????)
WITH EXECUTE AS 'SPExecutor'
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
	IF EXISTS (SELECT * FROM dbo.AuctionEnableItemsFromDNT WITH (NOLOCK) WHERE ItemID = @intItemID)
	BEGIN
		SET @intReturnValue = 1030000050;
		GOTO ErrorHandler;
	END

	/**_# [AuctionEnableItemsFromDOLIS] ???? INSERT???.*/
	INSERT dbo.AuctionEnableItemsFromDOLIS (ItemID, ItemCategoryID, ItemName, ActivateFlag)
	VALUES (@intItemID, @inyItemCategoryID, @nvcItemName, @bitActivateFlag);

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_%'
		SET @intReturnValue = 1030000051;

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
