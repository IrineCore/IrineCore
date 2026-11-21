USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarOpeningPointDailyTotal]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
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
    4 = ?? ??
	5 = RANK() ??, ???? ??     by 20110819.wgkim
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarOpeningPointDailyTotal]
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
    TRUNCATE TABLE dbo.GuildWarOpeningPointDailyTotal;

    /**_# ??? ?? ???? ???? ?????. */
    INSERT dbo.GuildWarOpeningPointDailyTotal (BasisDate, Ranking, CharacterID, OpeningPointSum, GuildID, RegisterDate)
    SELECT CONVERT(DATE, RegisterDate) AS RegisterDate
         , RANK() OVER(PARTITION BY CONVERT(DATE, RegisterDate) ORDER BY SUM(OpeningPoint) DESC)
         , CharacterID
         , SUM(OpeningPoint)
         , GuildID
         , GETDATE()
      FROM dbo.GuildWarOpeningPointRecord WITH (NOLOCK)
     GROUP BY CONVERT(DATE, RegisterDate), CharacterID, GuildID

    --/**_# ???? ?? ?? ??? ???. */
    --SELECT PT1.BasisDate
    --     , PT1.OpeningPointSum
    --     , MIN(PT1.Ranking) AS Ranking
    --  INTO #Temp_Revision
    --  FROM dbo.GuildWarOpeningPointDailyTotal PT1 WITH (NOLOCK)
    --       INNER JOIN dbo.GuildWarOpeningPointDailyTotal PT2 WITH (NOLOCK) ON (PT1.BasisDate = PT2.BasisDate AND PT1.OpeningPointSum = PT2.OpeningPointSum)
    -- WHERE PT1.Ranking <> PT2.Ranking
    -- GROUP BY PT1.BasisDate, PT1.OpeningPointSum

    --UPDATE dbo.GuildWarOpeningPointDailyTotal
    --   SET Ranking = TR.Ranking
    --  FROM dbo.GuildWarOpeningPointDailyTotal PT
    --       INNER JOIN #Temp_Revision TR ON (PT.BasisDate = TR.BasisDate AND PT.OpeningPointSum = TR.OpeningPointSum)

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
