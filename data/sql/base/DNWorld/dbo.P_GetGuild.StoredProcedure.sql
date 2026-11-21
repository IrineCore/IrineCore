USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuild]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-27
description : ?? ??? ?????.
result set : 
ver.8 = GuildName/nvarchar, GuildLevel/tinyint, NoticeContent/nvarchar, GuildCoin/bigint, RegisterDate/datetime2, Privilege1/int, DailyTakeItemLimit1/int, DailyTakeCoinLimit1/int, Privilege2/int, DailyTakeItemLimit2/int, DailyTakeCoinLimit2/int, Privilege3/int, DailyTakeItemLimit3/int, DailyTakeCoinLimit3/int, Privilege4/int, DailyTakeItemLimit4/int, DailyTakeCoinLimit4/int, GuildDepotSize/smallint, GuildWarScheduleID/smallint, TeamColorCode/tinyint, GuildMarkBG/smallint, GuildMarkBorder/smallint, GuildMark/smallint, GuildWarFinalCount/smallint, GuildWarFinalWinCount/smallint, TotalGuildEXP/int, DailyStagePoint/int, DailyMissionPoint/int, GuildMaxMemberCount/int
ver.8 = GuildName/nvarchar, GuildLevel/tinyint, NoticeContent/nvarchar, GuildCoin/bigint, RegisterDate/datetime2, Privilege1/int, DailyTakeItemLimit1/int, DailyTakeCoinLimit1/int, Privilege2/int, DailyTakeItemLimit2/int, DailyTakeCoinLimit2/int, Privilege3/int, DailyTakeItemLimit3/int, DailyTakeCoinLimit3/int, Privilege4/int, DailyTakeItemLimit4/int, DailyTakeCoinLimit4/int, GuildDepotSize/smallint, GuildWarScheduleID/smallint, TeamColorCode/tinyint, GuildMarkBG/smallint, GuildMarkBorder/smallint, GuildMark/smallint, GuildWarFinalCount/smallint, GuildWarFinalWinCount/smallint, TotalGuildEXP/int, DailyStagePoint/int, DailyMissionPoint/int, GuildMaxMemberCount/int, GuildHomepageURL/nvarchar(max)
ver.12 = GuildName/nvarchar, GuildLevel/tinyint, NoticeContent/nvarchar, GuildCoin/bigint, RegisterDate/datetime2, Privilege1/int, DailyTakeItemLimit1/int, DailyTakeCoinLimit1/int, Privilege2/int, DailyTakeItemLimit2/int, DailyTakeCoinLimit2/int, Privilege3/int, DailyTakeItemLimit3/int, DailyTakeCoinLimit3/int, Privilege4/int, DailyTakeItemLimit4/int, DailyTakeCoinLimit4/int, GuildDepotSize/smallint, GuildWarScheduleID/smallint, TeamColorCode/tinyint, GuildMarkBG/smallint, GuildMarkBorder/smallint, GuildMark/smallint, GuildWarFinalCount/smallint, GuildWarFinalWinCount/smallint, TotalGuildEXP/int, DailyStagePoint/int, DailyMissionPoint/int, GuildMaxMemberCount/int, GuildHomepageURL/nvarchar(max), GuildTypeCode/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103238 = ??? ???? ????.
history :
4 = ?? ?? ?? (GuildDepotSize)? ????? ?? at 2010-11-03
5 = [???-????] ???? SELECT ?? ?? (?????/TeamColor) - 20110111.wgkim
6 = ?? ?? ?? (GuildMarkBG, GuildMarkBorder, GuildMark) at 2011-04-05
7 = ??? ?? result set ?? (@intVersion = 7)     by 2011049.wgkim
8 = ?? ??? ?? result set ?? (@intVersion = 8)     by ??? at 2011-11-24
10 = ?? ?? ?? ??? ?? result set ?? (@intVersion = 8)     by ??? at 2011-11-30
11 = result set ?? (?? ????)     by ??? at 2012-05-24
**/

CREATE PROCEDURE [dbo].[P_GetGuild]
	  @intGuildID int --// ?? ID
	, @intVersion int = 5 --// ??? SP? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @insGuildWarFinalCount smallint
	  , @insGuildWarFinalWinCount smallint;

DECLARE @dt2Today datetime2(0) = CAST(GETDATE() AS date);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Guilds] ???? SELECT???.*/
	IF @intVersion = 5
		WITH C1 AS (
			SELECT T.n AS GuildMemberRoleCode
				, ISNULL(GMR.Privilege, -1) AS Privilege
				, ISNULL(GMR.DailyTakeItemLimit, -1) AS DailyTakeItemLimit
				, ISNULL(GMR.DailyTakeCoinLimit, -1) AS DailyTakeCoinLimit
			FROM dbo.Tally T WITH (NOLOCK)
				LEFT OUTER JOIN dbo.GuildMemberRoles GMR WITH (NOLOCK) ON CAST(T.n AS tinyint) = GMR.GuildMemberRoleCode AND GMR.GuildID = @intGuildID
			WHERE T.n BETWEEN 1 AND 4
		), C2 AS (
			SELECT MAX(CASE GuildMemberRoleCode WHEN 1 THEN Privilege END) AS Privilege1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN Privilege END) AS Privilege2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN Privilege END) AS Privilege3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN Privilege END) AS Privilege4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit4
			FROM C1
		)
		SELECT G.GuildName, G.GuildLevel, G.TotalGuildPoint, G.RemainGuildPoint, G.NoticeContent, G.GuildCoin, G.RegisterDate
			, C2.Privilege1, C2.DailyTakeItemLimit1, C2.DailyTakeCoinLimit1
			, C2.Privilege2, C2.DailyTakeItemLimit2, C2.DailyTakeCoinLimit2
			, C2.Privilege3, C2.DailyTakeItemLimit3, C2.DailyTakeCoinLimit3
			, C2.Privilege4, C2.DailyTakeItemLimit4, C2.DailyTakeCoinLimit4
			, G.GuildDepotSize
			, G.GuildWarScheduleID
			, G.TeamColorCode
		FROM dbo.Guilds G WITH (NOLOCK)
			CROSS JOIN C2
		WHERE G.GuildID = @intGuildID AND G.CloseFlag = CAST(0 AS bit);

	ELSE IF @intVersion = 6
		WITH C1 AS (
			SELECT T.n AS GuildMemberRoleCode
				, ISNULL(GMR.Privilege, -1) AS Privilege
				, ISNULL(GMR.DailyTakeItemLimit, -1) AS DailyTakeItemLimit
				, ISNULL(GMR.DailyTakeCoinLimit, -1) AS DailyTakeCoinLimit
			FROM dbo.Tally T WITH (NOLOCK)
				LEFT OUTER JOIN dbo.GuildMemberRoles GMR WITH (NOLOCK) ON CAST(T.n AS tinyint) = GMR.GuildMemberRoleCode AND GMR.GuildID = @intGuildID
			WHERE T.n BETWEEN 1 AND 4
		), C2 AS (
			SELECT MAX(CASE GuildMemberRoleCode WHEN 1 THEN Privilege END) AS Privilege1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN Privilege END) AS Privilege2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN Privilege END) AS Privilege3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN Privilege END) AS Privilege4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit4
			FROM C1
		)
		SELECT G.GuildName, G.GuildLevel, G.TotalGuildPoint, G.RemainGuildPoint, G.NoticeContent, G.GuildCoin, G.RegisterDate
			, C2.Privilege1, C2.DailyTakeItemLimit1, C2.DailyTakeCoinLimit1
			, C2.Privilege2, C2.DailyTakeItemLimit2, C2.DailyTakeCoinLimit2
			, C2.Privilege3, C2.DailyTakeItemLimit3, C2.DailyTakeCoinLimit3
			, C2.Privilege4, C2.DailyTakeItemLimit4, C2.DailyTakeCoinLimit4
			, G.GuildDepotSize
			, G.GuildWarScheduleID
			, G.TeamColorCode
			, G.GuildMarkBG
			, G.GuildMarkBorder
			, G.GuildMark
		FROM dbo.Guilds G WITH (NOLOCK)
			CROSS JOIN C2
		WHERE G.GuildID = @intGuildID AND G.CloseFlag = CAST(0 AS bit);

	ELSE IF @intVersion = 7
	BEGIN

		SET @insGuildWarFinalCount = 0;		-- ??? ?? ?? ??
		SET @insGuildWarFinalWinCount = 0;	-- ??? ?? ?? ??

		SELECT @insGuildWarFinalCount = @insGuildWarFinalCount + 1
			 , @insGuildWarFinalWinCount = @insGuildWarFinalWinCount + CASE WHEN MatchTypeCode = 1 AND MatchResultCode = 1 THEN 1 ELSE 0 END
		  FROM dbo.GuildWarFinalResults WITH (NOLOCK)
		 WHERE GuildID = @intGuildID;

		WITH C1 AS (
			SELECT T.n AS GuildMemberRoleCode
				, ISNULL(GMR.Privilege, -1) AS Privilege
				, ISNULL(GMR.DailyTakeItemLimit, -1) AS DailyTakeItemLimit
				, ISNULL(GMR.DailyTakeCoinLimit, -1) AS DailyTakeCoinLimit
			FROM dbo.Tally T WITH (NOLOCK)
				LEFT OUTER JOIN dbo.GuildMemberRoles GMR WITH (NOLOCK) ON CAST(T.n AS tinyint) = GMR.GuildMemberRoleCode AND GMR.GuildID = @intGuildID
			WHERE T.n BETWEEN 1 AND 4
		), C2 AS (
			SELECT MAX(CASE GuildMemberRoleCode WHEN 1 THEN Privilege END) AS Privilege1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN Privilege END) AS Privilege2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN Privilege END) AS Privilege3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN Privilege END) AS Privilege4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit4
			FROM C1
		)
		SELECT G.GuildName, G.GuildLevel, G.TotalGuildPoint, G.RemainGuildPoint, G.NoticeContent, G.GuildCoin, G.RegisterDate
			, C2.Privilege1, C2.DailyTakeItemLimit1, C2.DailyTakeCoinLimit1
			, C2.Privilege2, C2.DailyTakeItemLimit2, C2.DailyTakeCoinLimit2
			, C2.Privilege3, C2.DailyTakeItemLimit3, C2.DailyTakeCoinLimit3
			, C2.Privilege4, C2.DailyTakeItemLimit4, C2.DailyTakeCoinLimit4
			, G.GuildDepotSize
			, G.GuildWarScheduleID
			, G.TeamColorCode
			, G.GuildMarkBG
			, G.GuildMarkBorder
			, G.GuildMark
			, @insGuildWarFinalCount AS GuildWarFinalCount
			, @insGuildWarFinalWinCount AS GuildWarFinalWinCount
		FROM dbo.Guilds G WITH (NOLOCK)
			CROSS JOIN C2
		WHERE G.GuildID = @intGuildID AND G.CloseFlag = CAST(0 AS bit);

	END
	ELSE IF @intVersion = 8
	BEGIN

		SET @insGuildWarFinalCount = 0;		-- ??? ?? ?? ??
		SET @insGuildWarFinalWinCount = 0;	-- ??? ?? ?? ??

		SELECT @insGuildWarFinalCount = @insGuildWarFinalCount + 1
			 , @insGuildWarFinalWinCount = @insGuildWarFinalWinCount + CASE WHEN MatchTypeCode = 1 AND MatchResultCode = 1 THEN 1 ELSE 0 END
		  FROM dbo.GuildWarFinalResults WITH (NOLOCK)
		 WHERE GuildID = @intGuildID;

		WITH C1 AS (
			SELECT T.n AS GuildMemberRoleCode
				, ISNULL(GMR.Privilege, -1) AS Privilege
				, ISNULL(GMR.DailyTakeItemLimit, -1) AS DailyTakeItemLimit
				, ISNULL(GMR.DailyTakeCoinLimit, -1) AS DailyTakeCoinLimit
			FROM dbo.Tally T WITH (NOLOCK)
				LEFT OUTER JOIN dbo.GuildMemberRoles GMR WITH (NOLOCK) ON CAST(T.n AS tinyint) = GMR.GuildMemberRoleCode AND GMR.GuildID = @intGuildID
			WHERE T.n BETWEEN 1 AND 4
		), C2 AS (
			SELECT MAX(CASE GuildMemberRoleCode WHEN 1 THEN Privilege END) AS Privilege1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN Privilege END) AS Privilege2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN Privilege END) AS Privilege3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN Privilege END) AS Privilege4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit4
			FROM C1
		)
		SELECT G.GuildName, G.GuildLevel, G.NoticeContent, G.GuildCoin, G.RegisterDate
			, C2.Privilege1, C2.DailyTakeItemLimit1, C2.DailyTakeCoinLimit1
			, C2.Privilege2, C2.DailyTakeItemLimit2, C2.DailyTakeCoinLimit2
			, C2.Privilege3, C2.DailyTakeItemLimit3, C2.DailyTakeCoinLimit3
			, C2.Privilege4, C2.DailyTakeItemLimit4, C2.DailyTakeCoinLimit4
			, G.GuildDepotSize
			, G.GuildWarScheduleID
			, G.TeamColorCode
			, G.GuildMarkBG
			, G.GuildMarkBorder
			, G.GuildMark
			, @insGuildWarFinalCount AS GuildWarFinalCount
			, @insGuildWarFinalWinCount AS GuildWarFinalWinCount
			, G.TotalGuildEXP
			, CASE WHEN G.StagePointLastModDate >= @dt2Today THEN G.DailyStagePoint ELSE 0 END AS DailyStagePoint
			, CASE WHEN G.MissionPointLastModDate >= @dt2Today THEN G.DailyMissionPoint ELSE 0 END AS DailyMissionPoint
			, G.GuildMaxMemberCount
		FROM dbo.Guilds G WITH (NOLOCK)
			CROSS JOIN C2
		WHERE G.GuildID = @intGuildID AND G.CloseFlag = CAST(0 AS bit);

	END
	ELSE IF @intVersion = 11
	BEGIN

		SET @insGuildWarFinalCount = 0;		-- ??? ?? ?? ??
		SET @insGuildWarFinalWinCount = 0;	-- ??? ?? ?? ??

		SELECT @insGuildWarFinalCount = @insGuildWarFinalCount + 1
			 , @insGuildWarFinalWinCount = @insGuildWarFinalWinCount + CASE WHEN MatchTypeCode = 1 AND MatchResultCode = 1 THEN 1 ELSE 0 END
		  FROM dbo.GuildWarFinalResults WITH (NOLOCK)
		 WHERE GuildID = @intGuildID;

		WITH C1 AS (
			SELECT T.n AS GuildMemberRoleCode
				, ISNULL(GMR.Privilege, -1) AS Privilege
				, ISNULL(GMR.DailyTakeItemLimit, -1) AS DailyTakeItemLimit
				, ISNULL(GMR.DailyTakeCoinLimit, -1) AS DailyTakeCoinLimit
			FROM dbo.Tally T WITH (NOLOCK)
				LEFT OUTER JOIN dbo.GuildMemberRoles GMR WITH (NOLOCK) ON CAST(T.n AS tinyint) = GMR.GuildMemberRoleCode AND GMR.GuildID = @intGuildID
			WHERE T.n BETWEEN 1 AND 4
		), C2 AS (
			SELECT MAX(CASE GuildMemberRoleCode WHEN 1 THEN Privilege END) AS Privilege1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN Privilege END) AS Privilege2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN Privilege END) AS Privilege3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN Privilege END) AS Privilege4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit4
			FROM C1
		)
		SELECT G.GuildName, G.GuildLevel, G.NoticeContent, G.GuildCoin, G.RegisterDate
			, C2.Privilege1, C2.DailyTakeItemLimit1, C2.DailyTakeCoinLimit1
			, C2.Privilege2, C2.DailyTakeItemLimit2, C2.DailyTakeCoinLimit2
			, C2.Privilege3, C2.DailyTakeItemLimit3, C2.DailyTakeCoinLimit3
			, C2.Privilege4, C2.DailyTakeItemLimit4, C2.DailyTakeCoinLimit4
			, G.GuildDepotSize
			, G.GuildWarScheduleID
			, G.TeamColorCode
			, G.GuildMarkBG
			, G.GuildMarkBorder
			, G.GuildMark
			, @insGuildWarFinalCount AS GuildWarFinalCount
			, @insGuildWarFinalWinCount AS GuildWarFinalWinCount
			, G.TotalGuildEXP
			, CASE WHEN G.StagePointLastModDate >= @dt2Today THEN G.DailyStagePoint ELSE 0 END AS DailyStagePoint
			, CASE WHEN G.MissionPointLastModDate >= @dt2Today THEN G.DailyMissionPoint ELSE 0 END AS DailyMissionPoint
			, G.GuildMaxMemberCount
			, G.GuildHomepageURL
		FROM dbo.Guilds G WITH (NOLOCK)
			CROSS JOIN C2
		WHERE G.GuildID = @intGuildID AND G.CloseFlag = CAST(0 AS bit);

	END
	ELSE IF @intVersion = 12
	BEGIN

		SET @insGuildWarFinalCount = 0;		-- ??? ?? ?? ??
		SET @insGuildWarFinalWinCount = 0;	-- ??? ?? ?? ??

		SELECT @insGuildWarFinalCount = @insGuildWarFinalCount + 1
			 , @insGuildWarFinalWinCount = @insGuildWarFinalWinCount + CASE WHEN MatchTypeCode = 1 AND MatchResultCode = 1 THEN 1 ELSE 0 END
		  FROM dbo.GuildWarFinalResults WITH (NOLOCK)
		 WHERE GuildID = @intGuildID;

		WITH C1 AS (
			SELECT T.n AS GuildMemberRoleCode
				, ISNULL(GMR.Privilege, -1) AS Privilege
				, ISNULL(GMR.DailyTakeItemLimit, -1) AS DailyTakeItemLimit
				, ISNULL(GMR.DailyTakeCoinLimit, -1) AS DailyTakeCoinLimit
			FROM dbo.Tally T WITH (NOLOCK)
				LEFT OUTER JOIN dbo.GuildMemberRoles GMR WITH (NOLOCK) ON CAST(T.n AS tinyint) = GMR.GuildMemberRoleCode AND GMR.GuildID = @intGuildID
			WHERE T.n BETWEEN 1 AND 4
		), C2 AS (
			SELECT MAX(CASE GuildMemberRoleCode WHEN 1 THEN Privilege END) AS Privilege1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 1 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit1
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN Privilege END) AS Privilege2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 2 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit2
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN Privilege END) AS Privilege3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 3 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit3
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN Privilege END) AS Privilege4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeItemLimit END) AS DailyTakeItemLimit4
				, MAX(CASE GuildMemberRoleCode WHEN 4 THEN DailyTakeCoinLimit END) AS DailyTakeCoinLimit4
			FROM C1
		)
		SELECT G.GuildName, G.GuildLevel, G.NoticeContent, G.GuildCoin, G.RegisterDate
			, C2.Privilege1, C2.DailyTakeItemLimit1, C2.DailyTakeCoinLimit1
			, C2.Privilege2, C2.DailyTakeItemLimit2, C2.DailyTakeCoinLimit2
			, C2.Privilege3, C2.DailyTakeItemLimit3, C2.DailyTakeCoinLimit3
			, C2.Privilege4, C2.DailyTakeItemLimit4, C2.DailyTakeCoinLimit4
			, G.GuildDepotSize
			, G.GuildWarScheduleID
			, G.TeamColorCode
			, G.GuildMarkBG
			, G.GuildMarkBorder
			, G.GuildMark
			, @insGuildWarFinalCount AS GuildWarFinalCount
			, @insGuildWarFinalWinCount AS GuildWarFinalWinCount
			, G.TotalGuildEXP
			, CASE WHEN G.StagePointLastModDate >= @dt2Today THEN G.DailyStagePoint ELSE 0 END AS DailyStagePoint
			, CASE WHEN G.MissionPointLastModDate >= @dt2Today THEN G.DailyMissionPoint ELSE 0 END AS DailyMissionPoint
			, G.GuildMaxMemberCount
			, G.GuildHomepageURL
			, G.GuildTypeCode
		FROM dbo.Guilds G WITH (NOLOCK)
			CROSS JOIN C2
		WHERE G.GuildID = @intGuildID AND G.CloseFlag = CAST(0 AS bit);

	END


	/**_## ??? ???? ??? ?????? 103238? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103238;
		GOTO ErrorHandler;
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
