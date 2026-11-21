USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModWorldEventQuestCounter]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-25
description : ?? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ModWorldEventQuestCounter]
	  @intEventQuestID int --// Event Quest ID
	, @intCounter1Delta int --// ???? ??? ? (???)
	, @intCouter1After int OUTPUT --// ?? ? ??? ?
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
	/**_# [WorldEventQuestCounter] ???? UPSERT???.*/
	UPDATE dbo.WorldEventQuestCounter
	SET @intCouter1After = Counter1 += @intCounter1Delta
	WHERE EventQuestID = @intEventQuestID;

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.WorldEventQuestCounter (EventQuestID, Counter1)
		VALUES (@intEventQuestID, @intCounter1Delta);
		SET @intCouter1After = @intCounter1Delta;
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
