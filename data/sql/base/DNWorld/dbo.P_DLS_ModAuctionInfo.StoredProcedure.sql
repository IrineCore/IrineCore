USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ModAuctionInfo]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-23
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000062 = ???? ?? ????? ?? ? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DLS_ModAuctionInfo]
	  @intAuctionID int --// ??ID	
	, @inbStartPrice bigint = NULL --// ?????	
	, @inbBuynowPrice bigint = NULL --// ?????	
	, @sdtBiddingEndDate smalldatetime = NULL --// ??????	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @intBiddingCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT @intBiddingCount = COUNT(*)
	FROM dbo.AuctionBidding WITH (NOLOCK)
	WHERE AuctionID = @intAuctionID;

	IF @intBiddingCount > 0
	BEGIN
		SET @intReturnValue = 1030000062;
		GOTO ErrorHandler;
	END

	/**_# [Auctions] ???? UPDATE???.*/
	UPDATE dbo.Auctions
	SET StartPrice = COALESCE(@inbStartPrice, StartPrice)
	  , BuynowPrice = COALESCE(@inbBuynowPrice, BuynowPrice)
	  , BiddingEndDate = COALESCE(@sdtBiddingEndDate, BiddingEndDate)
	WHERE AuctionID = @intAuctionID;

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
