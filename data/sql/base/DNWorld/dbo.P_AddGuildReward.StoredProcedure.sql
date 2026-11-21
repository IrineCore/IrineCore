USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildReward]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-30
description : ?? ??? ?????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103324 = ?? ?? ??? ?? ?? ??????.
history
6~7 = ???? OUTPUT ???? ?? by ??? at 2011-12-06
**/
CREATE PROCEDURE [dbo].[P_AddGuildReward]
	  @intGuildID int --// ?? ID
	, @intGuildItemID int --// ?? ?? ???IDsp
	, @bitEternityFlag bit --// 0=???? ?? ??, 1=?? ???
	, @intItemLifespan int --// ??? ???? ??, ??? ?? (?)
    , @inyItemBuyMethod tinyint--// 0=GuildItem? ?? ?????. ,1=??? GuildItem? ?????. ,2=??? GuildItem? ??? ?????. 
	, @inbCharacterID bigint --// ?? ?? ??? ??? ?? ??? ??? ??? ID
	, @intCoin int --// ?? ?? ?? ?? - ?? ??
	, @intMapID int --// ? ID
	, @intDeletedGuildItemID int = NULL --// ?? ??? ??? ??? ?? ????? GuildItemID (@inyItemBuyMethod = 2? ???? ??)
	, @dt2ItemExpireDate datetime2(0) OUTPUT --//??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyItemBuyMethod = 0
	BEGIN
		IF @bitEternityFlag = 1
			SET @dt2ItemExpireDate = DATEADD(YEAR, 100, @dt2Now);
		ELSE
			SET @dt2ItemExpireDate = DATEADD(MINUTE, @intItemLifespan, @dt2Now);

		INSERT dbo.GuildRewards (GuildID, GuildItemID, EternityFlag, RegisterDate, ItemExpireDate)
		VALUES (@intGuildID, @intGuildItemID, @bitEternityFlag, @dt2Now, @dt2ItemExpireDate);	
	END
	ELSE IF @inyItemBuyMethod = 1
	BEGIN
		UPDATE dbo.GuildRewards
		SET @dt2ItemExpireDate = ItemExpireDate = DATEADD(MINUTE, @intItemLifespan, ItemExpireDate) 						
		WHERE GuildID = @intGuildID AND GuildItemID = @intGuildItemID AND ItemExpireDate >= @dt2Now;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @dt2ItemExpireDate = DATEADD(MINUTE, @intItemLifespan, @dt2Now);
	
			INSERT dbo.GuildRewards (GuildID, GuildItemID, EternityFlag, RegisterDate, ItemExpireDate)
			VALUES (@intGuildID, @intGuildItemID, @bitEternityFlag, @dt2Now, @dt2ItemExpireDate);
		END
	END
	ELSE IF @inyItemBuyMethod = 2
	BEGIN
		DELETE dbo.GuildRewards
		WHERE GuildID = @intGuildID AND GuildItemID = @intDeletedGuildItemID;

		SET @dt2ItemExpireDate = DATEADD(MINUTE, @intItemLifespan, @dt2Now);

		INSERT dbo.GuildRewards (GuildID, GuildItemID, EternityFlag, RegisterDate, ItemExpireDate)
		VALUES (@intGuildID, @intGuildItemID, @bitEternityFlag, @dt2Now, @dt2ItemExpireDate);
	END

	/**_# ??? ?????.*/
	IF @intCoin > 0
	BEGIN
		SET @intCoin = ABS(@intCoin);

		EXEC dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterID
			, @inyCoinChangeCode = 33
			, @inbCoinChangeFKey = @intGuildItemID
			, @inbCoin = @intCoin
			, @intChannelID = 0
			, @intMapID = 1
			, @dt2LogDate = @dt2Now;
	END

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_GuildRewards%'
		SET @intReturnValue = 103324;

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
