USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelGuildWarOpeningPointRecord]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-11

description :
    ??? ?? ?? ?? ???
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    10 = ?? ??
	11 = ??? ?? ???? ??? ??? ?? ???     by 20110923.wgkim
	12 = ??? ??? ?? ?? ?? ?? ??? ??     by 20111005.wgkim
**/

CREATE PROCEDURE [dbo].[P_DelGuildWarOpeningPointRecord]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @dtmBackupDate datetime
      , @insGuildWarScheduleID smallint
      , @nvcSQL nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

    SET @dtmBackupDate = GETDATE();

	/**_# ??? ?? ??? ??? ?? ???. (??? ???? ????? ?? ? ?? ??) */
    INSERT dbo.GuildWarOpeningPointRecordBackup (SequenceNo, CharacterID, MissionID, OpeningPoint, GuildID, TeamColorCode, RegisterDate, BackupDate)
    SELECT SequenceNo, CharacterID, MissionID, OpeningPoint, GuildID, TeamColorCode, RegisterDate, @dtmBackupDate
      FROM dbo.GuildWarOpeningPointRecord WITH (NOLOCK);

	/**_# 2? ?? ?? ??? ?? ??? ?? ??? ?? ???. */
    DELETE dbo.GuildWarOpeningPointRecordBackup WHERE BackupDate <= DATEADD(DAY, -90, GETDATE());

	/**_# ??? ?? ??? ??? ??? ???. */
    IF OBJECT_ID(N'dbo.VIX_GuildWarOpeningPointForCharacter', N'V') IS NOT NULL
        DROP VIEW dbo.VIX_GuildWarOpeningPointForCharacter;

    IF OBJECT_ID(N'dbo.VIX_GuildWarOpeningPointForGuild', N'V') IS NOT NULL
        DROP VIEW dbo.VIX_GuildWarOpeningPointForGuild;

    IF OBJECT_ID(N'dbo.VIX_GuildWarOpeningPointForTeam', N'V') IS NOT NULL
        DROP VIEW dbo.VIX_GuildWarOpeningPointForTeam;

    TRUNCATE TABLE dbo.GuildWarOpeningPointRecord;

    SET @nvcSQL = N'CREATE VIEW dbo.VIX_GuildWarOpeningPointForCharacter
                    WITH SCHEMABINDING
                    AS
                    SELECT CharacterID, SUM(OpeningPoint) AS OpeningPoint, COUNT_BIG(*) AS Cnt
                    FROM dbo.GuildWarOpeningPointRecord
                    GROUP BY CharacterID;
                   '
    EXEC(@nvcSQL);

    SET @nvcSQL = N'CREATE UNIQUE CLUSTERED INDEX [IX_UC_VIX_GuildWarOpeningPointForCharacter] ON [dbo].[VIX_GuildWarOpeningPointForCharacter] ([CharacterID]) ON [IndexFG1]
                   '
    EXEC(@nvcSQL);

    SET @nvcSQL = N'CREATE VIEW dbo.VIX_GuildWarOpeningPointForGuild WITH SCHEMABINDING AS
                    SELECT GuildID, SUM(OpeningPoint) AS OpeningPoint, COUNT_BIG(*) AS Cnt
                    FROM dbo.GuildWarOpeningPointRecord
                    GROUP BY GuildID
                   '
    EXEC(@nvcSQL);

    SET @nvcSQL = N'CREATE UNIQUE CLUSTERED INDEX [IX_UC_VIX_GuildWarOpeningPointForGuild] ON [dbo].[VIX_GuildWarOpeningPointForGuild] ([GuildID]) ON [IndexFG1]
                   '
    EXEC(@nvcSQL);

    SET @nvcSQL = N'CREATE VIEW dbo.VIX_GuildWarOpeningPointForTeam
                    WITH SCHEMABINDING
                    AS
                    SELECT TeamColorCode, SUM(OpeningPoint) AS OpeningPoint, COUNT_BIG(*) AS Cnt
                    FROM dbo.GuildWarOpeningPointRecord
                    GROUP BY TeamColorCode;
                   '
    EXEC(@nvcSQL);

    SET @nvcSQL = N'CREATE UNIQUE CLUSTERED INDEX [IX_UC_VIX_GuildWarOpeningPointForTeam] ON [dbo].[VIX_GuildWarOpeningPointForTeam] ([TeamColorCode]) ON [IndexFG1]
                   '
    EXEC(@nvcSQL);

	/**_# ??? ?? ?? ?? ?? ??? ??? ???. */
    TRUNCATE TABLE dbo.GuildWarOpeningPointGuildRunningTotal;

	/**_# ??? ?? ??? ??? ?? ?? ??? ??? ???. */
    TRUNCATE TABLE dbo.GuildWarOpeningPointTotal;

	/**_# ??? ?? ??? ??? ?? ?? ??? ??? ???. */
    TRUNCATE TABLE dbo.GuildWarOpeningPointDailyTotal;

	/**_# ??? ?? ??? ??? ?? ?? ??? ??? ???. */
    TRUNCATE TABLE dbo.GuildWarOpeningPointGuildTotal;

	/**_# ??? ?? ?? ??? ??? ???. */
    TRUNCATE TABLE dbo.GuildWarPopularityVote;

	/**_# 2? ?? ?? ??? ?? ??? ?? ???. */
    SELECT @insGuildWarScheduleID = MAX(GuildWarScheduleID)
      FROM dbo.GuildWarSchedule WITH (NOLOCK)
     WHERE RewardEndDate <= DATEADD(DAY, -60, GETDATE());

    DELETE dbo.GuildWarRewardForCharacter WHERE GuildWarScheduleID <= @insGuildWarScheduleID;

    DELETE dbo.GuildWarRewardForGuild WHERE GuildWarScheduleID <= @insGuildWarScheduleID;

	/**_# ??? ?? ???? ??? ??? ??? ??? ???. */
    TRUNCATE TABLE dbo.GuildWarItemTradeRecord;

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
