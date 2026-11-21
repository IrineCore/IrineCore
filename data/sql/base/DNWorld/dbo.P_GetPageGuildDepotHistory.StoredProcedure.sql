USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGuildDepotHistory]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-08
description : ?? ?? ????? ??????.
result set : CharacterID/bigint, CharacterName/nvarchar, ItemCoinFlag/bit, InOutFlag/bit, ItemID/int, ItemLevel/tinyint, ItemCount/smallint, GuildCoin/int, GuildCoinAfter/bigint, RegisterDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103278 = ?? ?? ????? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPageGuildDepotHistory]
	  @intStartNumber int --// ?? ??.
	, @intEndNumber int --// ? ??.
	, @inyDirection tinyint --// 1=????, 2=????
	, @intGuildID int --// ?? ID
	, @bitItemCoinFlag bit --// 0=???, 1=????. NULL? ???? ?? ?????.
	, @bitInOutFlag bit --// 0=?, 1=?. NULL? ???? ?? ?????.
	, @intRecordCount int = 0 OUTPUT --// ??? ?. ?, @intStartNumber ?? 1? ?? ?????.
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
	IF @intStartNumber = 1
	BEGIN
		SELECT @intRecordCount = COUNT(*)
		FROM dbo.GuildDepotHistory WITH (NOLOCK)
		WHERE GuildID = @intGuildID AND ItemCoinFlag = COALESCE(@bitItemCoinFlag, ItemCoinFlag) AND InOutFlag = COALESCE(@bitInOutFlag, InOutFlag);
	END

	IF @inyDirection = 1
		WITH C1 AS (
			SELECT ROW_NUMBER() OVER(ORDER BY GuildDepotHistoryID DESC) AS RowNumber
				, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, GuildCoinAfter, RegisterDate
			FROM dbo.GuildDepotHistory WITH (NOLOCK)
			WHERE GuildID = @intGuildID AND ItemCoinFlag = COALESCE(@bitItemCoinFlag, ItemCoinFlag) AND InOutFlag = COALESCE(@bitInOutFlag, InOutFlag)
		)
		SELECT CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, GuildCoinAfter, RegisterDate
		FROM C1
		WHERE RowNumber BETWEEN @intStartNumber AND @intEndNumber
		ORDER BY RowNumber;

	ELSE
		WITH C1 AS (
			SELECT ROW_NUMBER() OVER(ORDER BY GuildDepotHistoryID) AS RowNumber
				, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, GuildCoinAfter, RegisterDate
			FROM dbo.GuildDepotHistory WITH (NOLOCK)
			WHERE GuildID = @intGuildID AND ItemCoinFlag = COALESCE(@bitItemCoinFlag, ItemCoinFlag) AND InOutFlag = COALESCE(@bitInOutFlag, InOutFlag)
		)
		SELECT CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, GuildCoinAfter, RegisterDate
		FROM C1
		WHERE RowNumber BETWEEN @intStartNumber AND @intEndNumber
		ORDER BY RowNumber;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103278;
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
