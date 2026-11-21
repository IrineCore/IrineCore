USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddQuest]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103118 = ?? ??? ??????.
history :
5 = @inyQuestCode input param. ?? at 2010-06-28
7 = UPSERT -> INSERT ? ?? at 2010-08-30
8~9 = input parameter ?? ?? (@inyQuestCode)     by ??? at 2013-08-27
**/

CREATE PROCEDURE [dbo].[P_AddQuest]
	  @inbCharacterID bigint --// ??? ID
	, @inyQuestIndex tinyint --// ??? ?? ???
	, @intQuestID int --// ??? ID
	, @inyQuestStatusCode tinyint --// 0=?????, 1=?????
	, @insQuestStep smallint --// ??? ??
	, @inyQuestJournal tinyint --// ??? ??
	, @binQuestExtra binary(90) --// ??? ?? ?? ????
	, @inyMemo1 tinyint --// ??1
	, @inyMemo2 tinyint --// ??2
	, @inyMemo3 tinyint --// ??3
	, @inyMemo4 tinyint --// ??4
	, @inyMemo5 tinyint --// ??5
	, @inyMemo6 tinyint --// ??6
	, @inyMemo7 tinyint --// ??7
	, @inyMemo8 tinyint --// ??8
	, @inyMemo9 tinyint --// ??9
	, @inyMemo10 tinyint --// ??10
	, @inyQuestCode tinyint = 1 --// 1=?????, 2=?????, 3=?????, 4=?? ?? ???
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
	/**_# [Quests] ???? INSERT???.*/
	INSERT dbo.Quests (CharacterID, QuestIndex, QuestID, QuestStatusCode, QuestStep, QuestJournal, QuestExtra, Memo1, Memo2, Memo3, Memo4, Memo5, Memo6, Memo7, Memo8, Memo9, Memo10, QuestCode, RegisterDate)
	VALUES (@inbCharacterID, @inyQuestIndex, @intQuestID, @inyQuestStatusCode, @insQuestStep, @inyQuestJournal, @binQuestExtra, @inyMemo1, @inyMemo2, @inyMemo3, @inyMemo4, @inyMemo5, @inyMemo6, @inyMemo7, @inyMemo8, @inyMemo9, @inyMemo10, @inyQuestCode, GETDATE())
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_Quests_CharacterID_QuestID%'
		SET @intReturnValue = 103118;

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
