USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPeriodicQuest]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-11
description : ?? ???? ?? ??? ??? ??? ?????.
result set : QuestID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPeriodicQuest]
	  @inyPeriod tinyint --// ?? ??. ?? : day
	, @dt2InitializeDate datetime2(0) OUTPUT --// ??? ??? ??? ??
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
	SELECT @dt2InitializeDate = InitializeDate
	FROM dbo.PeriodicQuestInitializeDates WITH (NOLOCK)
	WHERE Period = @inyPeriod;

	SELECT QuestID
	FROM dbo.PeriodicQuests WITH (NOLOCK)
	WHERE Period = @inyPeriod
	ORDER BY QuestID;
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
