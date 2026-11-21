USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModFriendStatusCode]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-10
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101221 = ?? ?? ??? ???? ????.
103407 = ??? ???? ??? ?????.
**/

CREATE PROCEDURE [dbo].[P_ModFriendStatusCode]
	  @inbCharacterID bigint --// ??? ID
	, @inbFriendCharacterID bigint --// ?? ??? ID	
	, @inyFriendStatusCode tinyint --// ?? ?? ?? (0=??? ?? ??, 1=?? ??? ??, 2=???? ??? ??, 3=????? ??)	
	, @intFriendAccountID int OUTPUT --// ?? ???? ??? ?? ID
	, @bitBlockedCharacterFlag bit OUTPUT --// 0=??? ??? ?? ??? ?? ???, 1=??? ??? ?? ??? ???	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inyMyStatusCode tinyint
	, @bitBlockedByFriend bit = CAST(0 AS bit);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;
	
	/**_# ? ???? ??? ????? ???? ?? ??? ??? 103407? ?????.*/
	SELECT @bitBlockedByFriend = CASE WHEN COUNT(*) > 0 THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END
	FROM dbo.BlockedCharacters WITH (NOLOCK)
	WHERE CharacterID = @inbFriendCharacterID AND BlockedCharacterID = @inbCharacterID;

	IF @bitBlockedByFriend = CAST(1 AS bit) AND @inyFriendStatusCode = 1
	BEGIN
		SET @intReturnValue = 103407;
		GOTO ErrorHandler;		
	END

	/**_# [Friends] ???? UPDATE???.*/
	UPDATE dbo.Friends
	SET @inyMyStatusCode = FriendStatusCode, FriendStatusCode = @inyFriendStatusCode
	WHERE CharacterID = @inbCharacterID AND FriendCharacterID = @inbFriendCharacterID;
		
	IF @@ROWCOUNT = 0 AND @inyFriendStatusCode = 1
		INSERT dbo.Friends (CharacterID, FriendCharacterID, FriendMemo, FriendGroupID, RegisterDate, FriendStatusCode)
		SELECT @inbCharacterID, @inbFriendCharacterID, N'', NULL, GETDATE(), 1;

	SELECT @intFriendAccountID = CH.AccountID, @bitBlockedCharacterFlag = CASE WHEN BC.BlockedCharacterID IS NULL THEN CAST(0 AS bit) ELSE CAST (1 AS bit) END
	FROM dbo.Characters CH WITH (NOLOCK) 
		 LEFT OUTER JOIN dbo.BlockedCharacters BC WITH (NOLOCK) ON CH.CharacterID = BC.BlockedCharacterID AND BC.CharacterID = @inbCharacterID
	WHERE CH.CharacterID = @inbFriendCharacterID;	
	/**_## ?? ??? ?? ?? ??? ???? ?? ??? ????*/
	IF @inyFriendStatusCode = 1  
	BEGIN
		IF NOT EXISTS (SELECT * FROM dbo.Friends WITH (NOLOCK) WHERE CharacterID = @inbFriendCharacterID AND FriendCharacterID = @inbCharacterID) AND @inyMyStatusCode <> 2
		BEGIN
			SET @intReturnValue = 101221;
			GOTO ErrorHandler;
		END
		ELSE
			UPDATE dbo.Friends
			SET FriendStatusCode = 1
			WHERE CharacterID = @inbFriendCharacterID AND FriendCharacterID = @inbCharacterID;	
		
	END	

	COMMIT TRANSACTION;
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
