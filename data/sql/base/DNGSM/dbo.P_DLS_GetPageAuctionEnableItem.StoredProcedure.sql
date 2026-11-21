USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetPageAuctionEnableItem]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-15
description : ?? ?? ???? ??????.
result set : ItemID/int, ItemCategoryID/tinyint, ItemName/nvarchar, ActivateFlag/bit, SourceCode/tinyint(1=DNT, 2=DB)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetPageAuctionEnableItem]
	  @intPageNumber int --// ??? ??
	, @intPageSize int --// ??? ???
	, @vchItemCategoryIDs varchar(100) = '' --// ,? ?????? ItemCotegoryID ???
	, @nvcItemName nvarchar(50) = N'' --// ??? ??
	, @intTotalCount int OUTPUT --// ??? ???? ?? ?? ?????. (1?????? ?????.)
WITH EXECUTE AS 'SPExecutor'
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
		EXEC @intReturnValue = dbo.P_DLS_GetAuctionEnableItemCount
			  @vchItemCategoryIDs = @vchItemCategoryIDs
			, @nvcItemName = @nvcItemName			
			, @intTotalCount = @intTotalCount OUTPUT;

	SET @nvcItemName = CASE WHEN @nvcItemName > N'' THEN N'%' + REPLACE(@nvcItemName, N' ', N'%') + N'%' ELSE N'' END;

	IF @nvcItemName > N''
		SET @nvcStmt = N'
WITH A AS (
	SELECT ItemID, ItemCategoryID, ItemName, ActivateFlag, 1 AS SourceCode
	FROM dbo.AuctionEnableItemsFromDNT WITH (NOLOCK)		
	WHERE ItemID >= 0 ' +		
		CASE WHEN @vchItemCategoryIDs > '' THEN N' AND ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END +
		CASE WHEN @nvcItemName > N'' THEN N' AND ItemName LIKE @nvcItemName' ELSE N'' END + N'
	UNION ALL
	SELECT ItemID, ItemCategoryID, ItemName, ActivateFlag, 2 AS SourceCode
	FROM dbo.AuctionEnableItemsFromDOLIS WITH (NOLOCK)		
	WHERE ItemID >= 0 ' +		
		CASE WHEN @vchItemCategoryIDs > '' THEN N' AND ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END +
		CASE WHEN @nvcItemName > N'' THEN N' AND ItemName LIKE @nvcItemName' ELSE N'' END + N'
), B AS (
	SELECT ROW_NUMBER() OVER(ORDER BY ItemID) AS RowNumber, ItemID, ItemCategoryID, ItemName, ActivateFlag, SourceCode
	FROM A 
)
SELECT ItemID, ItemCategoryID, ItemName, ActivateFlag, SourceCode
FROM B		
WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY RowNumber
OPTION (KEEPFIXED PLAN, MAXDOP 1);';

	ELSE
		SET @nvcStmt = N'
WITH A AS (
	SELECT ItemID, ItemCategoryID, ItemName, ActivateFlag, 1 AS SourceCode
	FROM dbo.AuctionEnableItemsFromDNT WITH (NOLOCK)		
	WHERE ItemID >= 0 ' +		
		CASE WHEN @vchItemCategoryIDs > '' THEN N' AND ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END + N'
	UNION ALL
	SELECT ItemID, ItemCategoryID, ItemName, ActivateFlag, 2 AS SourceCode
	FROM dbo.AuctionEnableItemsFromDOLIS WITH (NOLOCK)		
	WHERE ItemID >= 0 ' +		
		CASE WHEN @vchItemCategoryIDs > '' THEN N' AND ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END + N'
), B AS (
	SELECT ROW_NUMBER() OVER(ORDER BY ItemID) AS RowNumber, ItemID, ItemCategoryID, ItemName, ActivateFlag, SourceCode
	FROM A 
)
SELECT ItemID, ItemCategoryID, ItemName, ActivateFlag, SourceCode
FROM B		
WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY RowNumber
OPTION (KEEPFIXED PLAN, MAXDOP 1);';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcItemName nvarchar(50)						
			, @intPageNumber int
			, @intPageSize int			
		'
		, @nvcItemName = @nvcItemName		
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;
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
