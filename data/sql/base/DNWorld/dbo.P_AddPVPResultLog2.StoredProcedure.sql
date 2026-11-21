USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPResultLog2]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-03-07
description : ???? PVP ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = ????? ?? ?? ?? (????, ??? ????? ??? ???? ?? ??) by ??? at 2012-04-02
**/

CREATE PROCEDURE [dbo].[P_AddPVPResultLog2]
	  @inbRoomID1 bigint --// ??? ? ID 1
	, @intRoomID2 int --// ??? ? ID 2
	, @inbCharacterID bigint --// ??? ID
	, @inyTeamCode tinyint --// 1=A?, 2=B?
	, @bitBreakIntoFlag bit --// 0=???? ??, 1=?? ?
	, @inyPVPResultCode tinyint --// 1=??, 2=??, 3=???, 4=??
	, @intPVPPlayTime int --// PVP ??? ?? (?)
	, @intPVPKillPoint int --// PVP ? ???
	, @intPVPAssistPoint int --// PVP ???? ???
	, @intPVPSupportPoint int --// PVP Support ???
	, @vchCharacterClassWinLose varchar(max) = '' --// ,? ?????? ???? ??,?? ??? ex)?? ????? ?? ???. 5?? ???? ????? '1,1,2,3,5,6,8,9,12,1' ????? ?? ???
	, @intOccupationCount int = NULL --// ????(???)
	, @bitOccupationTimeLimitWinFlag bit = NULL --//0=?? ???? ??, 1=?? ???? ???? ?? ??(???)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbPVPStartID bigint, @dt2PVPStartDate datetime2(0), @intLogTableSuffix int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT TOP (1) @inbPVPStartID = PVPStartID, @dt2PVPStartDate = PVPStartDate
	FROM dbo.PVPStartIDs WITH (NOLOCK)
	WHERE RoomID1 = @inbRoomID1 AND RoomID2 = @intRoomID2
	ORDER BY PVPStartID DESC;

	IF @@ROWCOUNT = 0 OR (DATEDIFF(DAY, @dt2PVPStartDate, @dt2Now) > 0 AND DATEDIFF(MINUTE, @dt2PVPStartDate, @dt2Now) > 180)
		RETURN 0;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2PVPStartDate);

	/**_# [PVPEndLogs0] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.PVPEndLogs0 (PVPStartID, CharacterID, TeamCode, BreakIntoFlag, PVPResultCode, PVPPlayTime, PVPKillPoint, PVPAssistPoint, PVPSupportPoint, CharacterClassWinLose, OccupationCount, OccupationTimeLimitWinFlag)
		VALUES (@inbPVPStartID, @inbCharacterID, @inyTeamCode, @bitBreakIntoFlag, @inyPVPResultCode, @intPVPPlayTime, @intPVPKillPoint, @intPVPAssistPoint, @intPVPSupportPoint, @vchCharacterClassWinLose, @intOccupationCount, @bitOccupationTimeLimitWinFlag);
	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PVPEndLogs1 (PVPStartID, CharacterID, TeamCode, BreakIntoFlag, PVPResultCode, PVPPlayTime, PVPKillPoint, PVPAssistPoint, PVPSupportPoint, CharacterClassWinLose, OccupationCount, OccupationTimeLimitWinFlag)
		VALUES (@inbPVPStartID, @inbCharacterID, @inyTeamCode, @bitBreakIntoFlag, @inyPVPResultCode, @intPVPPlayTime, @intPVPKillPoint, @intPVPAssistPoint, @intPVPSupportPoint, @vchCharacterClassWinLose, @intOccupationCount, @bitOccupationTimeLimitWinFlag);
	ELSE
		INSERT dbo.PVPEndLogs2 (PVPStartID, CharacterID, TeamCode, BreakIntoFlag, PVPResultCode, PVPPlayTime, PVPKillPoint, PVPAssistPoint, PVPSupportPoint, CharacterClassWinLose, OccupationCount, OccupationTimeLimitWinFlag)
		VALUES (@inbPVPStartID, @inbCharacterID, @inyTeamCode, @bitBreakIntoFlag, @inyPVPResultCode, @intPVPPlayTime, @intPVPKillPoint, @intPVPAssistPoint, @intPVPSupportPoint, @vchCharacterClassWinLose, @intOccupationCount, @bitOccupationTimeLimitWinFlag);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX/_UN/_PVPEndLogs%' ESCAPE('/')
		RETURN 0;

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
