USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildHistory]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-19
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddGuildHistory]
	  @intGuildID int --// ?? ID
	, @inyGuildHistoryCode tinyint --// 7=??? ?? ?, 8=??? ? ?? ??
	, @inbCharacterID bigint --// ??? ID
	, @intItemID int --// ??? ID
	, @inyItemLevel tinyint --// ??? ??
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
	/**_# [GuildHistory] ???? INSERT???.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, ItemID, ItemLevel, RegisterDate)
	SELECT @intGuildID, @inyGuildHistoryCode, @inbCharacterID, CharacterLevel, @intItemID, @inyItemLevel, GETDATE()
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
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
