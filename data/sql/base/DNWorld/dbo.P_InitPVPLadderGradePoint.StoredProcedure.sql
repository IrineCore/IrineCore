USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_InitPVPLadderGradePoint]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-10
description : PVP ?? ??? ??? & PVP ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103400 = ?? ??? ???? ??? ? ?? ???? ????.
history :
4 = ??? ?? WeeklyPVPCount? 0?? update??? ?? at 2011-02-16
**/

CREATE PROCEDURE [dbo].[P_InitPVPLadderGradePoint]
	  @inbCharacterID bigint --// ??? ID
	, @inyPVPLadderCode tinyint --// 1=1:1, 2=2:2, 3=3:3, 4=4:4
	, @intPVPLadderPoint int --// PVP ?? ??? (???? ?????)
	, @intPVPLadderGradePoint int --// PVP ?? ?? (???? PVP ?? ??? ?????)
	, @vchIP varchar(15) --// ????? IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intPrevPVPLadderPoint int, @intLogTableSuffix int, @dt2Now datetime2(0) = GETDATE();
DECLARE @intCurrentPVPLadderPoint int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	UPDATE dbo.PVPLadderScores
	SET  PVPLadderGradePoint = @intPVPLadderGradePoint
		, TodayWin = CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN TodayWin ELSE 0 END
		, TodayLose = CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN TodayLose ELSE 0 END
		, TodayDraw = CASE CAST(@dt2Now AS date) WHEN CAST(LastPVPLadderScoreDate AS date) THEN TodayDraw ELSE 0 END
		, LastPVPLadderScoreDate = @dt2Now
		, WeeklyPVPCount = 0
	WHERE CharacterID = @inbCharacterID AND PVPLadderCode = COALESCE(@inyPVPLadderCode, PVPLadderCode);

	IF @intPVPLadderPoint > 0
	BEGIN
		UPDATE dbo.CharacterStatus
		SET @intPrevPVPLadderPoint = ISNULL(PVPLadderPoint, 0)
			, PVPLadderPoint = ISNULL(PVPLadderPoint, 0) + @intPVPLadderPoint
		WHERE CharacterID = @inbCharacterID;

		SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

		IF @intLogTableSuffix = 0
			INSERT dbo.PointLogs0 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
			VALUES (@dt2Now, @inbCharacterID, 1, @intPrevPVPLadderPoint, @intPrevPVPLadderPoint + @intPVPLadderPoint, 0, dbo.FN_IP2IPNumber(@vchIP));

		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.PointLogs1 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
			VALUES (@dt2Now, @inbCharacterID, 1, @intPrevPVPLadderPoint, @intPrevPVPLadderPoint + @intPVPLadderPoint, 0, dbo.FN_IP2IPNumber(@vchIP));

		ELSE
			INSERT dbo.PointLogs2 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
			VALUES (@dt2Now, @inbCharacterID, 1, @intPrevPVPLadderPoint, @intPrevPVPLadderPoint + @intPVPLadderPoint, 0, dbo.FN_IP2IPNumber(@vchIP));
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
