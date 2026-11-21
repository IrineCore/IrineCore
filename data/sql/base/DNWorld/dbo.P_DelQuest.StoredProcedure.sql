USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelQuest]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 15
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103116 = ???? ????.
103121 = ?? ??? ??????.
history :
7 = ??? ?? QuestCode, RegisterDate? NULL? ???? ??? ?? at 2010-06-28
9 = QuestCode ?? ?? at 2010-06-30
10 = dbo.CompleteQuestBinaries ???? ???? ??? ?? at 2010-08-30
11 = dbo.Quests ?????? UPDATE -> DELETE ? ?? at 2010-08-30
13 = ?????? ?? ?? by ??? at 2012-10-17
**/

CREATE PROCEDURE [dbo].[P_DelQuest]
	  @inbCharacterID bigint --// ??? ID
	, @inyQuestIndex tinyint --// ??? ?? ???
	, @intQuestID int --// ??? ID
	, @bitQuestCompleteFlag bit --// 0=?? ??? ??, 1=??? ?? ??? ?? ??
	, @bitRepetitionQuestFlag bit --// 0=?? ???, 1=?? ???
	, @binCompleteQuestBin binary(1250) = NULL --// (optional) ??? ??? ???? (??? ??? ??? ???? ? ??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @intLogTableSuffix int
	  ,	@dt2Now datetime2(0) = GETDATE();
DECLARE @tblQuests table (CharacterID bigint NOT NULL, QuestID int NOT NULL, QuestCode tinyint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# [Quests] ???? UPDATE???.*/
	DELETE dbo.Quests
	OUTPUT deleted.CharacterID, deleted.QuestID, deleted.QuestCode INTO @tblQuests (CharacterID, QuestID, QuestCode)
	WHERE CharacterID = @inbCharacterID AND QuestIndex = @inyQuestIndex AND QuestID = @intQuestID;

	/**_# ??? ?? ??? ?? ???? ??? ??? 103117? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103116;
		GOTO ErrorHandler;
	END

	/**_# ???? ???? ???? ??, [CompleteQuests] ???? UPSERT???.*/
	IF @bitQuestCompleteFlag = 1
	BEGIN
		MERGE dbo.CompleteQuests T
		USING @tblQuests S
		ON T.CharacterID = S.CharacterID AND T.QuestID = S.QuestID
		WHEN MATCHED THEN
			UPDATE SET CompleteCount += 1
		WHEN NOT MATCHED BY TARGET THEN
			INSERT (CharacterID, QuestID, CompleteDate, CompleteCount, QuestCode)
			VALUES (S.CharacterID, S.QuestID, @dt2Now, 1, S.QuestCode);

		IF @bitRepetitionQuestFlag = 1
		BEGIN
			SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);
			/**_# [QuestCompleteLogs] ???? INSERT???.*/
			IF @intLogTableSuffix = 0
				INSERT dbo.RepetitionQuestCompleteLogs0 (CompleteDate, CharacterID, QuestID)
				VALUES (@dt2Now, @inbCharacterID, @intQuestID);

			ELSE IF @intLogTableSuffix = 1
				INSERT dbo.RepetitionQuestCompleteLogs1 (CompleteDate, CharacterID, QuestID)
				VALUES (@dt2Now, @inbCharacterID, @intQuestID);

			ELSE
				INSERT dbo.RepetitionQuestCompleteLogs2 (CompleteDate, CharacterID, QuestID)
				VALUES (@dt2Now, @inbCharacterID, @intQuestID);
		END
	END

	COMMIT TRANSACTION;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_CompleteQuests_CharacterID%'
		SET @intReturnValue = 103102;

	ELSE IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_CompleteQuests%'
		SET @intReturnValue = 103121;

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
