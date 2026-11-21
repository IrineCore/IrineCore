USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModQuest]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103116 = ???? ????.
103120 = ??? ???? QuestID? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModQuest]
	  @inbCharacterID bigint --// ??? ID
	, @inyQuestIndex tinyint --// ??? ?? ???
	, @intQuestID int --// ??? ID
	, @intModFieldBitmap int --// ??? ??? bitmap
	, @inyQuestStatusCode tinyint --// 0=?????, 1=????? (0)
	, @insQuestStep smallint --// ??? ?? (1)
	, @inyQuestJournal tinyint --// ??? ?? (2)
	, @binQuestExtra binary(90) --// ??? ?? ?? ???? (3)
	, @inyMemo1 tinyint --// ??1 (4)
	, @inyMemo2 tinyint --// ??2 (5)
	, @inyMemo3 tinyint --// ??3 (6)
	, @inyMemo4 tinyint --// ??4 (7)
	, @inyMemo5 tinyint --// ??5 (8)
	, @inyMemo6 tinyint --// ??6 (9)
	, @inyMemo7 tinyint --// ??7 (10)
	, @inyMemo8 tinyint --// ??8 (11)
	, @inyMemo9 tinyint --// ??9 (12)
	, @inyMemo10 tinyint --// ??10 (13)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intOrginalQuestID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;
		/**_# [Quests]???? UPDATE???.*/
		UPDATE dbo.Quests
		SET @intOrginalQuestID = QuestID
			, QuestStatusCode = CASE @intModFieldBitmap & POWER(2, 0) WHEN 0 THEN QuestStatusCode ELSE @inyQuestStatusCode END
			, QuestStep = CASE @intModFieldBitmap & POWER(2, 1) WHEN 0 THEN QuestStep ELSE @insQuestStep END
			, QuestJournal = CASE @intModFieldBitmap & POWER(2, 2) WHEN 0 THEN QuestJournal ELSE @inyQuestJournal END
			, QuestExtra = CASE @intModFieldBitmap & POWER(2, 3) WHEN 0 THEN QuestExtra ELSE @binQuestExtra END
			, Memo1 = CASE @intModFieldBitmap & POWER(2, 4) WHEN 0 THEN Memo1 ELSE @inyMemo1 END
			, Memo2 = CASE @intModFieldBitmap & POWER(2, 5) WHEN 0 THEN Memo2 ELSE @inyMemo2 END
			, Memo3 = CASE @intModFieldBitmap & POWER(2, 6) WHEN 0 THEN Memo3 ELSE @inyMemo3 END
			, Memo4 = CASE @intModFieldBitmap & POWER(2, 7) WHEN 0 THEN Memo4 ELSE @inyMemo4 END
			, Memo5 = CASE @intModFieldBitmap & POWER(2, 8) WHEN 0 THEN Memo5 ELSE @inyMemo5 END
			, Memo6 = CASE @intModFieldBitmap & POWER(2, 9) WHEN 0 THEN Memo6 ELSE @inyMemo6 END
			, Memo7 = CASE @intModFieldBitmap & POWER(2, 10) WHEN 0 THEN Memo7 ELSE @inyMemo7 END
			, Memo8 = CASE @intModFieldBitmap & POWER(2, 11) WHEN 0 THEN Memo8 ELSE @inyMemo8 END
			, Memo9 = CASE @intModFieldBitmap & POWER(2, 12) WHEN 0 THEN Memo9 ELSE @inyMemo9 END
			, Memo10 = CASE @intModFieldBitmap & POWER(2, 13) WHEN 0 THEN Memo10 ELSE @inyMemo10 END
		WHERE CharacterID = @inbCharacterID AND QuestIndex = @inyQuestIndex;

		/**_# ??? ?? ??? 103116? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103116;
			GOTO ErrorHandler;
		END

		/**_# ??? ?? QuestID? ???? ??? 103120? ?????.*/
		IF @intOrginalQuestID <> @intQuestID
		BEGIN
			SET @intReturnValue = 103120;
			GOTO ErrorHandler;
		END
	COMMIT TRANSACTION;
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
