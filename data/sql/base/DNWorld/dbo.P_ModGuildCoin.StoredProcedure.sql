USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildCoin]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-08
description : ?? ??? ????? ?/?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103123 = ??? ?????.
103238 = ??? ???? ????.
103277 = ?? ?? ????? 0?? ??? ??? ???.
103285 = ?? ??? ??? ? ?? ?? ?? ??? ?????.
history :
3 = ?? ?? ??? ?? ??? ???? ?? ?? ?? at 2010-10-20
4 = output param. @sdtLastGuildDepotUseDate ?? at 2010-10-20
5 = 21? ?? ?? ?? ?? at 2010-11-17
7 = ??? ?? ?? at 2011-03-18
8 = ??? ??? ?? ?? at 2011-03-29
9 = ?? ?? ?? ?? ?? at 2011-05-03
10 = ?? ?? ?? ??? ????? ?? ?? by ??? at 2012-04-20
**/

CREATE PROCEDURE [dbo].[P_ModGuildCoin]
	  @intGuildID int --// ?? ID
	, @inbCharacterID bigint --// ??? ID
	, @bitInOutFlag bit --// 0=??, 1=??
	, @intGuildCoin int --// ?? ??
	, @intMapID int --// ? ID
	, @vchIP varchar(15) --// IP??
	, @inbGuildCoinAfter bigint = 0 OUTPUT --// ?/?? ? ?? ?? ?? ?? ??
	, @sdtLastGuildDepotUseDate smalldatetime = NULL OUTPUT --// ??? ?? ?? (??? ?? ?? ???? ????.)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbCoinBefore bigint
	, @inbCurrentCoin bigint
	, @dt2Now datetime2(0) = GETDATE()
	, @inyCoinChangeCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# [CharacterStatus] ???? UPDATE???.*/
	/**_# ?? ????? ???? 103123? ?????.*/
	UPDATE dbo.CharacterStatus
	SET @inbCoinBefore = Coin
		, @inbCurrentCoin = Coin += (@intGuildCoin * CASE @bitInOutFlag WHEN 0 THEN -1 ELSE 1 END)
	WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END

	/**_# [Guilds] ???? UPDATE???.*/
	/**_## ?? ????? ???? 103277? ?????.*/
	UPDATE dbo.Guilds
	SET @inbGuildCoinAfter = GuildCoin += (@intGuildCoin * CASE @bitInOutFlag WHEN 0 THEN 1 ELSE -1 END)
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

	/**_# ???? ?? ??? ??? ?????.*/
	IF @bitInOutFlag = 1
		UPDATE dbo.GuildMembers
		SET DailyTakeCoinAmount = 
			CASE
				WHEN LastGuildDepotUseDate < DATEADD(HOUR, 4, CAST(CAST(CASE WHEN DATEPART(HOUR, @dt2Now) < 4 THEN DATEADD(DAY, -1, @dt2Now) ELSE @dt2Now END AS date) AS smalldatetime)) THEN @intGuildCoin
				ELSE CASE WHEN CAST(DailyTakeCoinAmount AS bigint) + CAST(@intGuildCoin AS bigint) > 2100000000 THEN 2100000000 ELSE DailyTakeCoinAmount + @intGuildCoin END
			END
			, @sdtLastGuildDepotUseDate = LastGuildDepotUseDate = @dt2Now
			, DailyTakeItemCount = 
			CASE
				WHEN LastGuildDepotUseDate < DATEADD(HOUR, 4, CAST(CAST(CASE WHEN DATEPART(HOUR, @dt2Now) < 4 THEN DATEADD(DAY, -1, @dt2Now) ELSE @dt2Now END AS date) AS smalldatetime)) THEN 0
				ELSE DailyTakeItemCount
			END
		WHERE GuildID = @intGuildID AND CharacterID = @inbCharacterID;

	SET @inyCoinChangeCode = CASE @bitInOutFlag WHEN 0 THEN 31 ELSE 32 END;

	/**_# ?? ??? ?????.*/
	EXEC dbo.P_AddCoinLog
		  @dt2LogDate = @dt2Now
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = @inyCoinChangeCode
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @intGuildCoin
		, @intChannelID = NULL
		, @intMapID = @intMapID
		, @inbCoinChangeFKey = @intGuildID
		, @vchIP = @vchIP;

	/**_# ?? ?? ??? ?????.*/
	INSERT dbo.GuildDepotHistory (GuildID, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, GuildCoin, GuildCoinAfter, RegisterDate)
	SELECT @intGuildID, @inbCharacterID, CharacterName, 1, @bitInOutFlag, @intGuildCoin, @inbGuildCoinAfter, @dt2Now
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_CharacterStatus_Coin%'
		SET @intReturnValue = 103123;

	ELSE IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_Guilds_GuildCoin%'
		SET @intReturnValue = 103277;

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
