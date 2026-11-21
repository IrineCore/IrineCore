USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddSS_AchieveMissions]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-08-22

description :
	??? ??? SS_AchieveMissions? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	6 = dbo.SS_AchieveMissionCounts ?? ?? ??     by 20110824.wgkim
	7 = ??? ???? ???? ???? DISTINCT? ??? ??	by 20110826.ohtack
	8 = ??? ?? ?? ?? ??	by 20110906.ohtack
	9 = [SS_AchieveMissions] ?? ??? ???? INSERT ??? ??, [SS_AchieveMissionCounts]? INSERT ?? ? ??? ?? ? INSERT??? ??	by 20110906.ohtack
	10 = [SS_AchieveMissionCounts] ??? TRUNCATE ??(??? ????? ??)	by 20110906.ohtack
	11 = ??? ???? ???? ???? DISTINCT? ??? ??	by 20110916.ohtack
**/

CREATE PROCEDURE [dbo].[P_AddSS_AchieveMissions]

WITH EXECUTE AS 'SPExecutor'
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

	/**_# [AchieveMissionsBuffer] ???? ???? ?????. */
	IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_NC_AchieveMissionsBuffer')
		CREATE CLUSTERED INDEX IX_NC_AchieveMissionsBuffer ON dbo.AchieveMissionsBuffer (CharacterID, MissionID);

	/**_# [SS_AchieveMissions] ???? [AchieveMissionsBuffer]? ?? ???? INSERT ??? */
	INSERT dbo.SS_AchieveMissions (CharacterID, MissionID)
	SELECT DISTINCT CharacterID,MissionID
	FROM dbo.AchieveMissionsBuffer A WITH (NOLOCK)
	WHERE NOT EXISTS (SELECT * FROM dbo.SS_AchieveMissions WITH (NOLOCK) WHERE A.CharacterID = CharacterID AND A.MissionID = MissionID)
	OPTION (MAXDOP 1);

	/**_# [AchieveMissionsBuffer] ???? ??? ???? ?????. */
	IF EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_NC_AchieveMissionsBuffer')
		DROP INDEX IX_NC_AchieveMissionsBuffer ON dbo.AchieveMissionsBuffer;

	/**_# [SS_AchieveMissionCounts]? ???? ?????. */
	TRUNCATE TABLE dbo.SS_AchieveMissionCounts;

	/**_# [SS_AchieveMissionCounts] ???? ???? ?????. */
	IF EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_NC_AchieveMissionCounts')
		DROP INDEX IX_NC_AchieveMissionCounts ON dbo.SS_AchieveMissionCounts;
	
	/**_# [SS_AchieveMissionCounts] ???? ??? ?? ???? ? ??? ?????*/
    INSERT dbo.SS_AchieveMissionCounts (CharacterID, MissionCount, MissionScore)
    SELECT B.CharacterID
         , COUNT(B.MissionID)
         , SUM(ISNULL(C.MissionScore, 0))
      FROM dbo.SS_AchieveMissions B WITH (NOLOCK)
           LEFT OUTER JOIN dbo.DimMissions C WITH (NOLOCK) ON (B.MissionID = C.MissionID)
     GROUP BY B.CharacterID
	OPTION (MAXDOP 1);

	/**_# [SS_AchieveMissionCounts] ???? ???? ?????. */
	IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_NC_AchieveMissionCounts')
		CREATE CLUSTERED INDEX IX_NC_AchieveMissionCounts ON dbo.SS_AchieveMissionCounts (CharacterID);

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
