USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountTradeNew]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-01-17
description : ??? ?? ??? ????? ?? ???? ??????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103171 = ??? ???? ???? ????.
history :
2 = KEEP PLAN, KEEPFIXED PLAN ?? ?? at 2011-01-18
3 = @bitCashItemFlag ?? NULL?? ?? ????? ?? at 2011-01-27
4 = @nvcItemName? ?? ??? ?? ??? ??? AND ????? ?? at 2011-02-07
**/

CREATE PROCEDURE [dbo].[P_GetCountTradeNew]
	  @vchItemCategoryIDs varchar(100) = '' --// ,? ?????? ItemCotegoryID ???
	, @nvcItemName nvarchar(50) = N'' --// ??? ??
	, @inyMinLevelLimit tinyint = 0 --// ?? ??
	, @inyMaxLevelLimit tinyint = 0 --// ?? ??
	, @vchJobCodes varchar(100) = '' --// ,? ?????? JobCode ???
	, @vchItemRankCodes varchar(100) = '' --// ,? ?????? ItemRankCode ???
	, @bitCashItemFlag bit = 0 --// 0=?? ???, 1=?? ???
	, @inyPayMethodCode tinyint = 1 --// 1=????, 3=??
	, @intTotalCount int OUTPUT --// ??? ???? ??? ?? ??? ?? ?????.
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
	SET @nvcItemName = CASE WHEN @nvcItemName > N'' THEN N'%' + REPLACE(@nvcItemName, N' ', N'%') + N'%' ELSE N'' END;

	SET @nvcStmt = N'
SELECT @intTotalCount = COUNT(*)
FROM dbo.TradeItems I WITH (NOLOCK)
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
OPTION (MAXDOP 1, KEEP PLAN, KEEPFIXED PLAN)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @inyMinLevelLimit tinyint
			, @inyMaxLevelLimit tinyint
			, @nvcItemName nvarchar(50)
			, @inyPayMethodCode tinyint
			, @dt2Now datetime2(0)
			, @bitCashItemFlag bit
			, @intTotalCount int OUTPUT
		'
		, @inyMinLevelLimit = @inyMinLevelLimit
		, @inyMaxLevelLimit = @inyMaxLevelLimit
		, @nvcItemName = @nvcItemName
		, @inyPayMethodCode = @inyPayMethodCode
		, @dt2Now = @dt2Now
		, @bitCashItemFlag = @bitCashItemFlag
		, @intTotalCount = @intTotalCount OUTPUT;
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
