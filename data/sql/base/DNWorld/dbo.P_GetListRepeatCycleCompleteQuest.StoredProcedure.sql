USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListRepeatCycleCompleteQuest]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2013-08-27

description :
	??? ?? ?? ??? ?? ??
result set : 
	QuestCode / tinyint / 1=?????, 2=?????, 3=?????, 4=?? ?? ???
	CompleteCount / int / ??? ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListRepeatCycleCompleteQuest]
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

	/**_# ??? ?? ?? ??? ??? ?????. */
	SELECT QuestID, CompleteCount
	  FROM dbo.CompleteQuests WITH (NOLOCK, INDEX(PK_CompleteQuests), FORCESEEK)
	 WHERE CharacterID = @inbCharacterID
	   AND QuestCode = 4
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	RETURN @intReturnValue;
GO
