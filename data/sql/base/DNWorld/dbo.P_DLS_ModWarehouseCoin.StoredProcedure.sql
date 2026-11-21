USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ModWarehouseCoin]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-18
description : ????? ?????. (???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103122 = @inyCoinChangeCode ? ??
103123 = ??? ?????.
103124 = ?? ??? ?????.
103298 = ?? ?/?? ??? ??? ? ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_ModWarehouseCoin]
	  @inbCharacterID bigint --// ??? ID
	, @inyCoinChangeCode tinyint --// 8=?? ??, 9=?? ??
	, @inbCoin bigint --// ?/???? ??
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID	
	, @inbCurrentCoin bigint OUTPUT --// ???? ?? ? ?? ??
	, @inbCurrentWarehouseCoin bigint OUTPUT --// ???? ?? ? ?? ?? ??
	, @bitAdminCall bit = 0 --// ??? ?? ?? (0=????, 1=??? ??)
	, @inyCharacterLevel tinyint = NULL OUTPUT --// ??? ??
	, @intCharacterEXP int = NULL OUTPUT --// ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbChangedCoin bigint
	, @dtmNow datetime = GETDATE();;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ???? ??? ?? LastServerType? ?????.*/
	IF @bitAdminCall = 0
	BEGIN
		/**_# Village Server???? ??? ??? 103298? ?????.*/
		IF NOT EXISTS (SELECT * FROM dbo.CharacterStatus WITH (NOLOCK) WHERE CharacterID = @inbCharacterID AND (LastServerType = 4 OR LastServerType IS NULL))
		BEGIN
			SET @intReturnValue = 103298;
			GOTO ErrorHandler;
		END
	END

	/**_# inyCoinChangeCode ?? 8 ?? 9? ??? 103122? ?????.*/
	IF @inyCoinChangeCode NOT IN (8, 9)
	BEGIN
		SET @intReturnValue = 103122;
		GOTO ErrorHandler;
	END

	/**_# [CharacterStatus] ???? UPDATE???.*/
	/**_## ??? ???? 103123? ?????.*/
	/**_## ?? ??? ???? 103124? ?????.*/
	UPDATE dbo.CharacterStatus
	SET @inbCurrentCoin = Coin = CASE @inyCoinChangeCode WHEN 8 THEN Coin - @inbCoin ELSE Coin + @inbCoin END
		, @inbCurrentWarehouseCoin = WarehouseCoin = CASE @inyCoinChangeCode WHEN 8 THEN WarehouseCoin + @inbCoin ELSE WarehouseCoin - @inbCoin END
		, @inyCharacterLevel = CharacterLevel
		, @intCharacterEXP = CharacterExp
	WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END

	/**_# [P_AddCoinLog]? ???? ?? ??? ?????.*/
	SET @inbChangedCoin = CASE @inyCoinChangeCode WHEN 8 THEN - @inbCoin ELSE @inbCoin END;

	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dtmNow
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = @inyCoinChangeCode
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @inbChangedCoin
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	/**_# [P_AddCoinLog]? ???? ?? ?? ??? ?????.*/
	SET @inbChangedCoin = - @inbChangedCoin;

	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dtmNow
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 2
		, @inyCoinChangeCode = @inyCoinChangeCode
		, @inbCurrentCoin = @inbCurrentWarehouseCoin
		, @inbChangedCoin = @inbChangedCoin
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_CharacterStatus_Coin%'
		SET @intReturnValue = 103123;

	ELSE IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_CharacterStatus_WarehouseCoin%'
		SET @intReturnValue = 103124;

	SELECT @inbCurrentCoin = Coin, @inbCurrentWarehouseCoin = WarehouseCoin
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
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
