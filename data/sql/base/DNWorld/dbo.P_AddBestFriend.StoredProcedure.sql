USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddBestFriend]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-16
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103333 = ?? ???? ???? ?? ??????.
**/

CREATE PROCEDURE [dbo].[P_AddBestFriend]
	  @inbCharacterID bigint --// ???ID
	, @inbBestFriendCharacterID bigint --// ?? ???ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intCharacterCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ???? ????? ?????.*/
	SELECT @intCharacterCount = COUNT(*) 
	FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
	WHERE CharacterID IN (@inbCharacterID, @inbBestFriendCharacterID)
	  AND DeleteFlag = 0;
	
	IF @intCharacterCount <> 2
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler
	END

	/**_# [BestFriends] ???? INSERT???.*/
	BEGIN TRANSACTION

	INSERT dbo.BestFriends (CharacterID, BestFriendCharacterID, BestFreindStatusFlag)
	VALUES (@inbCharacterID, @inbBestFriendCharacterID, 0);

	INSERT dbo.BestFriends (CharacterID, BestFriendCharacterID, BestFreindStatusFlag)
	VALUES (@inbBestFriendCharacterID, @inbCharacterID, 0);

	COMMIT TRANSACTION
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_BestFriends%'
		SET @intReturnValue = 103333;

	ELSE IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNI_BestFriends_BestFriendCharacterID%'
		SET @intReturnValue = 103333;

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
