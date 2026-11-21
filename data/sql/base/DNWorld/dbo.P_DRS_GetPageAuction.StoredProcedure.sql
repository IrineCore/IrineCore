USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetPageAuction]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-26
description : ?? ???? ??????.
result set : AuctionID/int, CharacterName/nvarchar, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, SealCount/tinyint, ItemPotentialMoveCount/tinyint, LevelLimit/tinyint, AuctionStatusCode/tinyint, BuynowPrice/bigint, BiddingEndDate/datetime2, BiddingPrice/bigint, MyBiddingStatusCode/tinyint, ItemSerial/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetPageAuction]
	  @intPageNumber int --// ??? ??
	, @intPageSize int --// ??? ???
	, @inbCharacterID bigint --// ???ID (???? ???)
	, @vchItemCategoryIDs varchar(100) = '' --// ,? ?????? ItemCotegoryID ???
	, @nvcItemName nvarchar(50) = N'' --// ??? ??
	, @inyMinLevelLimit tinyint = 0 --// ?? ??
	, @inyMaxLevelLimit tinyint = 0 --// ?? ??
	, @vchJobCodes varchar(100) = '' --// ,? ?????? JobCode ???
	, @vchItemRankCodes varchar(100) = '' --// ,? ?????? ItemRankCode ???
	, @bitCashItemFlag bit = 0 --// 0=?? ???, 1=?? ???	
	, @inySortType tinyint = 1 --// ?? ??. 0=???? ??, 1=???? ??, 2=??? ??, 3=??? ??, 4=??? ??, 5=??? ??, 6=????? ??, 7=????? ??
	, @intTotalCount int OUTPUT --// ??? ???? ?? ?? ?????. (1?????? ?????.)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int,
		@nvcStmt nvarchar(max),
		@dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intPageNumber = 1
		EXEC @intReturnValue = dbo.P_DRS_GetAuctionCount
			  @vchItemCategoryIDs = @vchItemCategoryIDs
			, @nvcItemName = @nvcItemName
			, @inyMinLevelLimit = @inyMinLevelLimit
			, @inyMaxLevelLimit = @inyMaxLevelLimit
			, @vchJobCodes = @vchJobCodes
			, @vchItemRankCodes = @vchItemRankCodes
			, @bitCashItemFlag = @bitCashItemFlag			
			, @intTotalCount = @intTotalCount OUTPUT;

	SET @nvcItemName = CASE WHEN @nvcItemName > N'' THEN N'%' + REPLACE(@nvcItemName, N' ', N'%') + N'%' ELSE N'' END;

	IF @nvcItemName > N''
		SET @nvcStmt = N'
WITH A AS (
	SELECT ROW_NUMBER() OVER(ORDER BY ' +
		CASE @inySortType
			WHEN 0 THEN N'A.BiddingEndDate, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 1 THEN N'A.BiddingEndDate DESC, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 2 THEN N'S.BiddingCount, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 3 THEN N'S.BiddingCount DESC, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 4 THEN N'S.MaxBiddingPrice, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 5 THEN N'S.MaxBiddingPrice DESC, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 6 THEN N'A.BuynowPrice, I.ItemName, A.ItemLevel DESC'
			WHEN 7 THEN N'A.BuynowPrice DESC, I.ItemName, A.ItemLevel DESC'
		END + N') AS RowNumber, A.AuctionID, A.CharacterID, A.ItemSerial, A.ItemID, A.ItemCount, A.ItemLevel, A.ItemDurability, A.RandomSeed, A.ItemPotential, A.ItemOption, A.SealCount, A.ItemPotentialMoveCount, I.LevelLimit
		  , CASE WHEN A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
					WHEN (A.AuctionStatusCode = 2 OR (A.BiddingEndDate <= @dt2Now AND ISNULL(S.BiddingCount, 0) > 0)) THEN 1
					WHEN A.BiddingEndDate <= @dt2Now AND ISNULL(S.BiddingCount, 0) = 0 THEN 2
                    ELSE 0 END AS AuctionStatusCode	
		  , A.BuynowPrice, A.BiddingEndDate, S.MaxBiddingPrice, S.BiddingCount
	FROM dbo.Auctions A WITH (NOLOCK, INDEX(IX_NNI_Auctions_ItemID), FORCESEEK)
		INNER JOIN dbo.AuctionItems I WITH (NOLOCK, INDEX(PK_AuctionItems)) ON A.ItemID = I.ItemID
		LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount AS S ON A.AuctionID = S.AuctionID
	WHERE A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now ' +
		CASE WHEN @bitCashItemFlag >= 0 THEN N' AND I.CashItemFlag = @bitCashItemFlag' ELSE N'' END +
		CASE WHEN @vchItemCategoryIDs > '' THEN N' AND I.ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END +
		CASE WHEN @nvcItemName > N'' THEN N' AND I.ItemName LIKE @nvcItemName' ELSE N'' END +
		CASE WHEN @inyMinLevelLimit > 0 THEN N' AND I.LevelLimit >= @inyMinLevelLimit' ELSE N'' END +
		CASE WHEN @inyMaxLevelLimit > 0 THEN N' AND I.LevelLimit <= @inyMaxLevelLimit' ELSE N'' END +
		CASE WHEN @vchJobCodes > '' THEN N' AND I.JobCode IN (' + @vchJobCodes + N')' ELSE N'' END +
		CASE WHEN @vchItemRankCodes > '' THEN N' AND I.ItemRankCode IN (' + @vchItemRankCodes + N')' ELSE N'' END + N'
), C AS (
	SELECT AuctionID, MAX(BiddingID) AS BiddingID
	FROM dbo.AuctionBidding WITH (NOLOCK, INDEX(IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode), FORCESEEK)
	WHERE CharacterID = @inbCharacterID 
	GROUP BY AuctionID
), D AS (
	SELECT C.AuctionID, AB.BiddingStatusCode
	FROM C INNER JOIN dbo.AuctionBidding AB ON C.BiddingID = AB.BiddingID
)
SELECT A.AuctionID, CH.CharacterName, A.ItemID, A.ItemCount, A.ItemDurability, A.RandomSeed, A.ItemLevel, A.ItemPotential, A.ItemOption, A.SealCount, A.ItemPotentialMoveCount
      , A.LevelLimit, A.AuctionStatusCode, A.BuynowPrice, A.BiddingEndDate, ISNULL(A.MaxBiddingPrice, 0) AS BiddingPrice, 
	  CASE WHEN D.BiddingStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
		   WHEN D.BiddingStatusCode = 0 AND A.BiddingEndDate <= @dt2Now THEN 1
		   WHEN D.BiddingStatusCode = 1 THEN 2
		   WHEN D.BiddingStatusCode = 2 THEN 3 END AS MyBiddingStatusCode, A.ItemSerial
FROM A	
	INNER JOIN dbo.Characters CH WITH (NOLOCK) ON A.CharacterID = CH.CharacterID	
	LEFT OUTER JOIN D ON A.AuctionID = D.AuctionID	
WHERE A.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY A.RowNumber
OPTION (KEEPFIXED PLAN, MAXDOP 1);';

	ELSE
		SET @nvcStmt = N'
WITH A AS (
	SELECT ROW_NUMBER() OVER(ORDER BY ' +
		CASE @inySortType
			WHEN 0 THEN N'A.BiddingEndDate, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 1 THEN N'A.BiddingEndDate DESC, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 2 THEN N'S.BiddingCount, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 3 THEN N'S.BiddingCount DESC, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 4 THEN N'S.MaxBiddingPrice, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 5 THEN N'S.MaxBiddingPrice DESC, I.ItemName, A.ItemLevel DESC, A.BuynowPrice ASC'
			WHEN 6 THEN N'A.BuynowPrice, I.ItemName, A.ItemLevel DESC'
			WHEN 7 THEN N'A.BuynowPrice DESC, I.ItemName, A.ItemLevel DESC'
		END + N') AS RowNumber, A.AuctionID, A.CharacterID, A.ItemSerial, A.ItemID, A.ItemCount, A.ItemLevel, A.ItemDurability, A.RandomSeed, A.ItemPotential, A.ItemOption, A.SealCount, A.ItemPotentialMoveCount, I.LevelLimit
		  , CASE WHEN A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
					WHEN (A.AuctionStatusCode = 2 OR (A.BiddingEndDate <= @dt2Now AND ISNULL(S.BiddingCount, 0) > 0)) THEN 1
					WHEN A.BiddingEndDate <= @dt2Now AND ISNULL(S.BiddingCount, 0) = 0 THEN 2
                    ELSE 0 END AS AuctionStatusCode	
		  , A.BuynowPrice, A.BiddingEndDate, S.MaxBiddingPrice, S.BiddingCount
	FROM dbo.Auctions A WITH (NOLOCK, INDEX(IX_NNI_Auctions_ItemID), FORCESEEK)
		INNER JOIN dbo.AuctionItems I WITH (NOLOCK, INDEX(PK_AuctionItems)) ON A.ItemID = I.ItemID
		LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount AS S ON A.AuctionID = S.AuctionID
	WHERE A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now ' +
		CASE WHEN @bitCashItemFlag >= 0 THEN N' AND I.CashItemFlag = @bitCashItemFlag' ELSE N'' END +
		CASE WHEN @vchItemCategoryIDs > '' THEN N' AND I.ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END +		
		CASE WHEN @inyMinLevelLimit > 0 THEN N' AND I.LevelLimit >= @inyMinLevelLimit' ELSE N'' END +
		CASE WHEN @inyMaxLevelLimit > 0 THEN N' AND I.LevelLimit <= @inyMaxLevelLimit' ELSE N'' END +
		CASE WHEN @vchJobCodes > '' THEN N' AND I.JobCode IN (' + @vchJobCodes + N')' ELSE N'' END +
		CASE WHEN @vchItemRankCodes > '' THEN N' AND I.ItemRankCode IN (' + @vchItemRankCodes + N')' ELSE N'' END + N'
), C AS (
	SELECT AuctionID, MAX(BiddingID) AS BiddingID
	FROM dbo.AuctionBidding WITH (NOLOCK, INDEX(IX_NNI_AuctionBidding_CharacterID_BiddingStatusCode), FORCESEEK)
	WHERE CharacterID = @inbCharacterID 
	GROUP BY AuctionID
), D AS (
	SELECT C.AuctionID, AB.BiddingStatusCode
	FROM C INNER JOIN dbo.AuctionBidding AB ON C.BiddingID = AB.BiddingID
)
SELECT A.AuctionID, CH.CharacterName, A.ItemID, A.ItemCount, A.ItemDurability, A.RandomSeed, A.ItemLevel, A.ItemPotential, A.ItemOption, A.SealCount, A.ItemPotentialMoveCount
      , A.LevelLimit, A.AuctionStatusCode, A.BuynowPrice, A.BiddingEndDate, ISNULL(A.MaxBiddingPrice, 0) AS BiddingPrice, 
	  CASE WHEN D.BiddingStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
		   WHEN D.BiddingStatusCode = 0 AND A.BiddingEndDate <= @dt2Now THEN 1
		   WHEN D.BiddingStatusCode = 1 THEN 2
		   WHEN D.BiddingStatusCode = 2 THEN 3 END AS MyBiddingStatusCode, A.ItemSerial
FROM A	
	INNER JOIN dbo.Characters CH WITH (NOLOCK) ON A.CharacterID = CH.CharacterID	
	LEFT OUTER JOIN D ON A.AuctionID = D.AuctionID	
WHERE A.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY A.RowNumber
OPTION (KEEPFIXED PLAN, MAXDOP 1);';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @inyMinLevelLimit tinyint
			, @inyMaxLevelLimit tinyint
			, @nvcItemName nvarchar(50)			
			, @dt2Now datetime2(0)
			, @bitCashItemFlag bit
			, @intPageNumber int
			, @intPageSize int
			, @inbCharacterID bigint
		'
		, @inyMinLevelLimit = @inyMinLevelLimit
		, @inyMaxLevelLimit = @inyMaxLevelLimit
		, @nvcItemName = @nvcItemName
		, @dt2Now = @dt2Now
		, @bitCashItemFlag = @bitCashItemFlag
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize
		, @inbCharacterID = @inbCharacterID;
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
