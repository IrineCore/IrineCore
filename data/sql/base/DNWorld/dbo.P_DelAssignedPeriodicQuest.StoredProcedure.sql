USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelAssignedPeriodicQuest]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-11
description : ??? ???? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = output param. @dt2AssignDate & [AssignQuests] ???? MERGE ?? ?? ?? at 2010-11-18
6 = (Tuning) Split ?? ??     by 20110817.wgkim
7 = ?? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_DelAssignedPeriodicQuest]
	  @inbCharacterID bigint --// ??? ID
	, @inyPeriod tinyint --// ???? ??? ???? ??. ?? : day
	, @vchQuestIDs varchar(max) --// ,? ?????? ??? ??? ??? ID ???
	, @dt2AssignDate datetime2(0) = NULL OUTPUT --// ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @intIndex1 int
	  , @intQuestID int;

DECLARE @tblQuestIDs table (QuestID int NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SET @dt2AssignDate = GETDATE();

	WHILE (LEN(@vchQuestIDs) > 0)
	BEGIN

		SELECT @intIndex1 = CHARINDEX(',', @vchQuestIDs);

		IF (@intIndex1 = 0) AND (LEN(@vchQuestIDs) > 0)
		BEGIN
			SELECT @intQuestID = @vchQuestIDs;
			INSERT @tblQuestIDs (QuestID) SELECT @intQuestID;

			BREAK;
		END

		IF (@intIndex1 > 1)
		BEGIN
			SELECT @intQuestID = LEFT(@vchQuestIDs, @intIndex1 - 1);

			INSERT @tblQuestIDs (QuestID) SELECT @intQuestID;

			SELECT @vchQuestIDs = RIGHT(@vchQuestIDs, LEN(@vchQuestIDs) - @intIndex1);
		END
		ELSE
		BEGIN
			SELECT @vchQuestIDs = RIGHT(@vchQuestIDs, LEN(@vchQuestIDs) - @intIndex1);
		END

	END

	/**_# [CompleteQuests] ????? DELETE???.*/
	DELETE dbo.CompleteQuests
	FROM CompleteQuests Q
		INNER JOIN @tblQuestIDs T ON Q.QuestID = T.QuestID
	WHERE Q.CharacterID = @inbCharacterID
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	/**_# [Quests] ????? DELETE???.*/
	DELETE dbo.Quests
	FROM Quests Q
		INNER JOIN @tblQuestIDs T ON Q.QuestID = T.QuestID
	WHERE Q.CharacterID = @inbCharacterID
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	/**_# [AssignQuests] ???? UPSERT???.*/
	UPDATE dbo.AssignQuests
	SET AssignDate = @dt2AssignDate
	WHERE CharacterID = @inbCharacterID AND Period = @inyPeriod;

	IF @@ROWCOUNT = 0
		INSERT dbo.AssignQuests (CharacterID, Period, AssignDate)
		VALUES (@inbCharacterID, @inyPeriod, @dt2AssignDate);

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
