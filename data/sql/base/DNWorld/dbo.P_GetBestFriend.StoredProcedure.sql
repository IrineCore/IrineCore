USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetBestFriend]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-16
description : ?? ???? ?? ??? ?????.
result set : BestFriendCharacterID, FriendshipStartDate, MyMemo, BestFreindStatusFlag, FriendshipEndFlag, FriendshipEndDate, BestFriendMemo, ItemSerial
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
7 = ?? ItemSerial ?? by ??? at 2012-05-03
**/

CREATE PROCEDURE [dbo].[P_GetBestFriend]
	  @inbCharacterID bigint --// ???ID
	, @inbBestFriendCharacterName nvarchar(30) OUTPUT --// ?? ??? ??
	, @inyBestFriendCharacterLevel tinyint OUTPUT --// ?? ??? ??
	, @inyBestFriendCharacterJobCode tinyint OUTPUT --// ?? ??? ??Code
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @inbBestFriendCharacterID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inbBestFriendCharacterID = BestFriendCharacterID 
	FROM dbo.BestFriends WITH(NOLOCK, INDEX(PK_BestFriends), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	
	/**_# ?? ?? ??? ??? ?????.*/
	SELECT @inbBestFriendCharacterName = C.CharacterName
         , @inyBestFriendCharacterLevel = CS.CharacterLevel
         , @inyBestFriendCharacterJobCode = CS.JobCode 
	FROM dbo.Characters C WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK)
		INNER JOIN dbo.CharacterStatus CS WITH(NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON C.CharacterID = CS.CharacterID
		INNER JOIN dbo.BestFriends BF WITH(NOLOCK, INDEX(IX_UNI_BestFriends_BestFriendCharacterID)) ON C.CharacterID = BF.BestFriendCharacterID
	WHERE C.CharacterID = @inbBestFriendCharacterID
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	/**_# ?? ??? ?????.*/
	SELECT BF.BestFriendCharacterID, BF.FriendshipStartDate, BF.MyMemo, BF.BestFreindStatusFlag, BF.FriendshipEndFlag, BF.FriendshipEndDate, BF2.MyMemo AS BestFriendMemo, BF.ItemSerial
	FROM dbo.BestFriends BF WITH(NOLOCK, INDEX(PK_BestFriends), FORCESEEK)
		INNER JOIN dbo.BestFriends BF2 WITH(NOLOCK, INDEX(IX_UNI_BestFriends_BestFriendCharacterID), FORCESEEK) ON BF.CharacterID = BF2.BestFriendCharacterID
	WHERE BF.CharacterID = @inbCharacterID
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

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
