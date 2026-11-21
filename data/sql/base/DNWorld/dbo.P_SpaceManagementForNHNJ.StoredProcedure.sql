USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SpaceManagementForNHNJ]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 29
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-12-12
description : Garbage Data ??? ??? (?? ???? ??, ?? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = ???? 3?? ?? ??? ??(#77291) by ??? at 2012-12-26 
8 = ?? ?? DWC ??? ?? ?? ?? by ??? at 2013-02-22
22 = ???? ?? ???  ?? ????? ?? by ??? at 2013-05-30
26 = ???? ?? ???  ?? ????? ?? by ??? at 2013-06-07
27 = ?? ??? ?? ?? ?? ?? by ??? at 2013-06-10
28 = ???? ??? ???? ??? ?? ?? by ??? at 2013-06-11
29 = ??? ????? 60? ?? ?????? ????? ?? by ??? at 2013-07-05
**/

CREATE PROCEDURE [dbo].[P_SpaceManagementForNHNJ]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @dt2Now datetime2(0) = GETDATE();
DECLARE @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dt2Now);
DECLARE @MaxCharacterLevel tinyint

DECLARE @DeletedItems TABLE(
	ItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	ItemRemainCount smallint NOT NULL,
	PayMethodCode tinyint NULL,
	ItemLevel tinyint NULL,
	ItemPotential tinyint NULL,
	SealCount tinyint NULL,
	ItemOption tinyint NULL,
	SoulBoundFlag bit NULL,
	ItemDurability smallint NULL
);

DECLARE @DeletedItems2 TABLE(
	ItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	ItemRemainCount smallint NOT NULL,
	OwnerCharacterID bigint NOT NULL,
	OwnershipStartDate datetime2(0) NULL,
	ItemLevel tinyint NULL,
	ItemPotential tinyint NULL,
	SealCount tinyint NULL,
	ItemOption tinyint NULL,
	SoulBoundFlag bit NULL,
	ItemDurability smallint NULL
);

DECLARE @DeletedItems3 table (
	ItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	ItemRemainCount smallint NOT NULL,
	OwnerCharacterID bigint NOT NULL,
	OwnershipStartDate datetime2(0) NULL,
	ItemDurability smallint NULL,
	ItemLevel tinyint NULL,
	ItemPotential tinyint NULL,
	ItemPotentialMoveCount tinyint NULL
);

DECLARE @DeletedCoin TABLE(
	CharacterID bigint NOT NULL PRIMARY KEY,
	Coin bigint NOT NULL,
	WarehouseCoin bigint NOT NULL
);

DECLARE @DeletedCashRebirthCoin TABLE(
	CharacterID bigint NOT NULL,
	RemainCoinCount smallint NOT NULL
);

DECLARE @Characters TABLE(CharacterID bigint NOT NULL PRIMARY KEY);

DECLARE @ToBeDeletedCharacters TABLE(
	CharacterID bigint NOT NULL
);

DECLARE @WrongGuildDepotItems TABLE(
	MaterializedItemID bigint NOT NULL, 
	OwnerCharacterID bigint NULL
);

DECLARE @tblDeletedDragonJewelIDs TABLE (
	MaterializedItemID bigint NOT NULL
);

DECLARE @nvcStmt nvarchar(max) = N'';
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/*?? ?? ??? ???.*/
	SELECT @MaxCharacterLevel = MAX(CS.CharacterLevel) 
	FROM dbo.Characters C WITH(NOLOCK)
		INNER JOIN dbo.CharacterStatus CS WITH(NOLOCK) ON C.CharacterID = CS.CharacterID
	WHERE C.AccountLevelCode = 0;

	SET @nvcStmt = N'
DECLARE @ToBeDeletedCharacters TABLE(
	CharacterID bigint NOT NULL
);

WITH r AS (
SELECT AccountID
FROM Membership.DNMembership.dbo.Restraints
WHERE CancelFlag = CAST(0 AS bit)
	AND StartDate < DATEADD(DAY, -90, GETDATE())
	AND EndDate >= ''2020-01-01''
)
INSERT @ToBeDeletedCharacters (CharacterID)
SELECT c.CharacterID
FROM r
	INNER JOIN Membership.DNMembership.dbo.Characters c ON r.AccountID = c.AccountID
WHERE NOT EXISTS (SELECT * FROM dbo.GuildMembers WHERE CharacterID = c.CharacterID AND GuildMemberRoleCode = 0);

INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate)
SELECT A.GuildID, 4, A.CharacterID, B.CharacterLevel, GETDATE()
FROM dbo.GuildMembers A
	INNER JOIN dbo.CharacterStatus B ON A.CharacterID = B.CharacterID
	INNER JOIN @ToBeDeletedCharacters C ON B.CharacterID = C.CharacterID;

DELETE dbo.GuildMembers
FROM dbo.GuildMembers A
	INNER JOIN @ToBeDeletedCharacters B ON A.CharacterID = B.CharacterID;

DELETE dbo.TeachersAndPupils
FROM dbo.TeachersAndPupils A
	INNER JOIN @ToBeDeletedCharacters B ON (A.TeacherCharacterID = B.CharacterID OR A.PupilCharacterID = B.CharacterID)
WHERE GraduateFlag = CAST(0 AS bit);

UPDATE dbo.Characters
SET DeleteFlag = 1, DeleteDate = GETDATE(), DeleteByAdminFlag = 1
FROM dbo.Characters A
	INNER JOIN @ToBeDeletedCharacters B ON A.CharacterID = B.CharacterID
WHERE A.DeleteFlag = CAST(0 AS bit);
INSERT Membership.DNMembership.dbo.DeletedCharacters (CharacterID, CharacterName, DeleteDate, DeletePrivateIP, DeletePublicIP)
SELECT A.CharacterID, ISNULL(A.CharacterName, N''''), GETDATE(), dbo.FN_IP2IPNumber(''0.0.0.0''), dbo.FN_IP2IPNumber(''0.0.0.0'')
FROM Membership.DNMembership.dbo.Characters A
	INNER JOIN @ToBeDeletedCharacters B ON A.CharacterID = B.CharacterID
WHERE A.DeleteFlag = CAST(0 AS bit);

INSERT Membership.DNMembership.dbo.CharacterDeleteReviveLogs (LogDate, CharacterID, DeleteFlag, PrivateIP, PublicIP)
SELECT GETDATE(), A.CharacterID, 1, dbo.FN_IP2IPNumber(''0.0.0.0''), dbo.FN_IP2IPNumber(''0.0.0.0'')
FROM Membership.DNMembership.dbo.Characters A
	INNER JOIN @ToBeDeletedCharacters B ON A.CharacterID = B.CharacterID
WHERE A.DeleteFlag = CAST(0 AS bit);

UPDATE Membership.DNMembership.dbo.Characters
SET DeleteFlag = 1, DeleteByAdminFlag = 1
FROM Membership.DNMembership.dbo.Characters A
	INNER JOIN @ToBeDeletedCharacters B ON A.CharacterID = B.CharacterID
WHERE A.DeleteFlag = CAST(0 AS bit);'

	EXEC (@nvcStmt);

	IF OBJECT_ID('TempDeleteCharacters') IS NULL
	BEGIN
		CREATE TABLE dbo.TempDeleteCharacters
		( 
			CharacterID bigint
		  , DeleteYN tinyint
		);

		CREATE CLUSTERED INDEX IX_NC_TempDeleteCharacters_DeleteYN_CharacterID ON TempDeleteCharacters (DeleteYN, CharacterID) ON IndexFG1
	END

	/**_# 40? ?? ?? ?? ?? / ??? ??? ??? ?? ?? ??? ??*/
	DELETE dbo.MaterializedItems
	OUTPUT deleted.ItemSerial, deleted.ItemID, deleted.ItemRemainCount, deleted.PayMethodCode, deleted.ItemLevel, deleted.ItemPotential, deleted.SealCount, deleted.ItemOption, deleted.SoulBoundFlag, deleted.ItemDurability
	INTO @DeletedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemDurability)
	FROM dbo.Mails M WITH (NOLOCK, INDEX(IX_NN_Mails_SendDate)) 
		INNER JOIN dbo.MailAttachItems MAI WITH (NOLOCK) ON MAI.MailID = M.MailID
		INNER JOIN dbo.MaterializedItems MI ON MI.ItemSerial = MAI.ItemSerial
	WHERE (M.SendDate < CAST(CAST(DATEADD(DAY, -30, @dt2Now) AS date) AS datetime2(0)) AND MAI.ReceiveFlag = CAST(0 AS bit)) OR (M.MailExpireDate IS NOT NULL AND M.MailExpireDate <= @dt2Now AND MAI.ReceiveFlag = CAST(0 AS bit))
	OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1);

	DELETE dbo.MailAttachItems
	FROM dbo.Mails M WITH (NOLOCK, INDEX(IX_NN_Mails_SendDate))
		INNER JOIN dbo.MailAttachItems MAI WITH (NOLOCK) ON MAI.MailID = M.MailID
	WHERE M.SendDate < CAST(CAST(DATEADD(DAY, -30, @dt2Now) AS date) AS datetime2(0)) OR (M.MailExpireDate IS NOT NULL AND M.MailExpireDate <= @dt2Now)
	OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1);
		
	DELETE dbo.WishItems
	FROM dbo.Mails A
		INNER JOIN dbo.WishItems B ON A.MailID = B.MailID
	WHERE A.SendDate < CAST(CAST(DATEADD(DAY, -30, @dt2Now) AS date) AS datetime2(0)) OR (A.MailExpireDate IS NOT NULL AND A.MailExpireDate <= @dt2Now)

	DELETE dbo.WishProducts
	FROM dbo.Mails A
		INNER JOIN dbo.WishProducts B ON A.MailID = B.MailID
	WHERE A.SendDate < CAST(CAST(DATEADD(DAY, -30, @dt2Now) AS date) AS datetime2(0)) OR (A.MailExpireDate IS NOT NULL AND A.MailExpireDate <= @dt2Now)

	DELETE dbo.Mails
	WHERE SendDate < CAST(CAST(DATEADD(DAY, -30, @dt2Now) AS date) AS datetime2(0)) OR (MailExpireDate IS NOT NULL AND MailExpireDate <= @dt2Now)

	IF @intLogTableSuffix = 0
		INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, NULL, ItemDurability
		FROM @DeletedItems T
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));
	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, NULL, ItemDurability
		FROM @DeletedItems T
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));

	ELSE
		INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemLevelBefore, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, NULL, ItemDurability
		FROM @DeletedItems T
	WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));

	/**_# ??? ??? ??*/
	DELETE dbo.PVPStartIDs WHERE PVPStartDate < CAST(CAST(DATEADD(DAY, -15, @dt2Now) AS date) AS datetime2(0));
	DELETE dbo.PartyStartIDs WHERE PartyStartDate < CAST(CAST(DATEADD(DAY, -15, @dt2Now) AS date) AS datetime2(0));
	DELETE dbo.StageStartIDs WHERE StageStartDate < CAST(CAST(DATEADD(DAY, -15, @dt2Now) AS date) AS datetime2(0));
	DELETE dbo.NestGateStartIDs WHERE NestGateStartDate < CAST(CAST(DATEADD(DAY, -15, @dt2Now) AS date) AS datetime2(0));

	/**_# ??? ?? DWC ??? ?? ??? ?? ?????? ????? ???.*/
	IF EXISTS (SELECT * FROM dbo.DWCChannels WHERE EnableFlag = 0 AND DataInitFlag = 0 AND DWCEndTime <= @dt2Now) AND NOT EXISTS (SELECT * FROM dbo.DWCChannels WHERE EnableFlag = 1 AND @dt2Now BETWEEN DWCStartTime AND DWCEndTime)
	BEGIN		
		DELETE dbo.DWCTeamMembers;
		DELETE dbo.DWCTeams;
		TRUNCATE TABLE dbo.DWCRanking;
		TRUNCATE TABLE dbo.DWCTeamHistory;
		TRUNCATE TABLE dbo.DWCPersonalPlayHistory;
		TRUNCATE TABLE dbo.DWCTeamPointPlayHistory;

		/**_# DWC ??? ?? ????*/
		INSERT TempDeleteCharacters (CharacterID, DeleteYN)
		SELECT CharacterID, 0
		FROM dbo.Characters WITH(NOLOCK)
		WHERE AccountLevelCode = 110 AND DeleteFlag = 0;

		UPDATE dbo.Characters
		SET DeleteFlag = 1, DeleteDate = CASE WHEN DeleteFlag = 1 THEN DeleteDate ELSE GETDATE() END, DeleteByAdminFlag = 1
		FROM dbo.Characters A
		WHERE AccountLevelCode = 110 AND (DeleteFlag = 0 OR (DeleteFlag = 1 AND DeleteByAdminFlag = 0))

		UPDATE dbo.DWCChannels
		SET DataInitFlag = 1
		WHERE EnableFlag = 0 AND DataInitFlag = 0;
	END

	/**_# ?? ? ? 180? ?? ?? ???? ?? ? ??? ??? ?? ?? ??? ????*/
	INSERT dbo.TempDeleteCharacters (CharacterID, DeleteYN)
	SELECT C.CharacterID, 0
	FROM dbo.Characters C WITH (NOLOCK)
	WHERE C.DeleteFlag = CAST(1 AS bit)
	  AND C.DeleteDate < DATEADD(DAY, -180, CAST(CAST(@dt2Now AS date) AS datetime2(0)))
	  AND NOT EXISTS (SELECT * FROM dbo.TempDeleteCharacters TC WITH (NOLOCK) WHERE TC.CharacterID = C.CharacterID);

	INSERT @Characters (CharacterID)
	SELECT CharacterID
	FROM dbo.TempDeleteCharacters WITH (NOLOCK)
	WHERE DeleteYN = 0;

	INSERT @Characters (CharacterID)
	SELECT CharacterID FROM @ToBeDeletedCharacters A WHERE NOT EXISTS (SELECT * FROM @Characters WHERE CharacterID = A.CharacterID);
	/**_# ?? ? ? 180? ?? ?? ???? ?? ? ??? ??*/

	DELETE dbo.MaterializedItems
	OUTPUT deleted.ItemSerial, deleted.ItemID, deleted.ItemRemainCount, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemLevel, deleted.ItemPotential, deleted.SealCount, deleted.ItemOption, deleted.SoulBoundFlag, deleted.ItemDurability
	INTO @DeletedItems2 (ItemSerial, ItemID, ItemRemainCount, OwnerCharacterID, OwnershipStartDate, ItemLevel, ItemPotential, SealCount, ItemOption, SoulBoundFlag, ItemDurability)
	FROM dbo.MaterializedItems MI
		INNER JOIN @Characters CH ON MI.OwnerCharacterID = CH.CharacterID;

	IF @intLogTableSuffix = 0
		INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemDurability, ItemLevel, ItemPotential)
		SELECT @dt2Now, ItemSerial, ItemID, 41, NULL, OwnerCharacterID, 0, ItemRemainCount, NULL, NULL, OwnershipStartDate, NULL, SealCount, ItemOption, SoulBoundFlag, ItemDurability, ItemLevel, ItemPotential
		FROM @DeletedItems2 A
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 41));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemDurability, ItemLevel, ItemPotential)
		SELECT @dt2Now, ItemSerial, ItemID, 41, NULL, OwnerCharacterID, 0, ItemRemainCount, NULL, NULL, OwnershipStartDate, NULL, SealCount, ItemOption, SoulBoundFlag, ItemDurability, ItemLevel, ItemPotential
		FROM @DeletedItems2 A
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 41));

	ELSE
		INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemDurability, ItemLevel, ItemPotential)
		SELECT @dt2Now, ItemSerial, ItemID, 41, NULL, OwnerCharacterID, 0, ItemRemainCount, NULL, NULL, OwnershipStartDate, NULL, SealCount, ItemOption, SoulBoundFlag, ItemDurability, ItemLevel, ItemPotential
		FROM @DeletedItems2 A
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 41));

	UPDATE dbo.CharacterStatus
	SET Coin = 0, WarehouseCoin = 0
	OUTPUT deleted.CharacterID, deleted.Coin, deleted.WarehouseCoin INTO @DeletedCoin (CharacterID, Coin, WarehouseCoin)
	FROM dbo.CharacterStatus CHS
		INNER JOIN @Characters CH ON CHS.CharacterID = CH.CharacterID;

	IF @intLogTableSuffix = 0
		INSERT dbo.CoinLogs0 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
		SELECT @dt2Now, CharacterID, 1, 26, 0, - Coin, NULL, NULL, NULL FROM @DeletedCoin WHERE Coin > 0
		UNION ALL
		SELECT @dt2Now, CharacterID, 2, 26, 0, - WarehouseCoin, NULL, NULL, NULL FROM @DeletedCoin WHERE WarehouseCoin > 0;

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.CoinLogs1 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
		SELECT @dt2Now, CharacterID, 1, 26, 0, - Coin, NULL, NULL, NULL FROM @DeletedCoin WHERE Coin > 0
		UNION ALL
		SELECT @dt2Now, CharacterID, 2, 26, 0, - WarehouseCoin, NULL, NULL, NULL FROM @DeletedCoin WHERE WarehouseCoin > 0;

	ELSE
		INSERT dbo.CoinLogs2 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
		SELECT @dt2Now, CharacterID, 1, 26, 0, - Coin, NULL, NULL, NULL FROM @DeletedCoin WHERE Coin > 0
		UNION ALL
		SELECT @dt2Now, CharacterID, 2, 26, 0, - WarehouseCoin, NULL, NULL, NULL FROM @DeletedCoin WHERE WarehouseCoin > 0;
	DELETE dbo.CashRebirthCoins
	OUTPUT deleted.CharacterID, deleted.RemainCoinCount INTO @DeletedCashRebirthCoin (CharacterID, RemainCoinCount)
	FROM dbo.CashRebirthCoins CRC
		INNER JOIN @Characters CH ON CRC.CharacterID = CH.CharacterID;
	IF @intLogTableSuffix = 0
		INSERT dbo.CashRebirthCoinLogs0 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, ISNULL(SUM(RemainCoinCount), 0), 0, NULL, NULL FROM @DeletedCashRebirthCoin GROUP BY CharacterID;

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.CashRebirthCoinLogs1 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, ISNULL(SUM(RemainCoinCount), 0), 0, NULL, NULL FROM @DeletedCashRebirthCoin GROUP BY CharacterID;

	ELSE
		INSERT dbo.CashRebirthCoinLogs2 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, ISNULL(SUM(RemainCoinCount), 0), 0, NULL, NULL FROM @DeletedCashRebirthCoin GROUP BY CharacterID;

	DELETE dbo.NPCFavor FROM dbo.NPCFavor NF INNER JOIN @Characters CH ON NF.CharacterID = CH.CharacterID;
	DELETE dbo.Quests FROM dbo.Quests A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.MissionBinaries FROM dbo.MissionBinaries A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.PVPScores FROM dbo.PVPScores A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.Notifiers FROM dbo.Notifiers A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.DungeonClear FROM dbo.DungeonClear A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.NestClear FROM dbo.NestClear A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.CharacterAbilities FROM dbo.CharacterAbilities A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.EquipmentAttributes FROM dbo.EquipmentAttributes A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.QuickSlots FROM dbo.QuickSlots A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.Appellations FROM dbo.Appellations A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.Skills FROM dbo.Skills A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.JobChangeLogs FROM dbo.JobChangeLogs A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.ManufactureSkillRecipes FROM dbo.ManufactureSkillRecipes A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.SecondarySkills FROM dbo.SecondarySkills A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.AchieveMissions FROM dbo.AchieveMissions A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.AssignMissionDetails FROM dbo.AssignMissionDetails C INNER JOIN dbo.AssignMissions A ON C.CharacterID = A.CharacterID AND C.MissionTypeCode = A.MissionTypeCode INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.AssignMissions FROM dbo.AssignMissions A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.AssignQuests FROM dbo.AssignQuests A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.BlockedCharacters FROM dbo.BlockedCharacters A INNER JOIN @Characters B ON A.BlockedCharacterID = B.CharacterID;
	DELETE dbo.BlockedCharacters FROM dbo.BlockedCharacters A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.CompleteQuests FROM dbo.CompleteQuests A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.DarkLairPartyMembers FROM dbo.DarkLairPartyMembers A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.DarkLairPersonalBest FROM dbo.DarkLairPersonalBest A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.DarkLairRanking WHERE DarkLairRankingID NOT IN (SELECT DarkLairRankingID FROM DarkLairPartyMembers) AND DarkLairRankingID NOT IN (SELECT DarkLairRankingID FROM DarkLairPersonalBest);
	DELETE dbo.EffectItems FROM dbo.EffectItems A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.FieldAttachments FROM dbo.FieldAttachments FA INNER JOIN dbo.Fields A ON (FA.FarmID = A.FarmID AND FA.FieldIndex = A.FieldIndex) INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.Fields FROM dbo.Fields A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.FieldForCharacterAttachments FROM dbo.FieldForCharacterAttachments A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.FieldsForCharacter FROM dbo.FieldsForCharacter A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.Friends FROM dbo.Friends A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.Friends FROM dbo.Friends A INNER JOIN @Characters B ON A.FriendCharacterID = B.CharacterID;
	DELETE dbo.FriendGroups FROM dbo.FriendGroups A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.GuildMembers FROM dbo.GuildMembers A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.HiddenCharacters FROM dbo.HiddenCharacters A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.MailAttachItems FROM dbo.MailAttachItems C INNER JOIN dbo.Mails A ON C.MailID = A.MailID INNER JOIN @Characters B ON A.SenderCharacterID = B.CharacterID;
	DELETE dbo.MailAttachItems FROM dbo.MailAttachItems C INNER JOIN dbo.Mails A ON C.MailID = A.MailID INNER JOIN @Characters B ON A.ReceiverCharacterID = B.CharacterID;
	DELETE dbo.Mails FROM dbo.Mails A INNER JOIN @Characters B ON A.ReceiverCharacterID = B.CharacterID;
	DELETE dbo.Profiles FROM dbo.Profiles A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;
	DELETE dbo.Trade FROM dbo.Trade A INNER JOIN @Characters CH ON A.CharacterID = CH.CharacterID;

	DECLARE @tblTemp table (
		ItemSerial bigint NOT NULL,
		ItemID int NOT NULL,
		TradeStatusCode tinyint NOT NULL
	);

	DELETE dbo.Trade
	OUTPUT deleted.ItemSerial, deleted.ItemID, deleted.TradeStatusCode
	INTO @tblTemp (ItemSerial, ItemID, TradeStatusCode)
	WHERE TradeStatusCode <> 1 AND RegisterDate < DATEADD(DAY, -7, CAST(CAST(@dt2Now AS date) AS datetime2(0)));

	UPDATE dbo.MaterializedItems
	SET SealCount += 1
	FROM dbo.MaterializedItems A
		INNER JOIN @tblTemp B ON A.ItemSerial = B.ItemSerial
		INNER JOIN dbo.TradeItems C ON B.ItemID = C.ItemID
	WHERE C.CashItemFlag = CAST(1 AS bit) AND B.TradeStatusCode = 0;

	/* ??? ?? ?? ??? ????? ?? 60? ?? ???? ??? ????.
		 --> ???? ???? ??? ?? ?? ???? ?? ?? ?? ?????? ?*/
	DECLARE @tblTemp2 table (
		CharacterID bigint NOT NULL,
		TradeID int NOT NULL,
		TradeCommission int NOT NULL,
		Price int NOT NULL
	);	
	DECLARE @tblTemp3 table (
		CharacterID bigint NOT NULL,
		CalculateResult bigint NOT NULL,--// ?????
		Coin bigint NOT NULL, --// ?? ???? ?? ?? ??
		AfterCoin bigint NOT NULL --// ?? ? ???? ?? ?? ??
	);	

	-- 60???? ??? ??? ??? ???? ?? ??? ??
	INSERT @tblTemp2(CharacterID, TradeID, TradeCommission, Price)
	SELECT CharacterID, TradeID, TradeCommission, Price
	FROM dbo.Trade
	WHERE TradeStatusCode = 1 
	  AND RegisterDate < DATEADD(DAY, -60, CAST(CAST(@dt2Now AS date) AS datetime2(0)))
	  AND PayMethodCode = 1;

	-- ???? ??? ??? int? max ?(2147483647)?? ?? ?? ?????? ?? ??
	WITH CTE AS
	(
		SELECT A.CharacterID, SUM(CAST(A.Price AS bigint) - A.TradeCommission) AS CalculateResult
		FROM @tblTemp2 A
			INNER JOIN dbo.Characters B ON A.CharacterID = B.CharacterID
		WHERE B.DeleteFlag = 0
		GROUP BY A.CharacterID
	)
	INSERT @tblTemp3(CharacterID, CalculateResult, Coin, AfterCoin)
	SELECT A.CharacterID, A.CalculateResult, B.Coin, CAST(B.Coin AS bigint) + A.CalculateResult
	FROM CTE A
		INNER JOIN dbo.CharacterStatus B ON A.CharacterID = B.CharacterID
	WHERE CAST(B.Coin AS bigint) + A.CalculateResult < 2147483647 
      AND B.CharacterLevel >= CASE WHEN @MaxCharacterLevel <= 60 THEN @MaxCharacterLevel ELSE 60 END; 

	--??? ??? 2147483647 ? ???? ??????? ????.
	DELETE @tblTemp2 
	FROM @tblTemp2 A
	WHERE NOT EXISTS (SELECT * FROM @tblTemp3 WHERE CharacterID = A.CharacterID);

	--??? ???? ??? ??? ????.
	DELETE dbo.Trade
	FROM @tblTemp2 A
		INNER JOIN dbo.Trade B ON A.TradeID = B.TradeID;

	--??? ??? ??? ????.
	UPDATE dbo.CharacterStatus
	SET Coin = AfterCoin
	  , CheckSumFlag = 0       --//???? OFF ???.
	FROM @tblTemp3 A 
		INNER JOIN dbo.CharacterStatus B ON A.CharacterID = B.CharacterID;

	-- CoinLog ??
	IF @intLogTableSuffix = 0
		INSERT dbo.CoinLogs0 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
		SELECT @dt2Now, CharacterID, 1, 61, AfterCoin, CalculateResult, NULL, NULL, NULL 
		FROM @tblTemp3

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.CoinLogs1 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
		SELECT @dt2Now, CharacterID, 1, 61, AfterCoin, CalculateResult, NULL, NULL, NULL 
		FROM @tblTemp3
	ELSE
		INSERT dbo.CoinLogs2 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey)
		SELECT @dt2Now, CharacterID, 1, 61, AfterCoin, CalculateResult, NULL, NULL, NULL 
		FROM @tblTemp3

	UPDATE dbo.TempDeleteCharacters
	SET DeleteYN = 1
	FROM dbo.TempDeleteCharacters TC
		INNER JOIN @Characters C ON (TC.CharacterID = C.CharacterID);

	DELETE dbo.MaterializedItems WHERE ItemRemainCount = 0;

	/**_# 30? ?? ?? ?? ?? ??*/

	-- ???? ????
	DELETE dbo.AttachItemsForMailSchedule
	  FROM dbo.AttachItemsForMailSchedule A
		   INNER JOIN dbo.MailSchedules B ON (A.MailScheduleID = B.MailScheduleID)
	 WHERE B.SendDate < CONVERT(smalldatetime, DATEADD(day, -30, @dt2Now));

	-- ???? ?? ??
	DELETE dbo.MailSchedulesTarget
	  FROM dbo.MailSchedulesTarget A
		   INNER JOIN dbo.MailSchedules B ON (A.MailScheduleID = B.MailScheduleID)
	 WHERE B.SendDate < CONVERT(smalldatetime, DATEADD(day, -30, @dt2Now));

	-- ???? ?? ??
	DELETE dbo.SendedMails
	  FROM dbo.SendedMails A
		   INNER JOIN dbo.MailSchedules B ON (A.MailScheduleID = B.MailScheduleID)
	 WHERE B.SendDate < CONVERT(smalldatetime, DATEADD(day, -30, @dt2Now));

	-- ????
	DELETE dbo.MailSchedules
	 WHERE SendDate < CONVERT(smalldatetime, DATEADD(day, -30, @dt2Now));


	/**_# ? ?? ???? ?? ??? ??? ?? ?? ?? */
	DELETE dbo.CompleteEventQuest
	WHERE EventQuestExpireDate < CAST(CAST(@dt2Now AS date) AS smalldatetime);

	/**_# ??? ??? ??? ?? ?? */
	TRUNCATE TABLE dbo.RepurchaseItem;

	/**_# ?? ?? ??? ?? */
	DELETE dbo.PartyJoins;
	DELETE dbo.Parties;

	/**_# ???? ?? ??? ?? */
	DELETE dbo.PrivateChannelMembers;

	DELETE dbo.PrivateChannels;

	UPDATE dbo.Characters
	SET PrivateChannelID = NULL;

	/**_# ?? ???? ?? ?? ?? (??? ????) */
	DELETE dbo.DragonJewels
	OUTPUT deleted.MaterializedItemID
	INTO @tblDeletedDragonJewelIDs (MaterializedItemID)
	FROM dbo.DragonJewels DJ WITH (NOLOCK) 
		 LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK) 
		 ON DJ.ParentItemMaterializedItemID = MI.MaterializedItemID
	WHERE MI.MaterializedItemID IS NULL;

	DELETE dbo.MaterializedItems
	FROM @tblDeletedDragonJewelIDs A INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON A.MaterializedItemID = MI.MaterializedItemID;

	/**_# ??? ????? ?? ? ??? */
	DELETE dbo.PushQueue;

	/**_# ??? ?? ?? ??? ?? */
	WITH A AS (
		SELECT MapID, CharacterID, ClearTime, MIN(ClearDate) AS ClearDate
		FROM dbo.NestClearRecord
		WHERE PartyID IS NULL
		GROUP BY MapID, CharacterID, ClearTime
	), B AS (
		SELECT RANK() OVER(PARTITION BY MapID ORDER BY ClearTime) AS Ranking, MapID, CharacterID, ClearTime, ClearDate	
		FROM A
	)
	DELETE dbo.NestClearRecord
	FROM B LEFT OUTER JOIN dbo.NestClearRecord C ON B.CharacterID = C.CharacterID AND B.MapID = C.MapID AND B.ClearTime = C.ClearTime
	WHERE B.Ranking > 5 OR B.ClearDate <> C.ClearDate;

	WITH A AS (
		SELECT MapID, CharacterID, ClearTime, MIN(ClearDate) AS ClearDate
		FROM dbo.NestClearRecord
		WHERE PartyID IS NOT NULL
		GROUP BY MapID, CharacterID, ClearTime
	), B AS (
		SELECT RANK() OVER(PARTITION BY MapID ORDER BY ClearTime) AS Ranking, MapID, CharacterID, ClearTime, ClearDate	
		FROM A
	)
	DELETE dbo.NestClearRecord
	FROM B LEFT OUTER JOIN dbo.NestClearRecord C ON B.CharacterID = C.CharacterID AND B.MapID = C.MapID AND B.ClearTime = C.ClearTime
	WHERE B.Ranking > 1 OR B.ClearDate <> C.ClearDate;

	/**_# ???? ??? ?? ???? ?? */
	INSERT @WrongGuildDepotItems(MaterializedItemID, OwnerCharacterID)
	SELECT A.MaterializedItemID, B.OwnerCharacterID
	FROM dbo.GuildDepots A WITH(NOLOCK)	
		INNER JOIN dbo.MaterializedItems B WITH(NOLOCK) ON A.MaterializedItemID = B.MaterializedItemID
	WHERE B.ItemLocationCode <> 16;

	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = 16
	FROM @WrongGuildDepotItems A
		INNER JOIN dbo.MaterializedItems B ON A.MaterializedItemID = B.MaterializedItemID
	WHERE A.OwnerCharacterID IS NULL;

	DELETE dbo.GuildDepots
	FROM @WrongGuildDepotItems A
		INNER JOIN dbo.GuildDepots B ON A.MaterializedItemID = B.MaterializedItemID
	WHERE A.OwnerCharacterID IS NOT NULL;

	DELETE dbo.GuildDepots
	FROM dbo.GuildDepots A
	WHERE NOT EXISTS (SELECT * 
					  FROM dbo.MaterializedItems 
					  WHERE MaterializedItemID = A.MaterializedItemID);

	/**_# ???? 3?? ?? ?? ?? */
	INSERT dbo.GuildHistoryBak (GuildID, GuildHistoryID, GuildHistoryCode, CharacterID, CharacterLevel, ItemID, ItemLevel, RegisterDate, GuildLevel, UsedGuildPoint, GuildMemberRoleCode, GuildDepotSize, GuildNameBefore, GuildNameAfter)
	SELECT GuildID, GuildHistoryID, GuildHistoryCode, CharacterID, CharacterLevel, ItemID, ItemLevel, RegisterDate, GuildLevel, UsedGuildPoint, GuildMemberRoleCode, GuildDepotSize, GuildNameBefore, GuildNameAfter
	FROM dbo.GuildHistory WITH(NOLOCK)
	WHERE RegisterDate <= DATEADD(MONTH, -3, @dt2Now);

	DELETE dbo.GuildHistory WHERE RegisterDate <= DATEADD(MONTH, -3, @dt2Now);

	/**_# ?? ?? ?? 31? ?? ?? ?? */
	INSERT dbo.GuildDepotHistoryBak (GuildID, GuildDepotHistoryID, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, RegisterDate, GuildCoinAfter)
	SELECT GuildID, GuildDepotHistoryID, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, RegisterDate, GuildCoinAfter
	FROM dbo.GuildDepotHistory WITH(NOLOCK)
	WHERE RegisterDate <= DATEADD(DAY, -31, @dt2Now);

	DELETE dbo.GuildDepotHistory WHERE RegisterDate <= DATEADD(DAY, -31, @dt2Now);

	/**_# 60??? ??? ?? ??? ?? */
	DELETE dbo.AbuseLogs WHERE LogDate < DATEADD(DAY, -60, GETDATE());

	/**_# 60??? ???? ?? ??? ?? */
	DELETE dbo.PVPTournamentResults
	FROM dbo.PVPTournamentIDs A
		INNER JOIN dbo.PVPTournamentResults B ON A.PVPTournamentID = B.PVPTournamentID
	WHERE A.PVPTournamentResultDate < DATEADD(DAY, -60, GETDATE());
	
	DELETE dbo.PVPTournamentIDs
	WHERE PVPTournamentResultDate < DATEADD(DAY, -60, GETDATE());

	/**_# 60??? ?? ??? ?? */
	DELETE dbo.GambleParticipantes
	FROM dbo.GambleIDs A
		INNER JOIN dbo.GambleParticipantes B ON A.GambleID = B.GambleID
	WHERE A.GambelStartDate < DATEADD(DAY, -60, GETDATE());

	DELETE dbo.GambleIDs
	WHERE GambelStartDate < DATEADD(DAY, -60, GETDATE());

	/**_# 30??? Job ?? ?? ?? */
	DELETE dbo.JobManagerLogs 
	WHERE LogDate < DATEADD(DAY, -30, GETDATE());

	/**_# 90??? ?? ?? ??? ?? */
	DELETE dbo.PotentialBoxStat 
	WHERE BasisDate < DATEADD(DAY, -90, GETDATE());

	/**_# ?? ??? ?? ?? */
	DELETE dbo.WeeklyLikes;
	/**_# ???? ???? ?? ??? ?? ?? */
	DELETE dbo.TradeItems 
	FROM dbo.TradeItems A
	WHERE NOT EXISTS (SELECT * FROM dbo.Trade WHERE ItemID = A.ItemID  AND TradeStatusCode IN (0, 1));

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
