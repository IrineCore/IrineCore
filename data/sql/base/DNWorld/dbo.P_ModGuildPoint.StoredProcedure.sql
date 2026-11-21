USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildPoint]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-20
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103236 = ? ?? ???? 0?? ??? ??? ???.
103237 = ?? ?? ???? 0?? ??? ??? ???.
103238 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModGuildPoint]
	  @intGuildID int --// ?? ID
	, @intGuildPoint int --// ?? ??? ??? (+/- ?? ??)
	, @inbCharacterID bigint --// ?? ???? ??? ??, ??? ???? ??? ID
	, @inyCharacterLevel tinyint --// ?? ???? ??? ??, ??? ???? ??
	, @intTotalGuildPoint int OUTPUT --// ?? ? ??? ? ?? ???
	, @intRemainGuildPoint int OUTPUT --// ?? ? ??? ?? ?? ???
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
	/**_# [Guilds] ???? UPDATE???.*/
	UPDATE dbo.Guilds
	SET @intTotalGuildPoint = TotalGuildPoint += CASE WHEN @intGuildPoint > 0 THEN @intGuildPoint ELSE 0 END
		, @intRemainGuildPoint = RemainGuildPoint += @intGuildPoint
	WHERE GuildID = @intGuildID;

	/**_# ??? ???? ??? 103238? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103238;
		GOTO ErrorHandler;
	END

	IF @inbCharacterID > 0
		UPDATE dbo.GuildMembers
		SET TotalGuildPoint += CASE WHEN @intGuildPoint > 0 THEN @intGuildPoint ELSE 0 END
			, UsedGuildPoint -= CASE WHEN @intGuildPoint < 0 THEN @intGuildPoint ELSE 0 END
		WHERE GuildID = @intGuildID AND CharacterID = @inbCharacterID;

	/**_# ?? ??? ?????.*/
	IF @intGuildPoint < 0
		INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate, UsedGuildPoint)
		VALUES (@intGuildID, CASE WHEN @inbCharacterID > 0 THEN 254 ELSE 255 END, @inbCharacterID, @inyCharacterLevel, GETDATE(), - @intGuildPoint);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_Guilds_TotalGuildPoint%'
		SET @intReturnValue = 103236;

	ELSE IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_Guilds_RemainGuildPoint%'
		SET @intReturnValue = 103237;

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
