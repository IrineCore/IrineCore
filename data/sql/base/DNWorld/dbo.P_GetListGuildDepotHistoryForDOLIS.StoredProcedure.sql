USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildDepotHistoryForDOLIS]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-11-12
description : ?? ?? ?? ???  ?????.
result set : 
		RegisterDate/datetime2, --// ?? ?? ?? ??
		ItemCoinFlag/bit, --// 0=???, 1=????		
		InOutFlag/bit, --// 0=?, 1=?		
		GuildName/nvarchar(50), --// ????
		CharacterID/bigint, --// ??? ID
		CharacterName/nvarchar(30), --// ??? ??
		ItemID/int, --// ??? ID
		ItemLevel/tinyint, --// ??? ?? ??
		ItemCount/smallint, --// ??? ?? ??
		GuildCoin/int, --// ?? ?? ?? ?? ?? ?
		GuildCoinAfter/bigint--// ?? ? ?? ?? ?? ??
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103353 = ????? ??ID? ??? ??? ???? ???.
history :
5 = ?? ?? ? ??? ???? ??? ????? ?? by ??? at 20
**/

CREATE PROCEDURE [dbo].[P_GetListGuildDepotHistoryForDOLIS]
	  @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ? ??
	, @intGuildID int = NULL --// ?? ID
	, @nvcGuildName nvarchar(50) = NULL --// ?? ??
	, @vchItemCoinFlag varchar(max) --// ???? ?? (0=????, 1=?????, ','? ???? ??? ex)'0,1')
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @nvcGuildName IS NULL AND @intGuildID IS NOT NULL
		SELECT @nvcGuildName = GuildName 
		FROM dbo.Guilds WITH(NOLOCK)
		WHERE GuildID = @intGuildID
	ELSE IF @intGuildID IS NULL AND @nvcGuildName IS NOT NULL	
		SELECT @intGuildID = GuildID 
		FROM dbo.Guilds WITH(NOLOCK)
		WHERE GuildName = @nvcGuildName
		  AND CloseFlag = 0;

	/**_# [GuildDepotHistory] ???? SELECT???.*/
	WITH CTE AS(
		SELECT RegisterDate, ItemCoinFlag, InOutFlag, @nvcGuildName AS GuildName, CharacterID, CharacterName, ItemID, ItemLevel, ItemCount, GuildCoin, GuildCoinAfter
		FROM dbo.GuildDepotHistory WITH (NOLOCK, INDEX(PK_GuildDepotHistory))
		WHERE RegisterDate >= @dt2StartDate AND RegisterDate <= @dt2EndDate
		  AND GuildID = @intGuildID
		  AND ItemCoinFlag IN (SELECT CAST(string AS bit) FROM dbo.TF_Split(@vchItemCoinFlag,','))
		UNION ALL
		SELECT RegisterDate, ItemCoinFlag, InOutFlag, @nvcGuildName, CharacterID, CharacterName, ItemID, ItemLevel, ItemCount, GuildCoin, GuildCoinAfter
		FROM dbo.GuildDepotHistoryBak WITH (NOLOCK, INDEX(IX_NC_GuildDepotHistoryBak_GuildID_GuildDepotHistoryID))
		WHERE RegisterDate >= @dt2StartDate AND RegisterDate <= @dt2EndDate
		  AND GuildID = @intGuildID
		  AND ItemCoinFlag IN (SELECT CAST(string AS bit) FROM dbo.TF_Split(@vchItemCoinFlag,','))
	)
	SELECT RegisterDate, ItemCoinFlag, InOutFlag, @nvcGuildName AS GuildName, CharacterID, CharacterName, ItemID, ItemLevel, ItemCount, GuildCoin, GuildCoinAfter
	FROM CTE
	ORDER BY RegisterDate
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103353;
		GOTO ErrorHandler;
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
