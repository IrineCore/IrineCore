USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListWishAuction]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-28
description : ???? ???? ?????.
result set : ItemID/int, ItemName/nvarchar, ItemLevel/tinyint, CharacterID/bigint, CharacterName/nvarchar, BiddingEndDate/smalldatetime, BuynowPrice/bigint, MaxBiddingPrice/bigint, ItemSerial/bigint, LevelLimit/tinyint, SealCount/tinyint, AuctionStatusCode/tinyint, ItemCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListWishAuction]
	  @inbCharacterID bigint --// ???ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [WishAuctions] ???? SELECT???.*/
	SELECT A.AuctionID, A.ItemID, AI.ItemName, A.ItemLevel, A.CharacterID, CH.CharacterName, A.BiddingEndDate, A.BuynowPrice, B.MaxBiddingPrice, A.ItemSerial, AI.LevelLimit, A.SealCount,
	       CASE WHEN A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
				WHEN (A.AuctionStatusCode = 2 OR (A.BiddingEndDate <= @dt2Now AND ISNULL(B.BiddingCount, 0) > 0)) THEN 1
				WHEN A.BiddingEndDate <= @dt2Now AND ISNULL(B.BiddingCount, 0) = 0 THEN 2
                ELSE 0 END AS AuctionStatusCode, A.ItemCount
	FROM dbo.WishAuctions WA WITH (NOLOCK) 
		 LEFT OUTER JOIN dbo.Auctions A WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK) ON WA.AuctionID = A.AuctionID
		 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount B WITH (NOLOCK) ON A.AuctionID = B.AuctionID
		 LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK, INDEX(PK_AuctionItems), FORCESEEK) ON A.ItemID = AI.ItemID
	     LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON A.CharacterID = CH.CharacterID
	WHERE WA.CharacterID = @inbCharacterID;
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
