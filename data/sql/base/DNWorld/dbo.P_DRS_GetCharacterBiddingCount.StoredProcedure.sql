USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetCharacterBiddingCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-21
description : ???? ?? ???? ?????.
result set : BiddingStatusCode/tinyint, StatusCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetCharacterBiddingCount]	  
	  @inbCharacterID bigint --// ?? ??? ID	
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

	/**_# [AuctionBidding] ???? SELECT???.*/
	;WITH A AS (
		SELECT AuctionID, MAX(BiddingID) AS BiddingID
		FROM dbo.AuctionBidding WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID 
		GROUP BY AuctionID
	)
	SELECT CASE WHEN AB.BiddingStatusCode = 0 AND AC.AuctionStatusCode = 0 AND AC.BiddingEndDate > @dt2Now THEN 0
				WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate <= @dt2Now THEN 1
				WHEN AB.BiddingStatusCode = 1 THEN 2 END AS BiddingStatusCode, COUNT(*) AS StatusCount
	FROM A INNER JOIN dbo.AuctionBidding AB WITH (NOLOCK) ON A.BiddingID = AB.BiddingID
		 INNER JOIN dbo.Auctions AC WITH (NOLOCK) ON A.AuctionID = AC.AuctionID
	GROUP BY CASE WHEN AB.BiddingStatusCode = 0 AND AC.AuctionStatusCode = 0 AND AC.BiddingEndDate > @dt2Now THEN 0
				WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate <= @dt2Now THEN 1
				WHEN AB.BiddingStatusCode = 1 THEN 2 END;

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
