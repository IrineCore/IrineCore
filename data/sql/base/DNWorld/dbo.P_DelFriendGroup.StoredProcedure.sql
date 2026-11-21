USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelFriendGroup]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-18
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103130 = ?? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_DelFriendGroup]
	  @inbCharacterID bigint --// ??? ID
	, @intFriendGroupID int --// ?? ?? ID
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
	/**_# ??? ?? ??? ?? ??? ??? ??? ??? UPDATE???.*/
	UPDATE dbo.Friends SET FriendGroupID = NULL WHERE FriendGroupID = @intFriendGroupID AND CharacterID = @inbCharacterID;

	/**_# [FriendGroups] ???? DELETE???.*/
	DELETE dbo.FriendGroups WHERE FriendGroupID = @intFriendGroupID AND CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103130;
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
