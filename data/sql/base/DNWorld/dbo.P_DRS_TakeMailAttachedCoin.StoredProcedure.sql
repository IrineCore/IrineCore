USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_TakeMailAttachedCoin]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-18
description : ??? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000052 = ????? ????.
1030000053 = ??? ???? ????.
1030000054 = ?? ??? ??? ? ?? ?? ?? ??? ?????.
1030000055 = ??? ??ID? ????.
1030000056 = ??? ???? ????.
1030000058 = 42? ??? ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_TakeMailAttachedCoin]
	  @intMailID int --// ?? ID	
	, @intGuildID int = NULL --// ??ID (MailTypeCode? 19?? ???? ?????.)
	, @intCharacterExp int = NULL OUTPUT -- // ??? ???
	, @inbCoin bigint = NULL OUTPUT -- // ??? ??
	, @inbWarehouseCoin bigint = NULL OUTPUT -- // ??? ????
	, @inyCharacterLevel tinyint = NULL OUTPUT -- // ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inbReceiverCharacterID bigint
	, @inbCurrentCoin bigint
	, @inyMailTypeCode tinyint
	, @inbGuildCoinAfter bigint
	, @inbAttachCoin bigint;

DECLARE @inbTmp1 bigint, @inbTmp2 bigint;

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
			SET @intReturnValue = 1030000055;
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
			SET @intReturnValue = 1030000052;
			GOTO ErrorHandler;
		END

		UPDATE dbo.Guilds
		SET @inbGuildCoinAfter = GuildCoin += @inbAttachCoin
		WHERE GuildID = @intGuildID;

		/**_# ??? ???? ??? 1030000053? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 1030000053;
			GOTO ErrorHandler;
		END

		/**_# ?? ??? ??? ? ?? ?? ?? ??? ???? 1030000054? ?????.*/
		IF @inbGuildCoinAfter > 2100000000
		BEGIN
			SET @intReturnValue = 1030000054;
			GOTO ErrorHandler;
		END

		COMMIT TRANSACTION;

		/**_# ?? ?? ??? ?????.*/
		INSERT dbo.GuildDepotHistory (GuildID, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, GuildCoin, GuildCoinAfter, RegisterDate)
		SELECT @intGuildID, @inbReceiverCharacterID, CharacterName, 1, 0, @inbAttachCoin, @inbGuildCoinAfter, @dt2Now
		FROM dbo.Characters WITH (NOLOCK)
		WHERE CharacterID = @inbReceiverCharacterID;		
		
	END	
	ELSE
	BEGIN				
		SELECT @inbReceiverCharacterID = ReceiverCharacterID, @inbTmp1 = AttachCoin FROM dbo.Mails WITH (NOLOCK, FORCESEEK) WHERE MailID = @intMailID OPTION (KEEPFIXED PLAN);		
		SELECT @inbTmp2 = Coin FROM dbo.CharacterStatus WITH (NOLOCK, FORCESEEK) WHERE CharacterID = @inbReceiverCharacterID OPTION (KEEPFIXED PLAN);		

		IF (@inbTmp1 + @inbTmp2) > 4200000000
		BEGIN
			SET @intReturnValue = 1030000058;
			GOTO ErrorHandler;
		END

		SET @inbAttachCoin = @inbTmp1;

		UPDATE dbo.Mails
		SET AttachCoinReceiveFlag = 1
		WHERE MailID = @intMailID AND AttachCoinReceiveFlag = CAST(0 AS bit) AND AttachCoin > 0;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 1030000056;
			GOTO ErrorHandler;
		END

		UPDATE dbo.CharacterStatus
		SET @inbCurrentCoin = Coin += @inbAttachCoin
		  , @intCharacterExp = CharacterExp		  
		  , @inbWarehouseCoin = WarehouseCoin
		  , @inyCharacterLevel = CharacterLevel
		WHERE CharacterID = @inbReceiverCharacterID;

		SET @inbCoin = @inbCurrentCoin;
		EXEC @intReturnValue = dbo.P_AddCoinLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbReceiverCharacterID
			, @inyCoinTypeCode = 1
			, @inyCoinChangeCode = 7
			, @inbCurrentCoin = @inbCurrentCoin
			, @inbChangedCoin = @inbAttachCoin
			, @intChannelID = NULL
			, @intMapID = NULL
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
