USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_DelAuctionEnableItemFromDOLIS]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-15
description : ????? ??? ?? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000001 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DLS_DelAuctionEnableItemFromDOLIS]
	  @intItemID int --// ???ID	
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
	/**_# [AuctionEnableItemsFromDOLIS] ???? DELETE???.*/
	DELETE dbo.AuctionEnableItemsFromDOLIS WHERE ItemID = @intItemID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000001;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
