USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddWishList]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-04-04
description : ?????? ???? ?????.(For Mobile)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000017 = ????? ?? ?? ??? ??????.
**/

CREATE PROCEDURE [dbo].[P_DRS_AddWishList]
	  @inbCharacterID bigint --// ???ID
	, @intItemID int --// ???ID
	, @inyItemLevel tinyint = NULL --// ??? ???? (Optional)
	, @bitNoticeFlag bit = 0 --// ???? (Optional)
	, @intNoticePrice int = NULL --// ?? ?? ?? (Optional)
	, @inyNoticeItemLevel tinyint = NULL--// ???? ???? (Optional)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inyWishListCount tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# WishList ?? ??? 5?? ????? ?????.*/
	SELECT @inyWishListCount = COUNT(*) FROM dbo.WishList WITH (NOLOCK) WHERE CharacterID = @inbCharacterID;

	IF @inyWishListCount >= 5 
	BEGIN
		SET @intReturnValue = 1030000017;
		GOTO ErrorHandler;
	END

	/**_# [WishList] ???? INSERT???.*/
	INSERT dbo.WishList (CharacterID, ItemID, ItemLevel, NoticeFlag, NoticePrice, NoticeItemLevel)
	VALUES (@inbCharacterID, @intItemID, ISNULL(@inyItemLevel, 0), @bitNoticeFlag, @intNoticePrice, @inyNoticeItemLevel);
	
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
