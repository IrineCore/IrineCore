USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModFriendMemo]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-18
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103133 = ?? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModFriendMemo]
	  @inbCharacterID bigint --// ??? ID
	, @inbFriendCharacterID bigint --// ?? ??? ID
	, @nvcFriendMemo nvarchar(300) --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inyAccountLevelCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ??? ?????.*/
	UPDATE dbo.Friends
	SET FriendMemo = @nvcFriendMemo
	WHERE CharacterID = @inbCharacterID AND FriendCharacterID = @inbFriendCharacterID;

	/**_# ?? ??? ???? ?? ????? 103133? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103133;
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
