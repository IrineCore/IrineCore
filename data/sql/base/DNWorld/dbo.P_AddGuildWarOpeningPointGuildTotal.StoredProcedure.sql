USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarOpeningPointGuildTotal]    Script Date: 11/14/2026 8:54:37 AM ******/
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
    ??? ?? ??? ?? ?? ??
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    11 = ?? ??
	12 = RANK() ??, ???? ??     by 20110819.wgkim
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarOpeningPointGuildTotal]
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

    /**_# ??? ?? ??? ?? ?? ???? ?????. */
    TRUNCATE TABLE dbo.GuildWarOpeningPointGuildTotal;

    /**_# ??? ?? ???? ?????. */
    INSERT dbo.GuildWarOpeningPointGuildTotal (GuildWarMissionTypeCode, Ranking, GuildID, OpeningPointSum, TeamColorCode, RegisterDate)
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
         , GuildID
         , SUM(OpeningPoint)
         , TeamColorCode
         , GETDATE()
      FROM dbo.GuildWarOpeningPointRecord WITH (NOLOCK)
     GROUP BY CASE WHEN MissionID IN (500000, 500001, 500002, 500003, 500004, 500005, 500006, 500007) THEN 1
                   WHEN MissionID IN (500008) THEN 2
                   WHEN MissionID IN (500009) THEN 3
                   WHEN MissionID IN (500010) THEN 4 ELSE 5
              END
            , GuildID
            , TeamColorCode

    /**_# ??? ?? ??? ??? ?????. */
    INSERT dbo.GuildWarOpeningPointGuildTotal (GuildWarMissionTypeCode, Ranking, GuildID, OpeningPointSum, TeamColorCode, RegisterDate)
    SELECT 0
         , RANK() OVER(ORDER BY SUM(OpeningPoint) DESC)
         , GuildID
         , SUM(OpeningPoint)
         , TeamColorCode
         , GETDATE()
      FROM dbo.GuildWarOpeningPointRecord WITH (NOLOCK)
     GROUP BY GuildID, TeamColorCode

    --/**_# ?? ??? ?? ?? ??? ???. */
    --SELECT PT1.GuildWarMissionTypeCode
    --     , PT1.OpeningPointSum
    --     , MIN(PT1.Ranking) AS Ranking
    --  INTO #Temp_Revision
    --  FROM dbo.GuildWarOpeningPointGuildTotal PT1 WITH (NOLOCK)
    --       INNER JOIN dbo.GuildWarOpeningPointGuildTotal PT2 WITH (NOLOCK) ON (PT1.GuildWarMissionTypeCode = PT2.GuildWarMissionTypeCode AND PT1.OpeningPointSum = PT2.OpeningPointSum)
    -- WHERE PT1.Ranking <> PT2.Ranking
    -- GROUP BY PT1.GuildWarMissionTypeCode, PT1.OpeningPointSum

    --UPDATE dbo.GuildWarOpeningPointGuildTotal
    --   SET Ranking = TR.Ranking
    --  FROM dbo.GuildWarOpeningPointGuildTotal PT
    --       INNER JOIN #Temp_Revision TR ON (PT.GuildWarMissionTypeCode = TR.GuildWarMissionTypeCode AND PT.OpeningPointSum = TR.OpeningPointSum)

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
