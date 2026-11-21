USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCompleteQuest]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-15
description : ??? ??? ??? ?????.
result set : 
	@inySelectClauseCode ??
	1 = QuestID/int, CompleteDate/datetime2, CompleteCount/int (DOLIS?? ??)
	2 = QuestID/int (?????? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103198 = ??? ???? ????.
history :
3 = input param. @inySelectClauseCode ?? at 2011-08-22
5~6 = ???? ???? ?? ?? ?? ???? ???? ????? ?? (?? ?? ???? ?? SP ? ??)     by ??? at 2013-08-27
7 = 5~6 ???? ??     by ??? at 2013-08-27
**/

CREATE PROCEDURE [dbo].[P_GetListCompleteQuest]
	  @inbCharacterID bigint --// ??? ID
	, @inySelectClauseCode tinyint = 1 --// ??? ?? ? ??
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
	IF @inySelectClauseCode = 1
		SELECT QuestID, CompleteDate, CompleteCount
		FROM dbo.CompleteQuests WITH (NOLOCK, INDEX(PK_CompleteQuests), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		ORDER BY CompleteDate
		OPTION (KEEPFIXED PLAN);

	ELSE IF @inySelectClauseCode = 2
		SELECT QuestID
		  FROM dbo.CompleteQuests WITH (NOLOCK, INDEX(PK_CompleteQuests), FORCESEEK)
		 WHERE CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103198;
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
