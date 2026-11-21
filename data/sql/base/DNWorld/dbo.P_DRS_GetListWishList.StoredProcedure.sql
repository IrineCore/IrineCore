USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListWishList]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-04-04
description : ???? ?????? ?????. (For Mobile)
result set :
	WishListID/int, ItemID/int, ItemLevel/tinyint, TradeCount/int, MinPrice/int, NoticePrice/int, NoticeItemLevel/int, NoticeFlag/bit, NoticeCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000001 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListWishList]
	  @inbCharacterID bigint --// ???ID	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT WL.WishListID, WL.ItemID, WL.ItemLevel, TT.TradeCount, TT.MinPrice, WL.NoticePrice, WL.NoticeItemLevel, WL.NoticeFlag, 0 AS NoticeCount
	FROM dbo.WishList WL WITH (NOLOCK, INDEX(PK_WishList), FORCESEEK)
		OUTER APPLY dbo.IF_TradeItemTotals(WL.ItemID, WL.NoticeItemLevel, WL.NoticePrice, WL.NoticeFlag) TT
	WHERE WL.CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000001;
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
