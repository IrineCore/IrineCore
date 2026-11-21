USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModMyMemo]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-16
description : ?? ??? ?????..
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103336 = ?? ?? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_ModMyMemo]
	  @inbCharacterID bigint --// ???ID
	, @nvcMyMemo nvarchar(300) --// ?? ??
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
	/**_# [BestFriends] ???? UPDATE???.*/
	UPDATE dbo.BestFriends
	SET MyMemo = @nvcMyMemo
	WHERE CharacterID = @inbCharacterID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103336;
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
