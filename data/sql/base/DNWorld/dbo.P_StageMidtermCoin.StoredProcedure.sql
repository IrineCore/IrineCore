USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_StageMidtermCoin]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-11
description : ?????? ?? ??? ???.(??!! ??? ??? ? ? ????.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_StageMidtermCoin]
	  @inbCharacterID bigint --// ??? ID
	, @inbCoin bigint --// ?? ??? (+? ??)
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @inbCurrentCoin bigint = NULL OUTPUT --// ???? ?? ? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dtmNow datetime = GETDATE()
	, @inbCoinBefore bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inbCoin < 0
		RETURN 0;

	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @inbCoinBefore = Coin
		, @inbCurrentCoin = Coin += @inbCoin 
	WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END
	
	IF @inbCoin > 0
		EXEC @intReturnValue = dbo.P_AddCoinLog
			  @dt2LogDate = @dtmNow
			, @inbCharacterID = @inbCharacterID
			, @inyCoinTypeCode = 1
			, @inyCoinChangeCode = 2
			, @inbCurrentCoin = @inbCurrentCoin
			, @inbChangedCoin = @inbCoin
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID;
	

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
