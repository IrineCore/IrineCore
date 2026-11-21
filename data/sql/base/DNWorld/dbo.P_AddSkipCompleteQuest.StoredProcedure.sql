USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddSkipCompleteQuest]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-08
description : Quests? ???? ?? ???? ??? ???? ?? ????? ?? ?????. (?? ???? P_DelQuest? ???? ??????.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddSkipCompleteQuest]
	  @inbCharacterID bigint --// ??? ID
	, @vchQuestID varchar(8000) --// (??? ID) ,? ?????? ???ID ???
	, @vchQuestCode varchar(8000) --// (1=?? ???, 2=??? ?? ???) ,? ?????? ????? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  ,	@dt2Now datetime2(0) = GETDATE();

DECLARE @tblTemp table (
	QuestID int NOT NULL,
	QuestCode tinyint NOT NULL
);


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblTemp (QuestID, QuestCode)
	SELECT CAST(A.string as int), CAST(B.string as tinyint) 
	FROM dbo.TF_Split(@vchQuestID,',') A 
		INNER JOIN dbo.TF_Split(@vchQuestCode,',') B ON A.seq = B.seq;

	;WITH T AS (
		SELECT CharacterID, QuestID, CompleteDate, CompleteCount, QuestCode
		FROM dbo.CompleteQuests WITH (UPDLOCK)
		WHERE CharacterID = @inbCharacterID
	), S (CharacterID, QuestID, CompleteDate, CompleteCount, QuestCode) AS (
		SELECT @inbCharacterID, QuestID, @dt2Now, 1, QuestCode
		FROM @tblTemp
	)
	MERGE T
	USING S
	ON T.CharacterID = S.CharacterID AND T.QuestID = S.QuestID
	WHEN MATCHED THEN
		UPDATE SET CompleteCount += 1
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (CharacterID, QuestID, CompleteDate, CompleteCount, QuestCode) 
		VALUES (S.CharacterID, S.QuestID, S.CompleteDate, S.CompleteCount, S.QuestCode);

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
