USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPEndLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-06
description : ???? PVP ?? ??? ?????. DNStaging ???
result set : PVPStartDate, PVPEndDate, PVPStartID, CharacterID, TeamCode, BreakIntoFlag, PVPResultCode, PVPPlayTime, PVPKillPoint, PVPAssistPoint, PVPSupportPoint, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, WorldID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
6 = ?? ??? ?? (????)     by 20110520.wgkim
7 = PVPEndLogs ?? ?? by ??? at 2012-03-09
8 = PVPEndLogs ?? ???? ?? by ??? at 2012-03-14
10 = ????? ?? ?? ?? (????, ??? ????? ??? ???? ?? ??) by ??? at 2012-04-02
11 = ??? ?? by ??? at 2013-07-25
**/

CREATE PROCEDURE [dbo].[P_GetListPVPEndLogForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dtmBasisDate), @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;

	IF @intLogTableSuffix = 0
		SELECT A.PVPStartDate, A.PVPEndDate, B.PVPStartID, B.CharacterID, B.TeamCode, B.BreakIntoFlag, B.PVPResultCode, B.PVPPlayTime, B.PVPKillPoint, B.PVPAssistPoint, B.PVPSupportPoint, @intWorldID AS WorldID
			 , SUM(ISNULL(B.vsWarriorKOWin, 0) + CASE WHEN C.seq = 1 THEN CAST(C.string AS int) ELSE 0 END) AS vsWarriorKOWin
			 , SUM(ISNULL(B.vsWarriorKOLose, 0) + CASE WHEN C.seq = 2 THEN CAST(C.string AS int) ELSE 0 END) AS vsWarriorKOLose 
			 , SUM(ISNULL(B.vsArcherKOWin, 0) + CASE WHEN C.seq = 3 THEN CAST(C.string AS int) ELSE 0 END) AS vsArcherKOWin
 			 , SUM(ISNULL(B.vsArcherKOLose, 0) + CASE WHEN C.seq = 4 THEN CAST(C.string AS int) ELSE 0 END) AS vsArcherKOLose
			 , SUM(ISNULL(B.vsSorceressKOWin, 0) + CASE WHEN C.seq = 5 THEN CAST(C.string AS int) ELSE 0 END) AS vsSorceressKOWin
			 , SUM(ISNULL(B.vsSorceressKOLose, 0) + CASE WHEN C.seq = 6 THEN CAST(C.string AS int) ELSE 0 END) AS vsSorceressKOLose
			 , SUM(ISNULL(B.vsClericKOWin, 0) + CASE WHEN C.seq = 7 THEN CAST(C.string AS int) ELSE 0 END) AS vsClericKOWin
			 , SUM(ISNULL(B.vsClericKOLose, 0) + CASE WHEN C.seq = 8 THEN CAST(C.string AS int) ELSE 0 END) AS vsClericKOLose
			 , SUM(ISNULL(B.vsAcademicKOWin, 0) + CASE WHEN C.seq = 9 THEN CAST(C.string AS int) ELSE 0 END) AS vsAcademicKOWin
			 , SUM(ISNULL(B.vsAcademicKOLose, 0) + CASE WHEN C.seq = 10 THEN CAST(C.string AS int) ELSE 0 END) AS vsAcademicKOLose
			 , SUM(CASE WHEN C.seq = 11 THEN CAST(C.string AS int) ELSE 0 END) AS vsKaliKOWin
			 , SUM(CASE WHEN C.seq = 12 THEN CAST(C.string AS int) ELSE 0 END) AS vsKaliKOLose
			 , OccupationCount
			 , OccupationTimeLimitWinFlag
			 , SUM(CASE WHEN C.seq = 13 THEN CAST(C.string AS int) ELSE 0 END) AS vsAssasinKOWin
			 , SUM(CASE WHEN C.seq = 14 THEN CAST(C.string AS int) ELSE 0 END) AS vsAssasinKOLose		
		FROM dbo.PVPStartIDs A WITH (NOLOCK)
			INNER JOIN dbo.PVPEndLogs0 B WITH (NOLOCK) ON A.PVPStartID = B.PVPStartID
			OUTER APPLY dbo.TF_Split(B.CharacterClassWinLose,',') C
		WHERE A.PVPStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND A.PVPStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		GROUP BY A.PVPStartDate, A.PVPEndDate, B.PVPStartID, B.CharacterID, B.TeamCode, B.BreakIntoFlag, B.PVPResultCode, B.PVPPlayTime, B.PVPKillPoint, B.PVPAssistPoint, B.PVPSupportPoint, B.OccupationCount, B.OccupationTimeLimitWinFlag
		OPTION (MAXDOP 1);
	ELSE IF @intLogTableSuffix = 1
		SELECT A.PVPStartDate, A.PVPEndDate, B.PVPStartID, B.CharacterID, B.TeamCode, B.BreakIntoFlag, B.PVPResultCode, B.PVPPlayTime, B.PVPKillPoint, B.PVPAssistPoint, B.PVPSupportPoint, @intWorldID AS WorldID
			 , SUM(ISNULL(B.vsWarriorKOWin, 0) + CASE WHEN C.seq = 1 THEN CAST(C.string AS int) ELSE 0 END) AS vsWarriorKOWin
			 , SUM(ISNULL(B.vsWarriorKOLose, 0) + CASE WHEN C.seq = 2 THEN CAST(C.string AS int) ELSE 0 END) AS vsWarriorKOLose 
			 , SUM(ISNULL(B.vsArcherKOWin, 0) + CASE WHEN C.seq = 3 THEN CAST(C.string AS int) ELSE 0 END) AS vsArcherKOWin
 			 , SUM(ISNULL(B.vsArcherKOLose, 0) + CASE WHEN C.seq = 4 THEN CAST(C.string AS int) ELSE 0 END) AS vsArcherKOLose
			 , SUM(ISNULL(B.vsSorceressKOWin, 0) + CASE WHEN C.seq = 5 THEN CAST(C.string AS int) ELSE 0 END) AS vsSorceressKOWin
			 , SUM(ISNULL(B.vsSorceressKOLose, 0) + CASE WHEN C.seq = 6 THEN CAST(C.string AS int) ELSE 0 END) AS vsSorceressKOLose
			 , SUM(ISNULL(B.vsClericKOWin, 0) + CASE WHEN C.seq = 7 THEN CAST(C.string AS int) ELSE 0 END) AS vsClericKOWin
			 , SUM(ISNULL(B.vsClericKOLose, 0) + CASE WHEN C.seq = 8 THEN CAST(C.string AS int) ELSE 0 END) AS vsClericKOLose
			 , SUM(ISNULL(B.vsAcademicKOWin, 0) + CASE WHEN C.seq = 9 THEN CAST(C.string AS int) ELSE 0 END) AS vsAcademicKOWin
			 , SUM(ISNULL(B.vsAcademicKOLose, 0) + CASE WHEN C.seq = 10 THEN CAST(C.string AS int) ELSE 0 END) AS vsAcademicKOLose
			 , SUM(CASE WHEN C.seq = 11 THEN CAST(C.string AS int) ELSE 0 END) AS vsKaliKOWin
			 , SUM(CASE WHEN C.seq = 12 THEN CAST(C.string AS int) ELSE 0 END) AS vsKaliKOLose
			 , OccupationCount
			 , OccupationTimeLimitWinFlag
			 , SUM(CASE WHEN C.seq = 13 THEN CAST(C.string AS int) ELSE 0 END) AS vsAssasinKOWin
			 , SUM(CASE WHEN C.seq = 14 THEN CAST(C.string AS int) ELSE 0 END) AS vsAssasinKOLose		
		FROM dbo.PVPStartIDs A WITH (NOLOCK)
			INNER JOIN dbo.PVPEndLogs1 B WITH (NOLOCK) ON A.PVPStartID = B.PVPStartID
			OUTER APPLY dbo.TF_Split(B.CharacterClassWinLose,',') C
		WHERE A.PVPStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND A.PVPStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		GROUP BY A.PVPStartDate, A.PVPEndDate, B.PVPStartID, B.CharacterID, B.TeamCode, B.BreakIntoFlag, B.PVPResultCode, B.PVPPlayTime, B.PVPKillPoint, B.PVPAssistPoint, B.PVPSupportPoint, B.OccupationCount, B.OccupationTimeLimitWinFlag
		OPTION (MAXDOP 1);
	ELSE
		SELECT A.PVPStartDate, A.PVPEndDate, B.PVPStartID, B.CharacterID, B.TeamCode, B.BreakIntoFlag, B.PVPResultCode, B.PVPPlayTime, B.PVPKillPoint, B.PVPAssistPoint, B.PVPSupportPoint, @intWorldID AS WorldID
			 , SUM(ISNULL(B.vsWarriorKOWin, 0) + CASE WHEN C.seq = 1 THEN CAST(C.string AS int) ELSE 0 END) AS vsWarriorKOWin
			 , SUM(ISNULL(B.vsWarriorKOLose, 0) + CASE WHEN C.seq = 2 THEN CAST(C.string AS int) ELSE 0 END) AS vsWarriorKOLose 
			 , SUM(ISNULL(B.vsArcherKOWin, 0) + CASE WHEN C.seq = 3 THEN CAST(C.string AS int) ELSE 0 END) AS vsArcherKOWin
 			 , SUM(ISNULL(B.vsArcherKOLose, 0) + CASE WHEN C.seq = 4 THEN CAST(C.string AS int) ELSE 0 END) AS vsArcherKOLose
			 , SUM(ISNULL(B.vsSorceressKOWin, 0) + CASE WHEN C.seq = 5 THEN CAST(C.string AS int) ELSE 0 END) AS vsSorceressKOWin
			 , SUM(ISNULL(B.vsSorceressKOLose, 0) + CASE WHEN C.seq = 6 THEN CAST(C.string AS int) ELSE 0 END) AS vsSorceressKOLose
			 , SUM(ISNULL(B.vsClericKOWin, 0) + CASE WHEN C.seq = 7 THEN CAST(C.string AS int) ELSE 0 END) AS vsClericKOWin
			 , SUM(ISNULL(B.vsClericKOLose, 0) + CASE WHEN C.seq = 8 THEN CAST(C.string AS int) ELSE 0 END) AS vsClericKOLose
			 , SUM(ISNULL(B.vsAcademicKOWin, 0) + CASE WHEN C.seq = 9 THEN CAST(C.string AS int) ELSE 0 END) AS vsAcademicKOWin
			 , SUM(ISNULL(B.vsAcademicKOLose, 0) + CASE WHEN C.seq = 10 THEN CAST(C.string AS int) ELSE 0 END) AS vsAcademicKOLose
			 , SUM(CASE WHEN C.seq = 11 THEN CAST(C.string AS int) ELSE 0 END) AS vsKaliKOWin
			 , SUM(CASE WHEN C.seq = 12 THEN CAST(C.string AS int) ELSE 0 END) AS vsKaliKOLose
			 , OccupationCount
			 , OccupationTimeLimitWinFlag
			 , SUM(CASE WHEN C.seq = 13 THEN CAST(C.string AS int) ELSE 0 END) AS vsAssasinKOWin
			 , SUM(CASE WHEN C.seq = 14 THEN CAST(C.string AS int) ELSE 0 END) AS vsAssasinKOLose		
		FROM dbo.PVPStartIDs A WITH (NOLOCK)
			INNER JOIN dbo.PVPEndLogs2 B WITH (NOLOCK) ON A.PVPStartID = B.PVPStartID
			OUTER APPLY dbo.TF_Split(B.CharacterClassWinLose,',') C
		WHERE A.PVPStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND A.PVPStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		GROUP BY A.PVPStartDate, A.PVPEndDate, B.PVPStartID, B.CharacterID, B.TeamCode, B.BreakIntoFlag, B.PVPResultCode, B.PVPPlayTime, B.PVPKillPoint, B.PVPAssistPoint, B.PVPSupportPoint, B.OccupationCount, B.OccupationTimeLimitWinFlag
		OPTION (MAXDOP 1);

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
