USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddWishAuction]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-28
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000034 = ?? ????? ???? ????.
1030000035 = ???? ??????? ??? ? ????.
1030000044 = ???? ?? ??ID???.
**/

CREATE PROCEDURE [dbo].[P_DRS_AddWishAuction]
	  @intAuctionID int --// ??ID
	, @inbCharacterID bigint --// ???ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intAuctionCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intAuctionCount = COUNT(*)
	FROM dbo.WishAuctions WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	IF @intAuctionCount >= 5
	BEGIN
		SET @intReturnValue = 1030000035;
		GOTO ErrorHandler;
	END

	/**_# [WishAuctions] ???? INSERT???.*/
	INSERT dbo.WishAuctions (AuctionID, CharacterID)
	VALUES (@intAuctionID, @inbCharacterID);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_%'
		SET @intReturnValue = 1030000034;
	ELSE IF ERROR_NUMBER() = 5000 AND ERROR_MESSAGE() LIKE N'%FK_%'
		SET @intReturnValue = 1030000044;
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
