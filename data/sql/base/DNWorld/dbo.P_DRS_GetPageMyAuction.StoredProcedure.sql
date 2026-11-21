USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetPageMyAuction]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-06
description : ?? ????? ??????.
result set : AuctionID/int, ItemID/int, ItemName/nvarchar, LevelLimit/tinyint, ItemSerial/bigint, ItemLevel/tinyint, SealCount/tinyint, AuctionStatusCode/tinyint, BuynowPrice/int, BiddingEndDate/datetime2, BiddingPrice/int, ItemCount/smallint, CoinCalculateFlag/bit, ItemCalculateFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetPageMyAuction]
	  @intPageNumber int --// ??? ??
	, @intPageSize int --// ??? ???
	, @inbCharacterID bigint --// ???ID	
	, @inyAuctionStatusCode tinyint = NULL --// ???????? (NULL=??, 0=???, 1=????, 2=??)	
	, @intTotalCount int OUTPUT --// ??? ???? ?? ???? ?????. (1?????? ?????.)
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
	IF @intPageNumber = 1
		EXEC @intReturnValue = dbo.P_DRS_GetMyAuctionCount
			  @inbCharacterID = @inbCharacterID
			, @inyAuctionStatusCode = @inyAuctionStatusCode			
			, @intAuctionCount = @intTotalCount OUTPUT;		

	/**_# [Auctions] ???? SELECT???.*/
	IF @inyAuctionStatusCode = 0
		WITH A AS(
			SELECT ROW_NUMBER() OVER(ORDER BY AC.AuctionStatusCode DESC, AC.BiddingEndDate DESC) AS RowNumber
				 , AC.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, AC.AuctionStatusCode, AC.BuynowPrice, AC.BiddingEndDate, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag
			FROM dbo.Auctions AC WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)			
				LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID				
			WHERE AC.CharacterID = @inbCharacterID AND AC.AuctionStatusCode = 0 AND AC.BiddingEndDate > @dt2Now
		)
		SELECT A.AuctionID, A.ItemID, A.ItemName, A.LevelLimit, A.ItemSerial, A.ItemLevel, A.SealCount
			 , 0 AS AuctionStatusCode		
			 , A.BuynowPrice, A.BiddingEndDate, ISNULL(B.MaxBiddingPrice, 0) AS BiddingPrice, A.ItemCount, A.CoinCalculateFlag, A.ItemCalculateFlag
		FROM A LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount B ON A.AuctionID = B.AuctionID	
		WHERE A.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		ORDER BY A.RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	ELSE IF @inyAuctionStatusCode = 1 
		WITH A AS(
			SELECT ROW_NUMBER() OVER(ORDER BY AC.AuctionStatusCode DESC, AC.BiddingEndDate DESC) AS RowNumber
				 , AC.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, AC.AuctionStatusCode, AC.BuynowPrice, AC.BiddingEndDate, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag				 
			FROM dbo.Auctions AC WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)			
				LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID
			    LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount AB WITH (NOLOCK) ON AC.AuctionID = AB.AuctionID				
			WHERE AC.CharacterID = @inbCharacterID AND (AC.AuctionStatusCode = 2 OR (AC.BiddingEndDate <= @dt2Now AND ISNULL(AB.BiddingCount, 0) > 0)) AND AC.AuctionStatusCode <> 4
		)
		SELECT A.AuctionID, A.ItemID, A.ItemName, A.LevelLimit, A.ItemSerial, A.ItemLevel, A.SealCount, A.ItemCount
			 , 1 AS AuctionStatusCode		
			 , A.BuynowPrice, A.BiddingEndDate, ISNULL(B.MaxBiddingPrice, 0) AS BiddingPrice, A.ItemCount, A.CoinCalculateFlag, A.ItemCalculateFlag
		FROM A LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount B ON A.AuctionID = B.AuctionID	
		WHERE A.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		ORDER BY A.RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	ELSE IF @inyAuctionStatusCode = 2
		WITH A AS(
			SELECT ROW_NUMBER() OVER(ORDER BY AC.AuctionStatusCode DESC, AC.BiddingEndDate DESC) AS RowNumber
				 , AC.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, AC.AuctionStatusCode, AC.BuynowPrice, AC.BiddingEndDate, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag				 
			FROM dbo.Auctions AC WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)			
				LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID	
			    LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount AB WITH (NOLOCK) ON AC.AuctionID = AB.AuctionID			
			WHERE AC.CharacterID = @inbCharacterID AND AC.BiddingEndDate <= @dt2Now AND ISNULL(AB.BiddingCount, 0) = 0 AND AC.AuctionStatusCode <> 4		
		)
		SELECT A.AuctionID, A.ItemID, A.ItemName, A.LevelLimit, A.ItemSerial, A.ItemLevel, A.SealCount, A.ItemCount
			 , 2 AS AuctionStatusCode		
			 , A.BuynowPrice, A.BiddingEndDate, ISNULL(B.MaxBiddingPrice, 0) AS BiddingPrice, A.ItemCount, A.CoinCalculateFlag, A.ItemCalculateFlag
		FROM A LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount B ON A.AuctionID = B.AuctionID			
		WHERE A.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		ORDER BY A.RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	ELSE
		WITH A AS(
			SELECT ROW_NUMBER() OVER(ORDER BY AC.AuctionStatusCode DESC, AC.BiddingEndDate DESC) AS RowNumber
				 , AC.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, AC.AuctionStatusCode, AC.BuynowPrice, AC.BiddingEndDate, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag				 
			FROM dbo.Auctions AC WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)			
				LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID				
			WHERE AC.CharacterID = @inbCharacterID AND AC.AuctionStatusCode = COALESCE(@inyAuctionStatusCode, AC.AuctionStatusCode) AND AC.AuctionStatusCode <> 4
		)
		SELECT A.AuctionID, A.ItemID, A.ItemName, A.LevelLimit, A.ItemSerial, A.ItemLevel, A.SealCount, A.ItemCount
			 , CASE WHEN A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
					WHEN (A.AuctionStatusCode = 2 OR (A.BiddingEndDate <= @dt2Now AND ISNULL(B.BiddingCount, 0) > 0)) THEN 1
					WHEN A.BiddingEndDate <= @dt2Now AND ISNULL(B.BiddingCount, 0) = 0 THEN 2
                    ELSE 0 END AS AuctionStatusCode		
			 , A.BuynowPrice, A.BiddingEndDate, ISNULL(B.MaxBiddingPrice, 0) AS BiddingPrice, A.ItemCount, A.CoinCalculateFlag, A.ItemCalculateFlag
		FROM A LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount B ON A.AuctionID = B.AuctionID		
		WHERE A.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		ORDER BY A.RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	
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
