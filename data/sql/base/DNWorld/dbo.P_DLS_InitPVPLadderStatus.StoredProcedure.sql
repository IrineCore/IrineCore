USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_InitPVPLadderStatus]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-20
description : PVP ?? ?? ??? ??????. (For Dolis)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_InitPVPLadderStatus]
	  @inbCharacterID bigint --// ??? ID
	, @inyPVPLadderCode tinyint --// 1=1:1, 2=2:2, 3=3:3, 4=4:4	
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
	/**_# [PVPLadderScores] ???? UPDATE???.*/
	UPDATE dbo.PVPLadderScores
	SET PVPLadderGradePoint = 1500, HiddenPVPLadderGradePoint = 0, WeeklyPVPCount = 0, LastPVPLadderScoreDate = @dt2Now
	WHERE CharacterID = @inbCharacterID AND PVPLadderCode = COALESCE(@inyPVPLadderCode, PVPLadderCode);

	/**_# [Characters] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @intPrevPVPLadderPoint = ISNULL(PVPLadderPoint, 0)
		, PVPLadderPoint = 0
	WHERE CharacterID = @inbCharacterID;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	IF @intLogTableSuffix = 0
		INSERT dbo.PointLogs0 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, 1, @intPrevPVPLadderPoint, 0, 0, dbo.FN_IP2IPNumber(@vchIP));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PointLogs1 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, 1, @intPrevPVPLadderPoint, 0, 0, dbo.FN_IP2IPNumber(@vchIP));

	ELSE
		INSERT dbo.PointLogs2 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, 1, @intPrevPVPLadderPoint, 0, 0, dbo.FN_IP2IPNumber(@vchIP));
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
