USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCharacterPartialy]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 25
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-19
description : ??? ID ?? ??? ???? ???? ?? ??? ?????.
result set : 
	@inySelectClauseCode
	1 : CharacterID/bigint, CharacterClassCode/tinyint, CharacterLevel/tinyint, JobCode/tinyint ?? : @nvcCharacterName?? ?????.
	2 : Coin/bigint, WarehouseCoin/bigint ?? : @inbCharacterID? ?????.
	3 : AccountID, AccountName, CharacterName, JobCode, AccountLevelCode, CharacterLevel, CharacterExp, SkillPoint, Coin, RebirthCoin, PCRoomRebirthCoin, LastMapID, LastDarkLairVillageMapID, LastVillageMapID, LastVillageGateNumber, PositionX, PositionY, PositionZ, Rotate, LastLoginDate, LastLogoutDate, GuildName, DeleteFlag, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate, VIPFatigue
	4 : CharacterLevel/tinyint, CharacterClassCode/tinyint
	5 : AccountID, AccountName, CharacterName, JobCode, AccountLevelCode, CharacterLevel, CharacterExp, SkillPoint, Coin, RebirthCoin, PCRoomRebirthCoin, LastMapID, LastDarkLairVillageMapID, LastVillageMapID, LastVillageGateNumber, PositionX, PositionY, PositionZ, Rotate, LastLoginDate, LastLogoutDate, GuildName, DeleteFlag, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate, LastLoginIP, VIPFatigue
	6 : CharacterLevel/tinyint, JobCode/tinyint
	7 : AccountID/int, CharacterID/bigint
	8 : WorldID/int
	9 : AccountID, AccountName, CharacterName, JobCode, AccountLevelCode, CharacterLevel, CharacterExp, SkillPoint, Coin, RebirthCoin, PCRoomRebirthCoin, LastMapID, LastDarkLairVillageMapID, LastVillageMapID, LastVillageGateNumber, PositionX, PositionY, PositionZ, Rotate, LastLoginDate, LastLogoutDate, GuildName, DeleteFlag, PCRoomFatigue, Fatigue, WeeklyFatigue, LastFatigueDate, LastLoginIP, VIPFatigue, CurrentSkillCode
	10 : AccountID/int, AccountName/nvarchar, CharacterID/bigint
	11 : TalismanSlotBitmap/int
	12 : PVPWin/int, PVPLose/int, PVPDraw/int, PVPExp/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
8 = @inySelectClauseCode ?? 6? ?? ?? ??
10 = @inySelectClauseCode ?? 6? ??, @nvcCharacterName?? ??? ??
11 = @inySelectClauseCode ?? 7? ?? ?? ??
12 = @inySelectClauseCode ?? 3, 5 ??, VIP ??? ????? ??
13 = @inySelectClauseCode ?? 8? ?? ?? ??
14 = input param. @intVersion ?? at 2010-12-02
15 = @inySelectClauseCode ?? 9? ?? ?? ??
16 = @inySelectClauseCode ?? 10? ?? ?? ??
17 = @inySelectClauseCode ??9 ? ?? @intVersion ??     by 20110530.wgkim
18 = ????? ??? ?? ?? ??     by 20110603.wgkim
21 = CharacterID? ?? ??? ? ????? ????? ???? ???? ????? ???? ?? ?? ?? ?? by ??? at 2011-11-14
22 = DormantCharacterStatus ??? ?? ?? by ??? at 2011-11-30
23 = ???? ?? ??? ??? ?? ?? by ??? at 2011-12-29
24 = ???? ?? ?? ?? by ??? at 2013-01-07
25 = PVP ?? ?? ?? by ??? at 2013-03-14
**/

CREATE PROCEDURE [dbo].[P_GetCharacterPartialy]
	  @inySelectClauseCode tinyint --// ??? ?? ? ??
	, @inbCharacterID bigint --// (optional) ??? ID
	, @nvcCharacterName nvarchar(30) --// (optional) ??? ??
	, @intVersion int = 13 --// ??? SP? ?? (?? ?? ? 14? ??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inyDormantType tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inySelectClauseCode = 1
		SELECT CH.CharacterID, CH.CharacterClassCode, CHS.CharacterLevel, CHS.JobCode
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
		WHERE CH.CharacterName = @nvcCharacterName AND CH.DeleteFlag = CAST(0 AS bit);

	ELSE IF @inySelectClauseCode = 2
		SELECT CHS.Coin, CHS.WarehouseCoin
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
		WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit);

	ELSE IF @inySelectClauseCode = 3
	BEGIN
		SELECT CH.AccountID, CH.AccountName, CH.CharacterName, CHS.JobCode, CH.AccountLevelCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.SkillPoint, CHS.Coin, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
			, CHS.LastMapID, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastVillageGateNumber, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
			, CHS.LastLoginDate, CHS.LastLogoutDate, G.GuildName, CH.DeleteFlag
			, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.LastFatigueDate, CHS.VIPFatigue
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON CH.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK) ON GM.GuildID = G.GuildID
		WHERE CH.CharacterID = @inbCharacterID;
	END

	ELSE IF @inySelectClauseCode = 4
		SELECT CHS.CharacterLevel, CH.CharacterClassCode
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
		WHERE CH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit);

	ELSE IF @inySelectClauseCode = 5
		SELECT CH.AccountID, CH.AccountName, CH.CharacterName, CHS.JobCode, CH.AccountLevelCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.SkillPoint, CHS.Coin, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
			, CHS.LastMapID, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastVillageGateNumber, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
			, CHS.LastLoginDate, CHS.LastLogoutDate, G.GuildName, CH.DeleteFlag
			, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.LastFatigueDate, dbo.FN_IPNumber2IP(CHS.LastLoginIP) AS LastLoginIP, CHS.VIPFatigue
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
			LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON CH.CharacterID = GM.CharacterID
			LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK) ON GM.GuildID = G.GuildID
		WHERE CH.CharacterID = @inbCharacterID;

	ELSE IF @inySelectClauseCode = 6
		SELECT CHS.CharacterLevel, CHS.JobCode
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
		WHERE CH.CharacterName = @nvcCharacterName AND CH.DeleteFlag = CAST(0 AS bit);

	ELSE IF @inySelectClauseCode = 7
		SELECT CH.AccountID, CH.CharacterID
		FROM dbo.Characters CH WITH (NOLOCK)
		WHERE CH.CharacterName = @nvcCharacterName AND CH.DeleteFlag = CAST(0 AS bit);

	ELSE IF @inySelectClauseCode = 8
		SELECT WorldID
		FROM dbo.Characters WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

	ELSE IF @inySelectClauseCode = 9
		SELECT CH.AccountID, CH.AccountName, CH.CharacterName, CHS.JobCode, CH.AccountLevelCode, CHS.CharacterLevel, CHS.CharacterExp, CHS.SkillPoint, CHS.Coin, CHS.RebirthCoin, CHS.PCRoomRebirthCoin
			, CHS.LastMapID, CHS.LastDarkLairVillageMapID, CHS.LastVillageMapID, CHS.LastVillageGateNumber, CHS.PositionX, CHS.PositionY, CHS.PositionZ, CHS.Rotate
			, CHS.LastLoginDate, CHS.LastLogoutDate, G.GuildName AS GuildName, CH.DeleteFlag
			, CHS.PCRoomFatigue, CHS.Fatigue, CHS.WeeklyFatigue, CHS.LastFatigueDate, dbo.FN_IPNumber2IP(CHS.LastLoginIP) AS LastLoginIP, CHS.VIPFatigue, CHS.CurrentSkillCode
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
				LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON CH.CharacterID = GM.CharacterID
				LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK) ON GM.GuildID = G.GuildID
		WHERE CH.CharacterID = @inbCharacterID;

	ELSE IF @inySelectClauseCode = 10
		SELECT CH.AccountID, CH.AccountName, CH.CharacterID
		FROM dbo.Characters CH WITH (NOLOCK)
		WHERE CH.CharacterName = @nvcCharacterName AND CH.DeleteFlag = CAST(0 AS bit);

	ELSE IF @inySelectClauseCode = 11
		SELECT CS.TalismanSlotBitmap
		FROM dbo.CharacterStatus CS WITH (NOLOCK)
		WHERE CS.CharacterID = @inbCharacterID;

	ELSE IF @inySelectClauseCode = 12
		SELECT PVPWin, PVPLose, PVPDraw, PVPExp
		FROM dbo.PVPScores WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;


	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
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
