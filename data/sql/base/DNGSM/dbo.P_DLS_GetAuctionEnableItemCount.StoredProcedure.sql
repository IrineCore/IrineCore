USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetAuctionEnableItemCount]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-15
description : ?? ?? ??? ???? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetAuctionEnableItemCount]
	  @vchItemCategoryIDs varchar(100) = '' --// ,? ?????? ItemCotegoryID ???
	, @nvcItemName nvarchar(50) = N'' --// ??? ??
	, @intTotalCount int OUTPUT --// ??? ???? ??? ?? ??? ?? ?????.
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int,
		@nvcStmt nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @nvcItemName = CASE WHEN @nvcItemName > N'' THEN N'%' + REPLACE(@nvcItemName, N' ', N'%') + N'%' ELSE N'' END;

	SET @nvcStmt = N'
	;WITH A AS (
		SELECT ItemID
		FROM dbo.AuctionEnableItemsFromDNT WITH (NOLOCK)			
		WHERE ItemID > 0 ' +				
				CASE WHEN @vchItemCategoryIDs > '' THEN N' AND ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END +
				CASE WHEN @nvcItemName > N'' THEN N' AND ItemName LIKE @nvcItemName' ELSE N'' END +N'
		UNION ALL
		SELECT ItemID
		FROM dbo.AuctionEnableItemsFromDOLIS WITH (NOLOCK)
		WHERE ItemID > 0 ' +				
				CASE WHEN @vchItemCategoryIDs > '' THEN N' AND ItemCategoryID IN (' + @vchItemCategoryIDs + N')' ELSE N'' END +
				CASE WHEN @nvcItemName > N'' THEN N' AND ItemName LIKE @nvcItemName' ELSE N'' END +N'
	)
	SELECT @intTotalCount = COUNT(*)
	FROM A
	OPTION (MAXDOP 1, KEEP PLAN, KEEPFIXED PLAN)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcItemName nvarchar(50)			
			, @intTotalCount int OUTPUT
		'
		, @nvcItemName = @nvcItemName				
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
