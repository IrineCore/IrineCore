USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModCharacterLevel]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
10 = ??? ?? ?? at 2011-03-18
11 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_ModCharacterLevel]
	  @inbCharacterID bigint --// ??? ID
	, @inyCharacterLevelChangeCode tinyint --// 1=??, 2=??, 3=??? ??, 4=?????? ??
	, @inyCharacterLevel tinyint --// ??? ??
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @insChangedCharacterLevel smallint
	, @insCharacterLevelBefore smallint
	, @dtmNow datetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @insCharacterLevelBefore = CharacterLevel
	  , @insChangedCharacterLevel = CAST(@inyCharacterLevel AS smallint) - CharacterLevel
	  , CharacterLevel = @inyCharacterLevel
	WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END

	IF ABS(@insChangedCharacterLevel) >= 10 AND @insCharacterLevelBefore >= 10
	BEGIN
		IF (SELECT AccountLevelCode FROM dbo.Characters WITH(NOLOCK) WHERE CharacterID = @inbCharacterID) = 0
		BEGIN
			UPDATE dbo.Characters
			SET DeleteFlag = 1, DeleteDate = @dtmNow, DeleteByAdminFlag = 1
			WHERE CharacterID = @inbCharacterID;
			
			INSERT dbo.TableAuditLogs (tableName, operationCode, columnNames, beforeImages, modifier, occurredDate)
			SELECT N'P_ModCharacterLevel', 3, N'', N'', N'', @dtmNow;

			RETURN 103102;
		END
	END

	/**_# [P_AddCharacterLevelLog]? ???? ??? ?? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_AddCharacterLevelLog
		  @dt2LogDate = @dtmNow
		, @inbCharacterID = @inbCharacterID
		, @inyCharacterLevelChangeCode = @inyCharacterLevelChangeCode
		, @inyCurrentCharacterLevel = @inyCharacterLevel
		, @insChangedCharacterLevel = @insChangedCharacterLevel
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID;
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
