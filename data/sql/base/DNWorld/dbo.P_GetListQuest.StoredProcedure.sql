USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListQuest]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ??? ??? ?????.
result set : QuestIndex/tinyint, QuestID/int, QuestStatusCode/tinyint, QuestStep/smallint, QuestJournal/tinyint, QuestExtra/binary, Memo1/tinyint, Memo2/tinyint, Memo3/tinyint, Memo4/tinyint, Memo5/tinyint, Memo6/tinyint, Memo7/tinyint, Memo8/tinyint, Memo9/tinyint, Memo10/tinyint, RegisterDate/datetime2(0)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103116 = ???? ????.
history :
2 = ??? ?? RegisterDate? ????? ?? at 2010-06-28
**/

CREATE PROCEDURE [dbo].[P_GetListQuest]
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
	SELECT QuestIndex, QuestID, QuestStatusCode, QuestStep, QuestJournal, QuestExtra, Memo1, Memo2, Memo3, Memo4, Memo5, Memo6, Memo7, Memo8, Memo9, Memo10, RegisterDate
	FROM dbo.Quests WITH (NOLOCK, INDEX(PK_Quests), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION(KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103116;
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
