USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_MoveIntoNewZone]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 34
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-26
description : ?? Zone?? ???? ?????. ?? ???, ?? ??, ??? ??/PC? ?? ??? DB? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103294 = ?????? ??? ????? 20??? ???????.
103300 = ??? ????? ???? ????.
history :
19 = ???, ??? ???? ?? ??? ?? at 2010-12-08
21 = ?? ??? 20?? ???? 103294? ????? ?? at 2011-01-04
22 = ver. 21? ?? ?? (20??? ?? 100??? ????? ??) at 2011-01-13
23 = input param. @inbGainCoin ?? (zone?? ??? ????? ??? ?? ??? ??) at 2011-03-03
27 = return value ?? at 2011-03-04
28 = ??? ?? ?? at 2011-03-18
29 = ??? ??? ?? ?? at 2011-03-29
30 = input param. @inbPetItemSerial, @intGainPetExp ?? at 2011-05-12
31 = 20?? ???? ???? ?? at 2011-05-30
33 = ????? ?? by ??? at 2012-07-25
34 = ??? ???? ?? by ??? at 2012-07-25
**/

CREATE PROCEDURE [dbo].[P_MoveIntoNewZone]
	  @inbCharacterID bigint --// ??? ID
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @intPartyID int --// (optional) ??? ?? ?? ID
	, @intCharacterExp int --// ?? ??? ??? (??? ??? ??? ??? ?!!!)
	, @inbCoin bigint --// ?? ?? ?? (??? ??? ??? ?? ?!!!)
	, @inyRebirthCoin tinyint --// ?? ?? ?? ??
	, @inyPCRoomRebirthCoin tinyint --// ?? ?? PC? ?? ??
	, @vchItemSerials varchar(8000) --// ,? ?????? ??? ?? ?? ???
	, @vchItemDurabilities varchar(8000) --// ,? ?????? ??? ???
	, @intDeathCount smallint = 0 --// ??? ?? ??
	, @inyDifficultyCode tinyint = 1 --// 1=??, 2=??, 3=???, 4=???, 5=???
	, @inbGainCoin bigint = NULL --// ??? ??
	, @inbPetItemSerial bigint = NULL --// ??? ?? ??? ???
	, @intGainPetExp int = NULL --// ??? ?? ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intChangedCharacterExp int
	, @inbChangedCoin bigint
	, @inyRebirthCoinBefore tinyint
	, @inyPCRoomRebirthCoinBefore tinyint
	, @insChangedRebirthCoin smallint
	, @insChangedPCRoomRebirthCoin smallint
	, @inyCharacterLevel tinyint
	, @inyJobCode tinyint
	, @intLogTableSuffix int
	, @inbCoinAfter bigint;
	
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intPartyID = CASE @intPartyID WHEN 0 THEN NULL ELSE @intPartyID END;

	BEGIN TRANSACTION;

	/**_# [CharacterStatus]???? UPDATE???.*/
	IF @inbGainCoin IS NOT NULL
	BEGIN
		UPDATE dbo.CharacterStatus
		SET @intChangedCharacterExp = CASE WHEN CharacterExp > @intCharacterExp THEN CharacterExp ELSE @intCharacterExp END - CharacterExp
			, @inyRebirthCoinBefore = RebirthCoin
			, @inyPCRoomRebirthCoinBefore = PCRoomRebirthCoin
			, @inbChangedCoin = @inbGainCoin
			, @inyCharacterLevel = CharacterLevel
			, @inyJobCode = JobCode
			, CharacterExp = CASE WHEN CharacterExp > @intCharacterExp THEN CharacterExp ELSE @intCharacterExp END
			, @inbCoinAfter = Coin += @inbGainCoin
			, RebirthCoin = @inyRebirthCoin
			, PCRoomRebirthCoin = @inyPCRoomRebirthCoin
		WHERE CharacterID = @inbCharacterID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103102;
			GOTO ErrorHandler;
		END

		IF @inbCoinAfter <> @inbCoin
		BEGIN
			SET @intReturnValue = 103300;
			GOTO ErrorHandler;
		END
	END

	ELSE
	BEGIN
		UPDATE dbo.CharacterStatus
		SET @intChangedCharacterExp = CASE WHEN CharacterExp > @intCharacterExp THEN CharacterExp ELSE @intCharacterExp END - CharacterExp
			, @inyRebirthCoinBefore = RebirthCoin
			, @inyPCRoomRebirthCoinBefore = PCRoomRebirthCoin
			, @inbChangedCoin = CASE WHEN Coin > @inbCoin THEN Coin ELSE @inbCoin END - Coin
			, @inyCharacterLevel = CharacterLevel
			, @inyJobCode = JobCode
			, CharacterExp = CASE WHEN CharacterExp > @intCharacterExp THEN CharacterExp ELSE @intCharacterExp END
			, Coin = CASE WHEN Coin > @inbCoin THEN Coin ELSE @inbCoin END
			, RebirthCoin = @inyRebirthCoin
			, PCRoomRebirthCoin = @inyPCRoomRebirthCoin
		WHERE CharacterID = @inbCharacterID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103102;
			GOTO ErrorHandler;
		END
	END

	COMMIT TRANSACTION;
	/**_# ?? ??? ?????.*/
	IF @inbChangedCoin <> 0
		EXEC @intReturnValue = dbo.P_AddCoinLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inyCoinTypeCode = 1
			, @inyCoinChangeCode = 2
			, @inbCurrentCoin = @inbCoin
			, @inbChangedCoin = @inbChangedCoin
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @inbCoinChangeFKey = @intPartyID;

	/**_# ??? ?? ?? ?? ??? ?????.*/
	SET @insChangedRebirthCoin = CAST(@inyRebirthCoin AS smallint) - @inyRebirthCoinBefore;

	IF @insChangedRebirthCoin < 0
		EXEC @intReturnValue = dbo.P_AddCoinLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inyCoinTypeCode = 3
			, @inyCoinChangeCode = 5
			, @inbCurrentCoin = @inyRebirthCoin
			, @inbChangedCoin = @insChangedRebirthCoin
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @inbCoinChangeFKey = @intPartyID;

	/**_# ??? PC? ?? ?? ??? ?????.*/
	SET @insChangedPCRoomRebirthCoin = CAST(@inyPCRoomRebirthCoin AS smallint) - @inyPCRoomRebirthCoinBefore;

	IF @insChangedPCRoomRebirthCoin < 0
		EXEC @intReturnValue = dbo.P_AddCoinLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inyCoinTypeCode = 4
			, @inyCoinChangeCode = 5
			, @inbCurrentCoin = @inyPCRoomRebirthCoin
			, @inbChangedCoin = @insChangedPCRoomRebirthCoin
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @inbCoinChangeFKey = @intPartyID;

	/**_# ??? ???? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_ModItemDurability
		  @inbCharacterID = @inbCharacterID
		, @vchItemSerials = @vchItemSerials
		, @vchItemDurabilities = @vchItemDurabilities
		, @inbCoin = 0
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID;

	/**_# ?? ??? ?? ?? ???? ?????.*/
	IF @inbPetItemSerial > 0 AND @intGainPetExp > 0
		UPDATE dbo.PetEquipments
		SET PetExp = ISNULL(PetExp, 0) + @intGainPetExp
		WHERE PetItemSerial = @inbPetItemSerial AND PetEquipmentIndex = 255;

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
