USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetCharacterAutionCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-21
description : ???? ?? ???? ?????.
result set : AuctionStatusCode/tinyint, AuctionCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetCharacterAutionCount]
	  @inbCharacterID bigint --// ???ID		
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
	/**_# [Auctions] ???? SELELCT???.*/
	SELECT CASE WHEN A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
				WHEN (A.AuctionStatusCode = 2 OR (A.BiddingEndDate <= @dt2Now AND ISNULL(S.BiddingCount, 0) > 0)) AND (A.CoinCalculateFlag = 0) THEN 1
				WHEN A.BiddingEndDate <= @dt2Now AND ISNULL(S.BiddingCount, 0) = 0 THEN 2
                ELSE 0 END AS AuctionStatusCode, COUNT(*) AS AuctionCount
	FROM dbo.Auctions A WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)
		 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount S WITH (NOLOCK) ON A.AuctionID = S.AuctionID
	WHERE CharacterID = @inbCharacterID	  
	GROUP BY CASE WHEN A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
				WHEN (A.AuctionStatusCode = 2 OR (A.BiddingEndDate <= @dt2Now AND ISNULL(S.BiddingCount, 0) > 0)) AND (A.CoinCalculateFlag = 0) THEN 1
				WHEN A.BiddingEndDate <= @dt2Now AND ISNULL(S.BiddingCount, 0) = 0 THEN 2
				ELSE 0 END;	
	
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
