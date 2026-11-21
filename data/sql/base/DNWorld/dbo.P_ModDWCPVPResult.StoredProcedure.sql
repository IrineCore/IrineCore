USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModDWCPVPResult]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-10
description : DWC PVP ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103361 = ???? ?? DWC????.
**/

CREATE PROCEDURE [dbo].[P_ModDWCPVPResult]
	  @intDWCTeamID int --// DWC ?ID 
	, @inyResultCode tinyint --// 1=??,2=??,3=???
	, @intDWCPoint int --// DWC ?? ??
	, @intDWCHiddenPoint int --// DWC Hidden ??
	, @intvsDWCTeamID int --// ?? DWC ?ID
	, @intCurrentWin int OUTPUT --// ?? ?? ??
	, @intCurrentLose int OUTPUT --// ?? ?? ??
	, @intCurrentDraw int OUTPUT --// ?? ??? ??
	, @insCurrentTodayWin smallint OUTPUT --// ?? ?? ?? ??
	, @insCurrentTodayLose smallint OUTPUT --// ?? ?? ?? ??
	, @insCurrentTodayDraw smallint OUTPUT --// ?? ?? ??? ??
	, @insCurrentConsecutiveWin smallint OUTPUT --// ?? ?? ??
	, @insCurrentConsecutiveLose smallint OUTPUT --// ?? ?? ??
	, @insWeeklyPlayCount smallint OUTPUT --// ?? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Now datetime2(0)= GETDATE()
      , @intCurrentDWCPoint int;
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [DWCTeams] ???? UPDATE???.*/
	UPDATE dbo.DWCTeams
	SET @intCurrentWin = Win += CASE WHEN @inyResultCode = 1 THEN 1 ELSE 0 END
      , @intCurrentLose = Lose += CASE WHEN @inyResultCode = 2 THEN 1 ELSE 0 END
      , @intCurrentDraw = Draw += CASE WHEN @inyResultCode = 3 THEN 1 ELSE 0 END
	  , @intCurrentDWCPoint = DWCPoint
      , DWCPoint = @intDWCPoint
      , @insCurrentTodayWin = TodayWin = CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN CASE WHEN @inyResultCode = 1 THEN TodayWin + 1 ELSE TodayWin END ELSE CASE WHEN @inyResultCode = 1 THEN 1 ELSE 0 END END
      , @insCurrentTodayLose = TodayLose = CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN CASE WHEN @inyResultCode = 2 THEN TodayLose + 1 ELSE TodayLose END ELSE CASE WHEN @inyResultCode = 2 THEN 1 ELSE 0 END END
      , @insCurrentTodayDraw = TodayDraw = CASE CAST(@dt2Now AS date) WHEN CAST(LastPlayDate AS date) THEN CASE WHEN @inyResultCode = 3 THEN TodayDraw + 1 ELSE TodayDraw END ELSE CASE WHEN @inyResultCode = 3 THEN 1 ELSE 0 END END
      , @insCurrentConsecutiveWin = ConsecutiveWin += CASE WHEN @inyResultCode = 1 THEN 1 ELSE - ConsecutiveWin END
      , @insCurrentConsecutiveLose = ConsecutiveLose += CASE WHEN @inyResultCode = 2 THEN 1 ELSE - ConsecutiveLose END
      , @insWeeklyPlayCount = WeeklyPlayCount += CASE WHEN @inyResultCode IN (1, 2) THEN 1 ELSE 0 END 
      , LastPlayDate = @dt2Now 
	  , DWCHiddenPoint = @intDWCHiddenPoint
	WHERE DWCTeamID = @intDWCTeamID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103361;
		GOTO ErrorHandler;
	END

	/**_# [DWCTeamPointPlayHistory] ???? INSERT???.*/
	INSERT dbo.DWCTeamPointPlayHistory (DWCTeamID, vsDWCTeamID, DWCPointChangeCode, DWCPointBefore, DWCPoint, RegisterDate)
	VALUES (@intDWCTeamID, @intvsDWCTeamID, @inyResultCode, @intCurrentDWCPoint, @intDWCPoint, @dt2Now);

	/**_# [DWCPersonalPlayHistory] ???? INSERT???.*/
	INSERT dbo.DWCPersonalPlayHistory (CharacterID, DWCTeamID, ResultCode, GainDWCPoint, RegisterDate)
	SELECT CharacterID, @intDWCTeamID, @inyResultCode, @intDWCPoint - @intCurrentDWCPoint, @dt2Now
	FROM dbo.DWCTeamMembers WITH(NOLOCK, INDEX(PK_DWCTeamMembers), FORCESEEK)
	WHERE DWCTeamID = @intDWCTeamID;

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
