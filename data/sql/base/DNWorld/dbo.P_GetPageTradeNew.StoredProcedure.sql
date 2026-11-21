USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageTradeNew]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 17
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-01-17

description :
	??? ?? ??? ????? ?????? ?????.
result set : 
	@intVersion < 9 : TradeID/int, CharacterName/nvarchar, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, ListPrice/int, SealCount/tinyint, PremiumFlag/bit, PayMethodCode/tinyint
	@intVersion = 9 : TradeID/int, CharacterName/nvarchar, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, ListPrice/int, SealCount/tinyint, PremiumFlag/bit, PayMethodCode/tinyint, ItemPotentialMoveCount/tinyint
	@intVersion = 10 : Version 9 + DragonJewelType/tinyint, ItemSerial/bigint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103171 = ??? ???? ???? ????.
history :
	2 = KEEP PLAN, KEEPFIXED PLAN ?? ?? at 2011-01-18
	3 = ?? ?? ?? ?? at 2011-01-25
	4 = @bitCashItemFlag ?? NULL?? ?? ????? ?? at 2011-01-27 
	5 = @nvcItemName? ?? ??? ?? ??? ??? AND ????? ?? at 2011-02-07
	7 = ?? ?? at 2011-07-20
	8 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
	9~12 = ?? ?? ?? ?? (@intVersion = 9)     by ??? at 2012-04-10
	16 = Characters??? ?? ??     by ??? at 2013-05-15
	17 = Trade ??? ?? ?? ??? ??     by ??? at 2013-06-11
**/

CREATE PROCEDURE [dbo].[P_GetPageTradeNew]
	  @intPageNumber int --// ??? ??
	, @intPageSize int --// ??? ???
	, @vchItemCategoryIDs varchar(100) = '' --// ,? ?????? ItemCotegoryID ???
	, @nvcItemName nvarchar(50) = N'' --// ??? ??
	, @inyMinLevelLimit tinyint = 0 --// ?? ??
	, @inyMaxLevelLimit tinyint = 0 --// ?? ??
	, @vchJobCodes varchar(100) = '' --// ,? ?????? JobCode ???
	, @vchItemRankCodes varchar(100) = '' --// ,? ?????? ItemRankCode ???
	, @bitCashItemFlag bit = 0 --// 0=?? ???, 1=?? ???
	, @inyPayMethodCode tinyint = 1 --// 1=????, 3=??
	, @inySortType tinyint = 4 --// ?? ??. 0=?? ??, 1=?? ??, 2=?? ??, 3=?? ??, 4=?? ??, 5=?? ??, 6=???? ??, 7=???? ??
	, @intTotalCount int OUTPUT --// ??? ???? ??? ?? ??? ?? ?????. (1?????? ?????.)
	, @intVersion int = 8 --// ??? SP? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
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
		EXEC @intReturnValue = dbo.P_GetCountTradeNew
			  @vchItemCategoryIDs = @vchItemCategoryIDs
			, @nvcItemName = @nvcItemName
			, @inyMinLevelLimit = @inyMinLevelLimit
			, @inyMaxLevelLimit = @inyMaxLevelLimit
			, @vchJobCodes = @vchJobCodes
			, @vchItemRankCodes = @vchItemRankCodes
			, @bitCashItemFlag = @bitCashItemFlag
			, @inyPayMethodCode = @inyPayMethodCode
			, @intTotalCount = @intTotalCount OUTPUT;

	SET @nvcItemName = CASE WHEN @nvcItemName > N'' THEN N'%' + REPLACE(@nvcItemName, N' ', N'%') + N'%' ELSE N'' END;

	SET @nvcStmt = N'
WITH C AS (
	SELECT ROW_NUMBER() OVER(ORDER BY TR.PremiumFlag DESC, ' +
		CASE @inySortType
			WHEN 0 THEN N'I.ItemName, TR.ItemLevel DESC, TR.ListPrice ASC'
			WHEN 1 THEN N'I.ItemName DESC, TR.ItemLevel DESC, TR.ListPrice ASC'
			WHEN 2 THEN N'I.LevelLimit, I.ItemName, TR.ItemLevel DESC, TR.ListPrice ASC'
			WHEN 3 THEN N'I.LevelLimit DESC, I.ItemName, TR.ItemLevel DESC, TR.ListPrice ASC'
			WHEN 4 THEN N'TR.ListPrice, I.ItemName, TR.ItemLevel DESC'
			WHEN 5 THEN N'TR.ListPrice DESC, I.ItemName, TR.ItemLevel DESC'
			WHEN 6 THEN N'TR.ItemLevel, TR.ListPrice ASC'
			WHEN 7 THEN N'TR.ItemLevel DESC, TR.ListPrice ASC'
		END + N') AS RowNumber, TR.TradeID
	FROM dbo.TradeItems I WITH (NOLOCK, INDEX(PK_TradeItems))
		INNER JOIN dbo.Trade TR WITH (NOLOCK, INDEX(IX_NNIF_Trade_ItemID), FORCESEEK) ON TR.ItemID = I.ItemID
	WHERE TR.TradeStatusCode = 0 AND TR.TradeExpireDate > @dt2Now' +
		CASE WHEN @bitCashItemFlag >= 0 THEN N' AND I.CashItemFlag = @bitCashItemFlag' ELSE N'' END +
		CASE WHEN @vchItemCategoryIDs > '' THEN N' AND I.ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END +
		CASE WHEN @nvcItemName > N'' THEN N' AND I.ItemName LIKE @nvcItemName' ELSE N'' END +
		CASE WHEN @inyMinLevelLimit > 0 THEN N' AND I.LevelLimit >= @inyMinLevelLimit' ELSE N'' END +
		CASE WHEN @inyMaxLevelLimit > 0 THEN N' AND I.LevelLimit <= @inyMaxLevelLimit' ELSE N'' END +
		CASE WHEN @vchJobCodes > '' THEN N' AND I.JobCode IN (' + @vchJobCodes + N')' ELSE N'' END +
		CASE WHEN @vchItemRankCodes > '' THEN N' AND I.ItemRankCode IN (' + @vchItemRankCodes + N')' ELSE N'' END +
		CASE WHEN @inyPayMethodCode > 0 THEN N' AND TR.PayMethodCode = @inyPayMethodCode' ELSE N'' END + N'
)
SELECT TR.TradeID, TR.CharacterName, TR.ItemID, TR.ItemCount, TR.ItemDurability, TR.RandomSeed, TR.ItemLevel, TR.ItemPotential, TR.ItemOption, TR.Price, TR.ListPrice, TR.SealCount, TR.PremiumFlag, TR.PayMethodCode' + CASE WHEN @intVersion = 9 THEN N', TR.ItemPotentialMoveCount' ELSE N'' END + CASE WHEN @intVersion = 10 THEN N', TR.ItemPotentialMoveCount, TR.DragonJewelType, TR.ItemSerial' ELSE N'' END + N'
FROM C
	INNER JOIN dbo.Trade TR WITH(NOLOCK, INDEX(PK_Trade), FORCESEEK) ON TR.TradeID = C.TradeID
WHERE C.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY C.RowNumber
OPTION (KEEPFIXED PLAN, MAXDOP 1);';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @inyMinLevelLimit tinyint
			, @inyMaxLevelLimit tinyint
			, @nvcItemName nvarchar(50)
			, @inyPayMethodCode tinyint
			, @dt2Now datetime2(0)
			, @bitCashItemFlag bit
			, @intPageNumber int
			, @intPageSize int
		'
		, @inyMinLevelLimit = @inyMinLevelLimit
		, @inyMaxLevelLimit = @inyMaxLevelLimit
		, @nvcItemName = @nvcItemName
		, @inyPayMethodCode = @inyPayMethodCode
		, @dt2Now = @dt2Now
		, @bitCashItemFlag = @bitCashItemFlag
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, 103171? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103171;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog;
RETURN @intReturnValue;
GO
