USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_EndBestFriend]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-16
description : ??? ??/???? ???.(??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103336 = ?? ?? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_EndBestFriend]
	  @inbCharacterID bigint --// ???ID
	, @bitBestFreindStatusFlag bit --// ?? ?? 0=??,1=??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2NowDate datetime2(0) = GETDATE()
      , @inbBestFriendCharacterID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [BestFriends] ???? ?? ??? UPDATE???.*/
	BEGIN TRAN

	UPDATE dbo.BestFriends
	SET BestFreindStatusFlag = @bitBestFreindStatusFlag
      , FriendshipEndDate = CASE WHEN @bitBestFreindStatusFlag = 1 THEN @dt2NowDate ELSE NULL END
	  , FriendshipEndFlag = CASE WHEN @bitBestFreindStatusFlag = 1 THEN 1 ELSE 0 END
      , @inbBestFriendCharacterID = BestFriendCharacterID
	WHERE CharacterID = @inbCharacterID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103336;
		GOTO ErrorHandler;
	END
	
	UPDATE dbo.BestFriends
	SET BestFreindStatusFlag = @bitBestFreindStatusFlag
      , FriendshipEndDate = CASE WHEN @bitBestFreindStatusFlag = 1 THEN @dt2NowDate ELSE NULL END
	  , FriendshipEndFlag = 0
	WHERE CharacterID = @inbBestFriendCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103336;
		GOTO ErrorHandler;
	END

	COMMIT TRAN
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
