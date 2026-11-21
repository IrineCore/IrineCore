USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ForfeitCoin]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-10-06
description : ?? ????? ?????. (??? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ForfeitCoin]
	  @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dtmNow datetime = GETDATE()
	, @inbChangedCoin bigint
	, @inbCoinBefore bigint
	, @inbChangedWarehouseCoin bigint
	, @inbWarehouseCoinBefore bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @inbCoinBefore = Coin
		, @inbWarehouseCoinBefore = WarehouseCoin
		, Coin = 0
		, WarehouseCoin = 0
	WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END

	/**_# [P_AddCoinLog]? ???? ?? ??? ?????.*/
	SET @inbChangedCoin = - @inbCoinBefore;
	SET @inbChangedWarehouseCoin = - @inbWarehouseCoinBefore;

	IF @inbChangedCoin <> 0
		EXEC dbo.P_AddCoinLog
			  @dt2LogDate = @dtmNow
			, @inbCharacterID = @inbCharacterID
			, @inyCoinTypeCode = 1
			, @inyCoinChangeCode = 4
			, @inbCurrentCoin = 0
			, @inbChangedCoin = @inbChangedCoin
			, @intChannelID = NULL
			, @intMapID = NULL
			, @inbCoinChangeFKey = NULL;

	IF @inbChangedWarehouseCoin <> 0
		EXEC dbo.P_AddCoinLog
			  @dt2LogDate = @dtmNow
			, @inbCharacterID = @inbCharacterID
			, @inyCoinTypeCode = 2
			, @inyCoinChangeCode = 4
			, @inbCurrentCoin = 0
			, @inbChangedCoin = @inbChangedWarehouseCoin
			, @intChannelID = NULL
			, @intMapID = NULL
			, @inbCoinChangeFKey = NULL;

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
