USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_CompleteEventQuest]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-13
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_CompleteEventQuest]
	  @inbCharacterID bigint --// ??? ID
	, @intEventQuestID int --// ??? ??? ID
	, @sdtEventQuestExpireDate smalldatetime --// ??? ??? ?? ??
	, @insCompleteCount smallint OUTPUT --// ?? ??
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
	UPDATE dbo.CompleteEventQuest
	SET @insCompleteCount = CompleteCount += 1
	WHERE CharacterID = @inbCharacterID AND EventQuestID = @intEventQuestID;

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.CompleteEventQuest (CharacterID, EventQuestID, EventQuestExpireDate, CompleteCount)
		VALUES (@inbCharacterID, @intEventQuestID, @sdtEventQuestExpireDate, 1);

		SET @insCompleteCount = 1;
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
