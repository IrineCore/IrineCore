USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModNotifier]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-13
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103108 = ???? ???? ????.
history :
2 ~ 3 = UPSERT? ?? by ??? at 2013-06-13
**/

CREATE PROCEDURE [dbo].[P_ModNotifier]
	  @inbCharacterID bigint --// ??? ID
	, @inyNotifierIndex tinyint --// ??? ???
	, @inyNotifyTargetCode tinyint --// ??? ?? ??
	, @intNotifyTargetID int --// ??? ?? ID
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
	/**_# [Notifiers]???? UPDATE???.*/
	UPDATE dbo.Notifiers
	SET NotifyTargetCode = @inyNotifyTargetCode, NotifyTargetID = @intNotifyTargetID
	WHERE CharacterID = @inbCharacterID AND NotifierIndex = @inyNotifierIndex;	

	/**_# ???? ???? ??? INSERT???.*/
	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.Notifiers (CharacterID, NotifierIndex, NotifyTargetCode, NotifyTargetID)
		SELECT @inbCharacterID, @inyNotifierIndex, @inyNotifyTargetCode, @intNotifyTargetID;
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
