USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RenewPeriodicQuest]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-11
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = output param. @dt2InitializeDate ?? at 2010-11-18
4 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_RenewPeriodicQuest]
	  @inyPeriod tinyint --// ?? ??. ?? : day
	, @vchQuestIDs varchar(8000) --// ,? ???? ?? ??? ID (?? 500?)
	, @dt2InitializeDate datetime2(0) = NULL OUTPUT --// ??? ??? ??? ??
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
	SET @dt2InitializeDate = GETDATE();

	BEGIN TRANSACTION;

	/**_# [PeriodicQuests] ????? DELETE???.*/
	DELETE dbo.PeriodicQuests WHERE Period = @inyPeriod;

	/**_# [PeriodicQuests] ???? INSERT???.*/
	INSERT dbo.PeriodicQuests (Period, QuestID)
	SELECT @inyPeriod, CAST(string AS int)
	FROM dbo.TF_Split(@vchQuestIDs, ',')
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);

	/**_# [PeriodicQuestInitializeDates] ???? UPDATE ???.*/
	UPDATE dbo.PeriodicQuestInitializeDates
	SET InitializeDate = @dt2InitializeDate
	WHERE Period = @inyPeriod;

	IF @@ROWCOUNT = 0
		INSERT dbo.PeriodicQuestInitializeDates (Period, InitializeDate)
		VALUES (@inyPeriod, @dt2InitializeDate);

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
