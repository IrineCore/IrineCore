USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_WithdrawWarehouseCoin]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-18
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000028 = ???? ???? ????.
1030000057 = ?? ??? ?????.
1030000058 = 42? ??? ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_WithdrawWarehouseCoin]
	  @inbCharacterID bigint --// ??? ID	
	, @inbCoin bigint --// ???? ??	
	, @inbCurrentCoin bigint OUTPUT --// ???? ?? ? ?? ??
	, @inbCurrentWarehouseCoin bigint OUTPUT --// ???? ?? ? ?? ?? ??
	, @intCharacterExp int OUTPUT ---// ?? ??? ???
	, @inyCharacterLevel tinyint OUTPUT ---// ?? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbChangedCoin bigint
	, @dtmNow datetime = GETDATE();

DECLARE @inbTmp1 bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT @inbTmp1 = Coin FROM dbo.CharacterStatus WITH (NOLOCK, FORCESEEK) WHERE CharacterID = @inbCharacterID OPTION (KEEPFIXED PLAN);	

	IF (@inbTmp1 + @inbCoin) > 4200000000
	BEGIN
		SET @intReturnValue = 1030000058;
		GOTO ErrorHandler;
	END

	/**_# [CharacterStatus] ???? UPDATE???.*/	
	/**_## ?? ??? ???? 1030000057? ?????.*/
	UPDATE dbo.CharacterStatus
	SET @inbCurrentCoin = Coin = Coin + @inbCoin
		, @inbCurrentWarehouseCoin = WarehouseCoin = WarehouseCoin - @inbCoin
		, @intCharacterExp = CharacterExp
		, @inyCharacterLevel = CharacterLevel 
	WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 1030000028? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000028;
		GOTO ErrorHandler;
	END

	/**_# [P_AddCoinLog]? ???? ?? ??? ?????.*/
	SET @inbChangedCoin = @inbCoin;

	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dtmNow
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = 9
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @inbChangedCoin
		, @intChannelID = NULL
		, @intMapID = NULL;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	/**_# [P_AddCoinLog]? ???? ?? ?? ??? ?????.*/
	SET @inbChangedCoin = - @inbChangedCoin;

	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dtmNow
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 2
		, @inyCoinChangeCode = 9
		, @inbCurrentCoin = @inbCurrentWarehouseCoin
		, @inbChangedCoin = @inbChangedCoin
		, @intChannelID = NULL
		, @intMapID = NULL;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_CharacterStatus_WarehouseCoin%'
		SET @intReturnValue = 1030000057;	

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
