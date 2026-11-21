USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListAuctionEnableItem]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-15
description : ?? ?? ??? ???? ?????.
result set : ItemID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListAuctionEnableItem]	  
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
	/**_# [AuctionEnableItemsFromDNT] ???? [AuctionEnableItemsFromDOLIS] ????? SELECT???.*/

	SELECT ItemID
	FROM dbo.AuctionEnableItemsFromDNT WITH (NOLOCK)
	WHERE ActivateFlag = 1
	UNION ALL
	SELECT ItemID
	FROM dbo.AuctionEnableItemsFromDOLIS WITH (NOLOCK)
	WHERE ActivateFlag = 1

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
