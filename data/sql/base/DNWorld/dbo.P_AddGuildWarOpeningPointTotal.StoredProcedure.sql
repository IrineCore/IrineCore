USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarOpeningPointTotal]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-10

description :
    ??? ?? ??? ??? ?? ??
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    11 = ?? ??
	12 = RANK() ??, ???? ??     by 20110819.wgkim
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarOpeningPointTotal]
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

    /**_# ??? ?? ??? ??? ?? ???? ?????. */
    TRUNCATE TABLE dbo.GuildWarOpeningPointTotal;

    /**_# ??? ?? ???? ???? ?????. */
    INSERT dbo.GuildWarOpeningPointTotal (GuildWarMissionTypeCode, Ranking, CharacterID, OpeningPointSum, GuildID, RegisterDate)
    SELECT CASE WHEN MissionID IN (500000, 500001, 500002, 500003, 500004, 500005, 500006, 500007) THEN 1
                WHEN MissionID IN (500008) THEN 2
                WHEN MissionID IN (500009) THEN 3
                WHEN MissionID IN (500010) THEN 4 ELSE 5
           END AS GuildWarMissionTypeCode
         , ROW_NUMBER() OVER(PARTITION BY CASE WHEN MissionID IN (500000, 500001, 500002, 500003, 500004, 500005, 500006, 500007) THEN 1
                                               WHEN MissionID IN (500008) THEN 2
                                               WHEN MissionID IN (500009) THEN 3
                                               WHEN MissionID IN (500010) THEN 4 ELSE 5
                                          END ORDER BY SUM(OpeningPoint) DESC)
         , CharacterID
         , SUM(OpeningPoint)
         , GuildID
         , GETDATE()
      FROM dbo.GuildWarOpeningPointRecord WITH (NOLOCK)
     GROUP BY CASE WHEN MissionID IN (500000, 500001, 500002, 500003, 500004, 500005, 500006, 500007) THEN 1
                   WHEN MissionID IN (500008) THEN 2
                   WHEN MissionID IN (500009) THEN 3
                   WHEN MissionID IN (500010) THEN 4 ELSE 5
              END
            , CharacterID
            , GuildID

    /**_# ??? ?? ??? ??? ?????. (MissionID=0) */
    INSERT dbo.GuildWarOpeningPointTotal (GuildWarMissionTypeCode, Ranking, CharacterID, OpeningPointSum, GuildID, RegisterDate)
    SELECT 0
         , RANK() OVER(ORDER BY SUM(OpeningPoint) DESC)
         , CharacterID
         , SUM(OpeningPoint)
         , GuildID
         , GETDATE()
      FROM dbo.GuildWarOpeningPointRecord WITH (NOLOCK)
     GROUP BY CharacterID, GuildID

    --/**_# ???? ?? ?? ?? ??? ???. */
    --SELECT PT1.GuildWarMissionTypeCode
    --     , PT1.OpeningPointSum
    --     , MIN(PT1.Ranking) AS Ranking
    --  INTO #Temp_Revision
    --  FROM dbo.GuildWarOpeningPointTotal PT1 WITH (NOLOCK)
    --       INNER JOIN dbo.GuildWarOpeningPointTotal PT2 WITH (NOLOCK) ON (PT1.GuildWarMissionTypeCode = PT2.GuildWarMissionTypeCode AND PT1.OpeningPointSum = PT2.OpeningPointSum)
    -- WHERE PT1.Ranking <> PT2.Ranking
    -- GROUP BY PT1.GuildWarMissionTypeCode, PT1.OpeningPointSum

    --UPDATE dbo.GuildWarOpeningPointTotal
    --   SET Ranking = TR.Ranking
    --  FROM dbo.GuildWarOpeningPointTotal PT
    --       INNER JOIN #Temp_Revision TR ON (PT.GuildWarMissionTypeCode = TR.GuildWarMissionTypeCode AND PT.OpeningPointSum = TR.OpeningPointSum)

    /**_# ??? ?? ???? ?? ???? ???? ???. */
    UPDATE dbo.GuildWarSchedule
       SET WinningTeamColorCode = (SELECT TOP 1 TeamColorCode
                                     FROM dbo.VIX_GuildWarOpeningPointForTeam WITH (NOEXPAND, NOLOCK)
                                    ORDER BY OpeningPoint DESC
                                  )
     WHERE ApplyStartDate < GETDATE()
       AND RewardEndDate > GETDATE()

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
