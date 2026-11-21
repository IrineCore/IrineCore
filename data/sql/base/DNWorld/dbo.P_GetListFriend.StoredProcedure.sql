USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFriend]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 15
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-18
description : ?? ??? ?????.
result set : 
	version 1 = FriendGroupID/int, FriendGroupName/nvarchar, FriendAccountID/int, FriendCharacterID/bigint, FriendCharacterName/nvarchar, FriendMemo/nvarchar
	version 2 = FriendGroupID/int, FriendGroupName/nvarchar, FriendAccountID/int, FriendCharacterID/bigint, FriendCharacterName/nvarchar, FriendMemo/nvarchar, MobileAuthenticationFlag/bit
	version 3 = FriendGroupID/int, FriendGroupName/nvarchar, FriendAccountID/int, FriendCharacterID/bigint, FriendCharacterName/nvarchar, FriendMemo/nvarchar, MobileAuthenticationFlag/bit, FriendStatusCode/tinyint
history :
	13 = ???? ?? ??? FriendStatusCode? 4? ?? by ??? at 2013-06-20
	14 = ???? ??? ?? ??? ???? ???? ?? ?? by ??? at 2013-06-28
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103133 = ?? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListFriend]
	  @inbCharacterID bigint --// ??? ID
	, @intVersion int = 1 --// ??? ???? ??? 2?
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
	IF @intVersion = 2
	BEGIN
		/**_# ?? ??? ?????.*/
		;WITH C1 (FriendGroupID, FriendAccountID, FriendCharacterID, FriendCharacterName, FriendMemo, MobileAuthenticationFlag) AS (
			SELECT FR.FriendGroupID, CH.AccountID, CH.CharacterID, CH.CharacterName, FR.FriendMemo, ISNULL(CH.MobileAuthenticationFlag, 0)
			FROM dbo.Friends FR WITH (NOLOCK)
				INNER JOIN dbo.Characters CH WITH (NOLOCK) ON FR.FriendCharacterID = CH.CharacterID
			WHERE FR.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		), C2 AS (
			SELECT FriendGroupID, FriendGroupName
			FROM dbo.FriendGroups FG WITH (NOLOCK)
			WHERE FG.CharacterID = @inbCharacterID
		)
		SELECT C2.FriendGroupID, C2.FriendGroupName, C1.FriendAccountID, C1.FriendCharacterID, C1.FriendCharacterName, C1.FriendMemo, C1.MobileAuthenticationFlag
		FROM C1
			FULL OUTER JOIN C2 ON C1.FriendGroupID = C2.FriendGroupID               
		ORDER BY C2.FriendGroupID;

		/**_# ?? ??? ??? 103133? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103133;
			GOTO ErrorHandler;
		END
	END
	ELSE IF @intVersion = 3
	BEGIN
		/**_# ?? ??? ?????.*/
		;WITH C1 (FriendGroupID, FriendAccountID, FriendCharacterID, FriendCharacterName, FriendMemo, MobileAuthenticationFlag, FriendStatusCode) AS (
			SELECT FR.FriendGroupID, CH.AccountID, CH.CharacterID, CH.CharacterName, FR.FriendMemo, ISNULL(CH.MobileAuthenticationFlag, 0), FR.FriendStatusCode
			FROM dbo.Friends FR WITH (NOLOCK)
				INNER JOIN dbo.Characters CH WITH (NOLOCK) ON FR.FriendCharacterID = CH.CharacterID
			WHERE FR.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
			UNION ALL
			SELECT FR.FriendGroupID, CH.AccountID, CH.CharacterID, CH.CharacterName, FR.FriendMemo, ISNULL(CH.MobileAuthenticationFlag, 0), 4 AS FriendStatusCode
			FROM dbo.Friends FR WITH (NOLOCK)
				INNER JOIN dbo.Characters CH WITH (NOLOCK) ON FR.CharacterID = CH.CharacterID
			WHERE FR.FriendCharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit) AND FR.FriendStatusCode = 3
		), C2 AS (
			SELECT FriendGroupID, FriendGroupName
			FROM dbo.FriendGroups FG WITH (NOLOCK)
			WHERE FG.CharacterID = @inbCharacterID
		)
		SELECT C2.FriendGroupID, C2.FriendGroupName, C1.FriendAccountID, C1.FriendCharacterID, C1.FriendCharacterName, C1.FriendMemo, C1.MobileAuthenticationFlag, MAX(C1.FriendStatusCode) AS FriendStatusCode
		FROM C1
			FULL OUTER JOIN C2 ON C1.FriendGroupID = C2.FriendGroupID               
		GROUP BY C2.FriendGroupID, C2.FriendGroupName, C1.FriendAccountID, C1.FriendCharacterID, C1.FriendCharacterName, C1.FriendMemo, C1.MobileAuthenticationFlag
		ORDER BY C2.FriendGroupID;

		/**_# ?? ??? ??? 103133? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103133;
			GOTO ErrorHandler;
		END
	END
	ELSE
	BEGIN
		/**_# ?? ??? ?????.*/
		;WITH C1 (FriendGroupID, FriendAccountID, FriendCharacterID, FriendCharacterName, FriendMemo) AS (
			SELECT FR.FriendGroupID, CH.AccountID, CH.CharacterID, CH.CharacterName, FR.FriendMemo
			FROM dbo.Friends FR WITH (NOLOCK)
				INNER JOIN dbo.Characters CH WITH (NOLOCK) ON FR.FriendCharacterID = CH.CharacterID
			WHERE FR.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		), C2 AS (
			SELECT FriendGroupID, FriendGroupName
			FROM dbo.FriendGroups FG WITH (NOLOCK)
			WHERE FG.CharacterID = @inbCharacterID
		)
		SELECT C2.FriendGroupID, C2.FriendGroupName, C1.FriendAccountID, C1.FriendCharacterID, C1.FriendCharacterName, C1.FriendMemo
		FROM C1
			FULL OUTER JOIN C2 ON C1.FriendGroupID = C2.FriendGroupID
		ORDER BY C2.FriendGroupID;
		
		/**_# ?? ??? ??? 103133? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103133;
			GOTO ErrorHandler;
		END
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
