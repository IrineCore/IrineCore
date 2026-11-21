USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListWeeklyGuildContributionPointRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-11
description : ?? ????? ??? ?????.
result set : Ranking/int, CharacterID/bigint, CharacterName/nvarchar, JobCode/tiyint, WeeklyGuildContributionPoint/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListWeeklyGuildContributionPointRanking]
	  @intGuildID int --// ??ID	
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
	/**_# [WeeklyGuildContributionPoint] ???? SELECT ???.*/
	SELECT RANK() OVER(ORDER BY WG.WeeklyGuildContributionPoint DESC) AS Ranking, WG.CharacterID, CH.CharacterName, CS.JobCode, WG.WeeklyGuildContributionPoint
	FROM dbo.WeeklyGuildContributionPoint WG WITH (NOLOCK, INDEX(PK_WeeklyGuildContributionPoint)) 
		 INNER JOIN dbo.Characters CH  WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON WG.CharacterID = CH.CharacterID
		 INNER JOIN dbo.CharacterStatus CS  WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CS.CharacterID
	WHERE WG.GuildID = @intGuildID;

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
