USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCharacter2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 27
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-08-22
description : ???? ?????.
result set : 
	version 2 : AccountLevelCode/tinyint, CharacterName/nvarchar, VillageFirstVisitFlag/it, CharacterClassCode/tinyint, JobCode/tinyint, CharacterLevel/tinyint, CharacterExp/int, Coin/bigint, WarehouseCoin/bigint, HairColor/int, EyeColor/int, SkinColor/int, LastVillageGateNumber/tinyint, LastDarkLairVillageMapID/int, LastVillageMapID/int, LastMapID/int, PositionX/int, PositionY/int, PositionZ/int, Rotate/float, PCRoomFatigue/smallint, Fatigue/smallint, WeeklyFatigue/smallint, LastFatigueDate/smalldatetime, DefaultBody/int, DefaultLeg/int, DefaultHand/int, DefaultFoot/int, MissionScore/int, SkillPoint/smallint, SkillPoint2/smallint, AppellationID/int, CoverAppellationID/int, RecentAchieveMissionIDs/nvarchar, RebirthCoin/tinyint, PCRoomRebirthCoin/tinyint, LastRebirthCoinDate/smalldatetime, RebirthCoinByCash/smallint, DailyMissionAssignDate/datetime2, WeeklyMissionAssignDate/datetime2, GainMissionBin/binary, AchieveMissionBin/binary, GainAppellationBin/binary, TradeTotalCount/int, TradeSuccessCount/int, TradeFailCount/int, RegisterCommission/int, TradeCommission/int, GuildID/int, GuildName/nvarchar, GuildMemberRoleCode/tinyint, EventFatigue/smallint, EventFatigueGainDate/smalldatetime, VIPFatigue/smallint, LastQuestCheckDate/smalldatetime, LastConnectDate/datetime2, GuildWarScheduleID/smallint, TeamColorCode/tinyint, TimeEventDate/datetime2, GuildMark/smallint, GuildMarkBG/smallint, GuildMarkBorder/smallint, CurrentSkillCode/tinyint, ViewCashEquipmentBitmap/int
	version 3 : version2 - AchieveMissionBin/binary, GainAppellationBin/binary
	version 4 : version3 + PrivateChannelID/bigint
	version 5 : version4 + GuildJoinLevel/tinyint, GuildMaxLevelRewardFlag/bit
	version 6 : version5 + LastStampDate/datetime2
	version 7 : version6 + MonthlyMissionAssignDate/datetime2
	version 8 : version7 + TalismanSlotBitmap/int
	version 9 : version8 + LikeCount/int, DailyLikeCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
6 = (59407) version 3 ??. AchieveMissionBin, GainAppellationBin ???? ??? ?? by ??? at 2012-06-12
7 = version 4 ??. PrivateChannelID ????? ?? by ??? at 2012-07-11
10 = version 5 ??. GuildJoinLevel,GuildMaxLevelRewardFlag ????? ?? by ??? at 2012-09-03
15 = version 7 ??. MonthlyMissionAssignDate ????? ?? by ??? at 2012-12-11
16 = version 8 ??. TalismanSlotBitmap ????? ?? by ??? at 2012-12-24
17 = version 9 ??. LastLoginDate, LastLogoutDate, CheckSumBin ????? ?? by ??? at 2013-01-23
20 = version 9 ??. by ??? at 2013-01-30
21 = version 9 ??. HeroLevel, HeroExp ????? ?? by ??? at 2013-03-19
23 = 21? ?? by ??? at 2013-03-25
24 = version 9 ??. LikeCount ????? ?? by ??? at 2013-05-13
**/

CREATE PROCEDURE [dbo].[P_GetCharacter2]
	  @inbCharacterID bigint --// ??? ID
	, @inyLastServerType tinyint = 4 --// ???? ??? ??? ??
	, @intVersion int = 1 --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int
	, @dtmNow datetime = GETDATE();
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intVersion = 1
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate) AS (
			SELECT [1], [2]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin, dbo.FN_AchieveMissionBin(@inbCharacterID) AS AchieveMissionBin, dbo.FN_AppellationBin(@inbCharacterID) AS GainAppellationBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);

	ELSE IF @intVersion = 2
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate) AS (
			SELECT [1], [2]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint, CHS.SkillPoint2
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint, CHI.SkillPoint2
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_ChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_CoverChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK, INDEX(IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin, dbo.FN_AchieveMissionBin(@inbCharacterID) AS AchieveMissionBin, dbo.FN_AppellationBin(@inbCharacterID) AS GainAppellationBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);

	ELSE IF @intVersion = 3
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate) AS (
			SELECT [1], [2]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint, CHS.SkillPoint2
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint, CHI.SkillPoint2
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_ChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_CoverChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK, INDEX(IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);

	ELSE IF @intVersion = 4
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate) AS (
			SELECT [1], [2]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint, CHS.SkillPoint2
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap, CH.PrivateChannelID
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint, CHI.SkillPoint2
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_ChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_CoverChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK, INDEX(IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap, CHI.PrivateChannelID
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1			
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);

	ELSE IF @intVersion = 5
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate) AS (
			SELECT [1], [2]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint, CHS.SkillPoint2
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap, CH.PrivateChannelID, CHS.GuildMaxLevelRewardFlag
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint, CHI.SkillPoint2
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_ChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_CoverChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK, INDEX(IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap, CHI.PrivateChannelID, ISNULL(GM.CharacterJoinLevel,0) AS CharacterJoinLevel, ISNULL(CHI.GuildMaxLevelRewardFlag,0) AS GuildMaxLevelRewardFlag
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1			
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);

	ELSE IF @intVersion = 6
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate) AS (
			SELECT [1], [2]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint, CHS.SkillPoint2
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap, CH.PrivateChannelID, CHS.GuildMaxLevelRewardFlag, CHS.LastStampDate
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint, CHI.SkillPoint2
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_ChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_CoverChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK, INDEX(IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap, CHI.PrivateChannelID, ISNULL(GM.CharacterJoinLevel,0) AS CharacterJoinLevel, ISNULL(CHI.GuildMaxLevelRewardFlag,0) AS GuildMaxLevelRewardFlag, CHI.LastStampDate
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1			
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);

	ELSE IF @intVersion = 7
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate, MonthlyMissionAssignDate) AS (
			SELECT [1], [2], [8]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2], [8])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint, CHS.SkillPoint2
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap, CH.PrivateChannelID, CHS.GuildMaxLevelRewardFlag, CHS.LastStampDate
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint, CHI.SkillPoint2
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_ChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_CoverChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK, INDEX(IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap, CHI.PrivateChannelID, ISNULL(GM.CharacterJoinLevel,0) AS CharacterJoinLevel, ISNULL(CHI.GuildMaxLevelRewardFlag,0) AS GuildMaxLevelRewardFlag, CHI.LastStampDate
			, MD.MonthlyMissionAssignDate
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1			
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);

	ELSE IF @intVersion = 8
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate, MonthlyMissionAssignDate) AS (
			SELECT [1], [2], [8]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2], [8])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint, CHS.SkillPoint2
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap, CH.PrivateChannelID, CHS.GuildMaxLevelRewardFlag, CHS.LastStampDate
				, CHS.TalismanSlotBitmap
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint, CHI.SkillPoint2
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_ChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_CoverChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK, INDEX(IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap, CHI.PrivateChannelID, ISNULL(GM.CharacterJoinLevel,0) AS CharacterJoinLevel, ISNULL(CHI.GuildMaxLevelRewardFlag,0) AS GuildMaxLevelRewardFlag, CHI.LastStampDate
			, MD.MonthlyMissionAssignDate, CHI.TalismanSlotBitmap
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1			
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);

	ELSE IF @intVersion = 9
		WITH MissionAssignDates (DailyMissionAssignDate, WeeklyMissionAssignDate, MonthlyMissionAssignDate) AS (
			SELECT [1], [2], [8]
			FROM (
				SELECT MissionTypeCode, AssignDate
				FROM dbo.AssignMissions WITH (NOLOCK, INDEX(PK_AssignMissions), FORCESEEK)
				WHERE CharacterID = @inbCharacterID
			) S PIVOT (
				MAX(AssignDate) FOR MissionTypeCode IN ([1], [2], [8])
			) PV
		), CharacterInfo AS (
			SELECT CH.CharacterID, CH.AccountLevelCode, CH.CharacterName, CH.VillageFirstVisitFlag, CH.CharacterClassCode
				, CHS.JobCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.Coin, CHS.WarehouseCoin, CHS.HairColor, CHS.EyeColor, CHS.SkinColor
				, CHS.LastVillageGateNumber, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastMapID, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
				, CH.DefaultBody, CH.DefaultLeg, CH.DefaultHand, CH.DefaultFoot
				, CHS.LastFatigueDate, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.MissionScore, CHS.SkillPoint, CHS.SkillPoint2
				, CHS.LastRebirthCoinDate, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
				, MB.GainMissionBin
				, CHS.EventFatigue, CHS.EventFatigueGainDate, CHS.VIPFatigue, CHS.LastQuestCheckDate, CASE WHEN CHS.LastLogoutDate > CHS.LastLoginDate THEN CHS.LastLogoutDate ELSE CHS.LastLoginDate END AS LastConnectDate
				, CHS.TimeEventDate, CHS.CurrentSkillCode, CHS.ViewCashEquipmentBitmap, CH.PrivateChannelID, CHS.GuildMaxLevelRewardFlag, CHS.LastStampDate
				, CHS.TalismanSlotBitmap, CHS.LikeCount
			FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.MissionBinaries MB WITH (NOLOCK, INDEX(PK_MissionBinaries), FORCESEEK) ON CH.CharacterID = MB.CharacterID
			WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit)
		)
		SELECT CHI.AccountLevelCode, CHI.CharacterName, CHI.VillageFirstVisitFlag, CHI.CharacterClassCode, CHI.JobCode, CHI.CharacterLevel, CHI.CharacterExp, CHI.Coin, CHI.WarehouseCoin, CHI.HairColor, CHI.EyeColor, CHI.SkinColor
			, CHI.LastVillageGateNumber, CHI.LastDarkLairVillageMapID, CHI.LastVillageMapID, CHI.LastMapID, CHI.PositionX, CHI.PositionY, CHI.PositionZ, CHI.Rotate
			, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate
			, CHI.DefaultBody, CHI.DefaultLeg, CHI.DefaultHand, CHI.DefaultFoot, CHI.MissionScore, CHI.SkillPoint, CHI.SkillPoint2
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_ChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit)), -1) AS AppellationID
			, ISNULL((SELECT AppellationID FROM dbo.Appellations WITH (NOLOCK, INDEX(IX_UNF_Appellations_CharacterID_CoverChoiceFlag), FORCESEEK) WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit)), -1) AS CoverAppellationID
			, STUFF((SELECT N',' + CAST(MissionID AS nvarchar(10)) FROM (SELECT TOP (3) MissionID FROM dbo.AchieveMissions WITH (NOLOCK, INDEX(IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND MissionTypeCode >= 0 AND MissionTypeCode <= 1 ORDER BY AchieveDate DESC) AM FOR XML PATH(N'')), 1, 1, N'') AS RecentAchieveMissionIDs
			, RebirthCoin, PCRoomRebirthCoin, LastRebirthCoinDate
			, ISNULL((SELECT RemainCoinCount FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID), 0) AS RebirthCoinByCash, MD.DailyMissionAssignDate, MD.WeeklyMissionAssignDate
			, CHI.GainMissionBin
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID) AS TradeTotalCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS TradeSuccessCount
			, (SELECT ISNULL(CAST(SUM(TradeCount) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 3 AND 4) AS TradeFailCount
			, (SELECT ISNULL(CAST(SUM(RegisterCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2) AS RegisterCommission
			, (SELECT ISNULL(CAST(SUM(TradeCommission) AS int), 0) FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 1 AND 2)  AS TradeCommission
			, ISNULL(GM.GuildID, 0) AS GuildID, ISNULL(G.GuildName, N'') AS GuildName, ISNULL(GM.GuildMemberRoleCode, 0) AS GuildMemberRoleCode
			, CHI.EventFatigue, CHI.EventFatigueGainDate, CHI.VIPFatigue, CHI.LastQuestCheckDate, CHI.LastConnectDate
			, G.GuildWarScheduleID, G.TeamColorCode, CHI.TimeEventDate, G.GuildMark, G.GuildMarkBG, G.GuildMarkBorder, CHI.CurrentSkillCode, CHI.ViewCashEquipmentBitmap, CHI.PrivateChannelID, ISNULL(GM.CharacterJoinLevel,0) AS CharacterJoinLevel, ISNULL(CHI.GuildMaxLevelRewardFlag,0) AS GuildMaxLevelRewardFlag, CHI.LastStampDate
			, MD.MonthlyMissionAssignDate, CHI.TalismanSlotBitmap, CHI.LikeCount, DLC.LikeCount AS DailyLikeCount
		FROM CharacterInfo CHI
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CHI.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			LEFT OUTER JOIN MissionAssignDates MD ON 1 = 1	
			LEFT OUTER JOIN dbo.VIX_DailyLikeCount DLC WITH (NOLOCK) ON CHI.CharacterID = DLC.CharacterID					
		WHERE CHI.CharacterID = @inbCharacterID
		OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1, KEEPFIXED PLAN);
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END

	/**_# @inyLastServerType ?? ?????.*/
	UPDATE dbo.CharacterStatus
	SET LastServerType = @inyLastServerType
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);
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
