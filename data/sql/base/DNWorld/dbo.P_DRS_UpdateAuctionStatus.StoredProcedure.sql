USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_UpdateAuctionStatus]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-28
description : ?? ??? ???????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_UpdateAuctionStatus]	  
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	UPDATE dbo.Auctions
	SET AuctionStatusCode = 
		CASE WHEN AB.BiddingStatusCode = 0 THEN 1
		WHEN BC.BiddingCount = 0 THEN 3
		ELSE AuctionStatusCode END					
	FROM dbo.Auctions A WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK) INNER JOIN dbo.VIX_AuctionBiddingCount BC WITH (NOLOCK)
		 ON A.AuctionID = BC.AuctionID
		 LEFT OUTER JOIN dbo.AuctionBidding AB WITH (NOLOCK, INDEX(PK_AuctionBidding), FORCESEEK) ON BC.LastBiddingID = AB.BiddingID
	WHERE A.AuctionStatusCode = 0 AND A.BiddingEndDate < @dt2Now
	OPTION (MAXDOP 1, KEEP PLAN, KEEPFIXED PLAN);
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
