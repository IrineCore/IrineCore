USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModFriendRequestFlag]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-07
description : ?? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ModFriendRequestFlag]
	  @inbCharacterID bigint --// ???ID
	, @bitFriendRequestFlag bit --// 0=??????, 1=????????
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

	/**_# [FriendRequestFlags] ???? UPSERT???.*/
	UPDATE dbo.FriendRequestFlags
	SET FriendRequestFlag = @bitFriendRequestFlag
	WHERE CharacterID = @inbCharacterID;
	
	IF @@ROWCOUNT = 0
		INSERT dbo.FriendRequestFlags (CharacterID, FriendRequestFlag)
		SELECT @inbCharacterID, @bitFriendRequestFlag;		
		
END TRY
BEGIN CATCH	

	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
