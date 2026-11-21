USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ClearQuest]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-02
description : ???? ??????. (???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = dbo.CompleteQuestBinaries ???? ???? ??? ?? at 2010-08-30
3 = dbo.Quests ???? ?? UPDATE -> DELETE ? ?? at 2010-08-30
4 = ??? ?? ??? ?? ????? DELETE ??? ?? at 2010-09-02
5 = ??? ?? ??? ?? ???? ???? ??? ?? at 2010-10-08
**/

CREATE PROCEDURE [dbo].[P_ClearQuest]
	  @inbCharacterID bigint --// ??? ID
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
	DELETE dbo.Quests
	WHERE CharacterID = @inbCharacterID;

	DELETE dbo.CompleteQuests WHERE CharacterID = @inbCharacterID;
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
