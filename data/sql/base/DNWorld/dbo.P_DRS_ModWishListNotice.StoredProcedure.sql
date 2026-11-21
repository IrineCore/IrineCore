USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_ModWishListNotice]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-04-25
description : ????? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_ModWishListNotice]
	  @intWishListID int --// ????? ID	
	, @intNoticePrice int = NULL --// ????? ??
	, @inyNoticeItemLevel tinyint = NULL --// ????? ????
	, @bitNoticeFlag bit = NULL --// ??????
	, @inbCharacterID bigint = NULL --// ??? ID
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
	/**_# [WishList] ???? Update???.*/
	UPDATE dbo.WishList
	SET NoticeFlag = COALESCE(@bitNoticeFlag,NoticeFlag),
		NoticePrice = COALESCE(@intNoticePrice,NoticePrice),
		NoticeItemLevel = COALESCE(@inyNoticeItemLevel,NoticeItemLevel)
	WHERE CharacterID = @inbCharacterID
		AND WishListID = @intWishListID;	
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
