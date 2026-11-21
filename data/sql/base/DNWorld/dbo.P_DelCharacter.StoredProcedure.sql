USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelCharacter]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 44
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-12
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103211 = ?? ?? ??? ???? ??? ? ????.
103334 = ?? ?? ???? ??? ? ????.
103346 = ??? ???? ????. ???? ??? ? ????.
103376 = DWC ??? ?? ? ? ????.
103418 = ??? ?? ????? ??? ???? ??? ? ????.
history :
12 = ???? ???, ?? ?? ??? ????? ?? at 2010-06-14
13 = ?? ?? ?? ?? ?? (+ / - ??) at 2010-06-14
14 = 10?? ?? ??? ??? ??? ??? ????? ?? at 2010-06-30
16 = AppellationBinaries ???? ? ?? ???? ?? at 2010-07-26
17 = ?? ?? ?? ???? ??? ???? ???? ?? at 2010-07-27
18 = SkipLoggingItems ???? ???? ?? ??? ????? ?? at 2010-08-09
19 = CompleteQuestBinaries ???? ? ?? ???? ?? at 2010-08-30
20 = SecondarySkills, ManufactureSkillRecipes ????? ????? ?? at 2010-09-03
21 = ????? ?? ?? ?? ?? ?? ??? ??? ?? ?? at 2010-10-06
22 = input param. @bitDeleteByAdminFlag ?? at 2010-12-02
23 = ??? ???? ??? ??? ?? ?? at 2011-02-18
27 = ???? ?? ??? ????? ???? ?? ?? by ??? at 2011-11-17
28 = 27? ?? ? ???? ?? ??? by ??? at 2011-11-17
29 = ?? ?? ?? ????? ?? by ??? at 2011-11-23
30 = [ItemLocation] ??? ??? ?? ?? by ??? at 2011-12-20
31 = ???? ?? ??? ??? ?? ?? by ??? at 2011-12-29
32 = PVPScores ??? ???? ?? ?? by ??? at 2012-03-07
35 = 32?? ??     by ??? at 2012-04-09
36 = ?????? ???? ??? ?? by ??? at 2012-04-16
37 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-18
38 = (65655) input param. @inyCheckGuildType ??. ?? ?????? ?? ???? ??? ? ??? ?? by ??? at 2012-08-10
39 = ???? ????? ????? ?? by ??? at 2013-01-08
41 = ???? ????? ?? by ??? at 2013-05-03
42 = 41? ?? by ??? at 2013-05-10
43 = ???? ????? ?? by ??? at 2013-08-07
44 = ??? ?? ?? ??? ??? ??? ??? ?? by ??? at 2013-08-19
**/

CREATE PROCEDURE [dbo].[P_DelCharacter]
	  @inbCharacterID bigint --// ??? ID
	, @bitCharacterNameReserveFlag bit OUTPUT --// ??? ?? ?? ??. 0=???? ??, 1=???
	, @dt2DeleteDate datetime2(0) OUTPUT --// ??? ?? ??
	, @bitDeleteByAdminFlag bit = 0 --// 0=?? ?? ??, 1=??? ??
	, @inyCheckGuildType tinyint = 1 --// 1=????? ????, 2=??? ?? ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
DECLARE @intReturnValue int, @intLogTableSuffix int, @inbCoin bigint, @inbWarehouseCoin bigint, @nvcStmt nvarchar(4000), @intGuildID int, @inyCharacterLevel tinyint, @inyAccountLevelCode tinyint;

DECLARE @tblCashRebirthCoin table (RemainCoinCount smallint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dt2DeleteDate = GETDATE();
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2DeleteDate);

	/**_# ?? ??? ????? 103101? ?????. */
	IF @inyCheckGuildType = 1 AND EXISTS (
		SELECT * FROM dbo.GuildMembers WITH (NOLOCK) WHERE CharacterID = @inbCharacterID AND GuildMemberRoleCode = 0
	)
	BEGIN
		SET @intReturnValue = 103101;
		GOTO ErrorHandler;
	END

	/**_# ??? ????? 103346? ?????. */
	IF @inyCheckGuildType = 2 AND EXISTS (
		SELECT * FROM dbo.GuildMembers WITH (NOLOCK) WHERE CharacterID = @inbCharacterID
	)
	BEGIN
		SET @intReturnValue = 103346;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ?? ???? 103101? ?????.*/
	IF EXISTS (
		SELECT *
		FROM dbo.TeachersAndPupils WITH (NOLOCK)
		WHERE GraduateFlag = CAST(0 AS bit) AND (TeacherCharacterID = @inbCharacterID OR PupilCharacterID = @inbCharacterID)
	)
	BEGIN
		SET @intReturnValue = 103211;
		GOTO ErrorHandler;
	END

	/**_# ?? ???? 103334? ?????.*/
	IF EXISTS (
		SELECT * 
		FROM dbo.BestFriends WITH (NOLOCK, INDEX(PK_BestFriends), FORCESEEK) 
		WHERE CharacterID = @inbCharacterID 
	)
	BEGIN
		SET @intReturnValue = 103334;
		GOTO ErrorHandler;
	END

	/**_# DWC ?? ???? 103376? ?????.*/
	IF EXISTS (
		SELECT * 
		FROM dbo.DWCTeamMembers WITH(NOLOCK)
		WHERE CharacterID = @inbCharacterID
	)	
	BEGIN
		SET @intReturnValue = 103376;
		GOTO ErrorHandler;
	END
	
	SELECT @inyAccountLevelCode = AccountLevelCode
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
	
	/**_# ??? ?? ??? ??? ???? ????? 103418? ?????.*/
	IF @inyAccountLevelCode = 120 AND EXISTS (SELECT * FROM dbo.DNGirlSeasonSchedules WITH (NOLOCK) WHERE SeasonStartDate <= @dt2DeleteDate AND CeremonyEndDate >= @dt2DeleteDate)
	BEGIN
		SET @intReturnValue = 103418;
		GOTO ErrorHandler;
	END
	
	SELECT @bitCharacterNameReserveFlag = CASE WHEN CharacterLevel <= 9 THEN 0 ELSE 1 END
		, @inyCharacterLevel = CharacterLevel
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
	
	
	/**_# [Characters] ???? UPDATE???.*/
	UPDATE dbo.Characters
	SET DeleteFlag = 1, DeleteDate = @dt2DeleteDate, DeleteByAdminFlag = @bitDeleteByAdminFlag
	WHERE CharacterID = @inbCharacterID AND DeleteFlag = CAST(0 AS bit);

	/**_# ??? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END
	
	/**_# ???? ??? ?????.*/
	DELETE dbo.GuildJoinCharacters WHERE CharacterID = @inbCharacterID;

	/**_# ???? ?????.*/
	IF @inyCheckGuildType <> 2
	BEGIN
		SELECT @intGuildID = GuildID
		FROM dbo.GuildMembers WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID;

		IF @@ROWCOUNT > 0
			EXEC dbo.P_DelGuildMember
				  @intGuildID = @intGuildID
				, @inbCharacterID = @inbCharacterID
				, @inyCharacterLevel = @inyCharacterLevel
				, @bitBanishFlag = 0;
	END

	IF @bitCharacterNameReserveFlag = 0
	BEGIN
		UPDATE dbo.CharacterStatus
		SET @inbCoin = Coin
			, @inbWarehouseCoin = WarehouseCoin
			, Coin = 0
			, WarehouseCoin = 0
		WHERE CharacterID = @inbCharacterID;

		IF @intLogTableSuffix = 0
			INSERT dbo.CoinLogs0 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
			SELECT @dt2DeleteDate, @inbCharacterID, 1, 26, 0, -@inbCoin, NULL, NULL, NULL WHERE @inbCoin > 0
			UNION ALL
			SELECT @dt2DeleteDate, @inbCharacterID, 2, 26, 0, -@inbWarehouseCoin, NULL, NULL, NULL WHERE @inbWarehouseCoin > 0;

		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.CoinLogs1 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
			SELECT @dt2DeleteDate, @inbCharacterID, 1, 26, 0, -@inbCoin, NULL, NULL, NULL WHERE @inbCoin > 0
			UNION ALL
			SELECT @dt2DeleteDate, @inbCharacterID, 2, 26, 0, -@inbWarehouseCoin, NULL, NULL, NULL WHERE @inbWarehouseCoin > 0;

		ELSE
			INSERT dbo.CoinLogs2 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
			SELECT @dt2DeleteDate, @inbCharacterID, 1, 26, 0, -@inbCoin, NULL, NULL, NULL WHERE @inbCoin > 0
			UNION ALL
			SELECT @dt2DeleteDate, @inbCharacterID, 2, 26, 0, -@inbWarehouseCoin, NULL, NULL, NULL WHERE @inbWarehouseCoin > 0;

		DELETE dbo.CashRebirthCoins
		OUTPUT deleted.RemainCoinCount INTO @tblCashRebirthCoin (RemainCoinCount)
		WHERE CharacterID = @inbCharacterID;

		IF @@ROWCOUNT > 0
		BEGIN
			IF @intLogTableSuffix = 0
				INSERT dbo.CashRebirthCoinLogs0 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
				SELECT @dt2DeleteDate, @inbCharacterID, ISNULL(SUM(RemainCoinCount), 0), 0, NULL, NULL FROM @tblCashRebirthCoin;

			ELSE IF @intLogTableSuffix = 1
				INSERT dbo.CashRebirthCoinLogs1 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
				SELECT @dt2DeleteDate, @inbCharacterID, ISNULL(SUM(RemainCoinCount), 0), 0, NULL, NULL FROM @tblCashRebirthCoin;

			ELSE
				INSERT dbo.CashRebirthCoinLogs2 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
				SELECT @dt2DeleteDate, @inbCharacterID, ISNULL(SUM(RemainCoinCount), 0), 0, NULL, NULL FROM @tblCashRebirthCoin;
		END

		DELETE dbo.Quests WHERE CharacterID = @inbCharacterID;

		DELETE dbo.MissionBinaries WHERE CharacterID = @inbCharacterID;

		DELETE dbo.PVPScores WHERE CharacterID = @inbCharacterID;
	
		DELETE dbo.PVPCharacterClassScores WHERE CharacterID = @inbCharacterID;

		DELETE dbo.Notifiers WHERE CharacterID = @inbCharacterID;

		DELETE dbo.DungeonClear WHERE CharacterID = @inbCharacterID;

		DELETE dbo.CharacterAbilities WHERE CharacterID = @inbCharacterID;

		DELETE dbo.EquipmentAttributes WHERE CharacterID = @inbCharacterID;

		DELETE dbo.QuickSlots WHERE CharacterID = @inbCharacterID;

		DELETE dbo.Appellations WHERE CharacterID = @inbCharacterID;
		DELETE dbo.Skills WHERE CharacterID = @inbCharacterID;
		DELETE dbo.JobChangeLogs WHERE CharacterID = @inbCharacterID;
		UPDATE dbo.Trade SET TradeStatusCode = 3, TradeDate = @dt2DeleteDate WHERE CharacterID = @inbCharacterID;

		DELETE dbo.ManufactureSkillRecipes WHERE CharacterID = @inbCharacterID;

		DELETE dbo.SecondarySkills WHERE CharacterID = @inbCharacterID;	
		
		DELETE dbo.Friends WHERE (CharacterID = @inbCharacterID AND FriendStatusCode = 3) OR (FriendCharacterID = @inbCharacterID AND FriendStatusCode = 3);		
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
