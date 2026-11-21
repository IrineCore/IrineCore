USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TakeMailAttachCoin]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-28
description : ?? ?? ??? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103167 = ????? ????.
103238 = ??? ???? ????.
103285 = ?? ??? ??? ? ?? ?? ?? ??? ?????.
history :
3 = ??? ?? ?? at 2011-03-18
4 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_TakeMailAttachCoin]
	  @intMailID int --// ?? ID
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @inbAttachCoin bigint OUTPUT --// ?? ??
	, @intGuildID int = NULL --// ??iD
	, @bitGuildItemMailFlag bit = NULL OUTPUT --//???? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inbReceiverCharacterID bigint
	, @inbCurrentCoin bigint
	, @inyMailTypeCode tinyint
	, @inbGuildCoinAfter bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inyMailTypeCode = MailTypeCode FROM dbo.Mails WITH (NOLOCK) WHERE MailID = @intMailID;

	IF @inyMailTypeCode = 19
	BEGIN
		IF @intGuildID IS NULL
		BEGIN
			SET @intReturnValue = 103350;
			GOTO ErrorHandler;
		END

		BEGIN TRANSACTION;

		UPDATE dbo.Mails
		SET @inbReceiverCharacterID = ReceiverCharacterID
			, @inbAttachCoin = AttachCoin
			, AttachCoinReceiveFlag = 1
		WHERE MailID = @intMailID AND AttachCoinReceiveFlag = CAST(0 AS bit) AND AttachCoin > 0;
	
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103167;
			GOTO ErrorHandler;
		END

		UPDATE dbo.Guilds
		SET @inbGuildCoinAfter = GuildCoin += @inbAttachCoin
		WHERE GuildID = @intGuildID;

		/**_# ??? ???? ??? 103238? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103238;
			GOTO ErrorHandler;
		END

		/**_# ?? ??? ??? ? ?? ?? ?? ??? ???? 103285? ?????.*/
		IF @inbGuildCoinAfter > 2100000000
		BEGIN
			SET @intReturnValue = 103285;
			GOTO ErrorHandler;
		END

		COMMIT TRANSACTION;

		/**_# ?? ?? ??? ?????.*/
		INSERT dbo.GuildDepotHistory (GuildID, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, GuildCoin, GuildCoinAfter, RegisterDate)
		SELECT @intGuildID, @inbReceiverCharacterID, CharacterName, 1, 0, @inbAttachCoin, @inbGuildCoinAfter, @dt2Now
		FROM dbo.Characters WITH (NOLOCK)
		WHERE CharacterID = @inbReceiverCharacterID;

		SET @bitGuildItemMailFlag = 1;
		
	END	
	ELSE
	BEGIN
		UPDATE dbo.Mails
		SET @inbReceiverCharacterID = ReceiverCharacterID
			, @inbAttachCoin = AttachCoin
			, AttachCoinReceiveFlag = 1
		WHERE MailID = @intMailID AND AttachCoinReceiveFlag = CAST(0 AS bit) AND AttachCoin > 0;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103167;
			GOTO ErrorHandler;
		END

		UPDATE dbo.CharacterStatus
		SET @inbCurrentCoin = Coin += @inbAttachCoin
		WHERE CharacterID = @inbReceiverCharacterID;

		EXEC @intReturnValue = dbo.P_AddCoinLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbReceiverCharacterID
			, @inyCoinTypeCode = 1
			, @inyCoinChangeCode = 7
			, @inbCurrentCoin = @inbCurrentCoin
			, @inbChangedCoin = @inbAttachCoin
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @inbCoinChangeFKey = @intMailID;
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
