USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelBestFriend]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-16
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103335 = ??? ??? ? ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_DelBestFriend]
	  @inbCharacterID bigint --// ???ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2NowDate datetime2(0) = GETDATE()
	  , @inbBestFriendCharacterID bigint; --// ?? ???ID

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
	OPTION(KEEPFIXED PLAN);
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103335;
		GOTO ErrorHandler;
	END

	BEGIN TRAN
	/**_# [BestFriends]???? DELETE???.*/
	DELETE dbo.BestFriends 
	WHERE CharacterID IN (@inbCharacterID, @inbBestFriendCharacterID);
	/**_# ?? ?? ?????? DELETE???.*/
	DELETE dbo.EffectItems
	WHERE CharacterID IN (@inbCharacterID, @inbBestFriendCharacterID) AND EffectItemGetCode = 8

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103335;
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
