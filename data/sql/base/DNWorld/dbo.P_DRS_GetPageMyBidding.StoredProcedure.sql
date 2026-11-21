USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetPageMyBidding]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-25
description : ?? ?? ???? ??????.
result set : AuctionID/int, ItemID/int, ItemName/nvarchar, LevelLimit/tinyint, BiddingStatusCode/tinyint, BuynowPrice/int, BiddingEndDate/datetime2, BiddingPrice/int, ItemSerial/bigint, ItemLevel/tinyint, SealCount/tinyint, ItemCount/smallint, CoinCalculateFlag/bit, ItemCalculateFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetPageMyBidding]
	  @intPageNumber int --// ??? ??
	, @intPageSize int --// ??? ???
	, @inbCharacterID bigint --// ?? ??? ID
	, @inyBiddingStatusCode tinyint = NULL --// ?????? (NULL=??, 0=???, 1=??, 2=????, 3=????)	
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
		EXEC @intReturnValue = dbo.P_DRS_GetMyBiddingCount
			  @inbCharacterID = @inbCharacterID
			, @inyBiddingStatusCode = @inyBiddingStatusCode			
			, @intBiddingCount = @intTotalCount OUTPUT;	

	/**_# [AuctionBidding] ???? SELECT???.*/	
	IF @inyBiddingStatusCode = 0
		WITH A AS (
			SELECT AuctionID, MAX(BiddingID) AS BiddingID
			FROM dbo.AuctionBidding WITH (NOLOCK, INDEX(IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode), FORCESEEK)
			WHERE CharacterID = @inbCharacterID 
			GROUP BY AuctionID
		), B AS (
			SELECT ROW_NUMBER() OVER(ORDER BY BiddingID DESC) AS RowNumber, AuctionID, BiddingID
			FROM A 
		)
		SELECT B.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, 
			   CASE WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate > @dt2Now THEN 0
					WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate <= @dt2Now THEN 1
					WHEN AB.BiddingStatusCode = 1 THEN 2
					WHEN AB.BiddingStatusCode = 2 THEN 3 END AS BiddingStatusCode, 
			   AC.BuynowPrice, AC.BiddingEndDate, C.MaxBiddingPrice AS BiddingPrice, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag
		FROM B INNER JOIN dbo.AuctionBidding AB WITH (NOLOCK, INDEX(PK_AuctionBidding), FORCESEEK) ON B.BiddingID = AB.BiddingID
			 INNER JOIN dbo.Auctions AC WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK) ON B.AuctionID = AC.AuctionID
			 LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID
			 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount C ON B.AuctionID = C.AuctionID
		WHERE B.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
			  AND AB.BiddingStatusCode = 0 AND AC.AuctionStatusCode = 0 AND AC.BiddingEndDate > @dt2Now
		ORDER BY B.RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	ELSE IF @inyBiddingStatusCode = 1
		WITH A AS (
			SELECT AuctionID, MAX(BiddingID) AS BiddingID
			FROM dbo.AuctionBidding WITH (NOLOCK, INDEX(IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode), FORCESEEK)
			WHERE CharacterID = @inbCharacterID 
			GROUP BY AuctionID
		), B AS (
			SELECT ROW_NUMBER() OVER(ORDER BY BiddingID DESC) AS RowNumber, AuctionID, BiddingID
			FROM A 		
		)
		SELECT B.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, 
			   CASE WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate > @dt2Now THEN 0
					WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate <= @dt2Now THEN 1
					WHEN AB.BiddingStatusCode = 1 THEN 2
					WHEN AB.BiddingStatusCode = 2 THEN 3 END AS BiddingStatusCode,
			   AC.BuynowPrice, AC.BiddingEndDate, C.MaxBiddingPrice AS BiddingPrice, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag
		FROM B INNER JOIN dbo.AuctionBidding AB WITH (NOLOCK, INDEX(PK_AuctionBidding), FORCESEEK) ON B.BiddingID = AB.BiddingID
			 INNER JOIN dbo.Auctions AC WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK) ON B.AuctionID = AC.AuctionID
			 LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID
			 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount C ON B.AuctionID = C.AuctionID
		WHERE B.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
			 AND AC.BiddingEndDate <= @dt2Now AND AB.BiddingStatusCode = 0 AND AC.AuctionStatusCode <> 4
		ORDER BY B.RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	ELSE IF @inyBiddingStatusCode = 2
		WITH A AS (
			SELECT AuctionID, MAX(BiddingID) AS BiddingID
			FROM dbo.AuctionBidding WITH (NOLOCK, INDEX(IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode), FORCESEEK)
			WHERE CharacterID = @inbCharacterID 
			GROUP BY AuctionID
		), B AS (
			SELECT ROW_NUMBER() OVER(ORDER BY BiddingID DESC) AS RowNumber, AuctionID, BiddingID
			FROM A 
		)
		SELECT B.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, 
			   CASE WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate > @dt2Now THEN 0
					WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate <= @dt2Now THEN 1
					WHEN AB.BiddingStatusCode = 1 THEN 2
					WHEN AB.BiddingStatusCode = 2 THEN 3 END AS BiddingStatusCode,
			   AC.BuynowPrice, AC.BiddingEndDate, C.MaxBiddingPrice AS BiddingPrice, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag
		FROM B INNER JOIN dbo.AuctionBidding AB WITH (NOLOCK, INDEX(PK_AuctionBidding), FORCESEEK) ON B.BiddingID = AB.BiddingID
			 INNER JOIN dbo.Auctions AC WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK) ON B.AuctionID = AC.AuctionID
			 LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID
			 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount C ON B.AuctionID = C.AuctionID
		WHERE B.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
			 AND AB.BiddingStatusCode = 1 AND AC.AuctionStatusCode <> 4
		ORDER BY B.RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	ELSE IF @inyBiddingStatusCode = 3
		WITH A AS (
			SELECT AuctionID, MAX(BiddingID) AS BiddingID
			FROM dbo.AuctionBidding WITH (NOLOCK, INDEX(IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode), FORCESEEK)
			WHERE CharacterID = @inbCharacterID 
			GROUP BY AuctionID
		), B AS (
			SELECT ROW_NUMBER() OVER(ORDER BY BiddingID DESC) AS RowNumber, AuctionID, BiddingID
			FROM A 		
		)
		SELECT B.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, 
			   CASE WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate > @dt2Now THEN 0
					WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate <= @dt2Now THEN 1
					WHEN AB.BiddingStatusCode = 1 THEN 2
					WHEN AB.BiddingStatusCode = 2 THEN 3 END AS BiddingStatusCode,
			   AC.BuynowPrice, AC.BiddingEndDate, C.MaxBiddingPrice AS BiddingPrice, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag
		FROM B INNER JOIN dbo.AuctionBidding AB WITH (NOLOCK, INDEX(PK_AuctionBidding), FORCESEEK) ON B.BiddingID = AB.BiddingID
			 INNER JOIN dbo.Auctions AC WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK) ON B.AuctionID = AC.AuctionID
			 LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID
			 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount C ON B.AuctionID = C.AuctionID
		WHERE B.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
			 AND AB.BiddingStatusCode = 2 AND AC.AuctionStatusCode <> 4
		ORDER BY B.RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	ELSE
		WITH A AS (
			SELECT AuctionID, MAX(BiddingID) AS BiddingID
			FROM dbo.AuctionBidding WITH (NOLOCK, INDEX(IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode), FORCESEEK)
			WHERE CharacterID = @inbCharacterID 
			GROUP BY AuctionID
		), B AS (
			SELECT ROW_NUMBER() OVER(ORDER BY BiddingID DESC) AS RowNumber, AuctionID, BiddingID
			FROM A 		
		)
		SELECT B.AuctionID, AC.ItemID, AI.ItemName, AI.LevelLimit, 
			   CASE WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate > @dt2Now THEN 0
					WHEN AB.BiddingStatusCode = 0 AND AC.BiddingEndDate <= @dt2Now THEN 1
					WHEN AB.BiddingStatusCode = 1 THEN 2
					WHEN AB.BiddingStatusCode = 2 THEN 3 END AS BiddingStatusCode,
			   AC.BuynowPrice, AC.BiddingEndDate, C.MaxBiddingPrice AS BiddingPrice, AC.ItemSerial, AC.ItemLevel, AC.SealCount, AC.ItemCount, AC.CoinCalculateFlag, AC.ItemCalculateFlag
		FROM B INNER JOIN dbo.AuctionBidding AB WITH (NOLOCK, INDEX(PK_AuctionBidding), FORCESEEK) ON B.BiddingID = AB.BiddingID
			 INNER JOIN dbo.Auctions AC WITH (NOLOCK, INDEX(PK_Autions), FORCESEEK) ON B.AuctionID = AC.AuctionID
			 LEFT OUTER JOIN dbo.AuctionItems AI WITH (NOLOCK) ON AC.ItemID = AI.ItemID
			 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount C ON B.AuctionID = C.AuctionID
		WHERE B.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
			  AND AC.AuctionStatusCode <> 4			 
		ORDER BY B.RowNumber
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
