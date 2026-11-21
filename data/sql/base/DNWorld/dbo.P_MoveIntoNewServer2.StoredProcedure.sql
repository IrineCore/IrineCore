USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_MoveIntoNewServer2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 21
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-25
description : ?? ??? ???? ?????. ?? ??, ? ??, ? ??(??, ???)? DB? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
3 = ???? ???? ???? ???? ?? ??     BY 20110929.wgkim
5 = (44494) input parameter ?? (@inyVersion) by ??? at 2011-10-31
6 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
7 = ????? ?? ?? ??	BY ??? at 2012-01-02
8 = ItemLog ???? SkipItem ?? ?? ??     by ??? at 2012-01-15
9 = @intLogTableSuffix ???? ??? ???? ??? ??     by ??? at 2012-03-08
10 = input param. @bitExecuteScheduledTask ?? by ??? at 2012-06-18
11 = ???? ???? ??? ?? ?? by ??? at 2012-06-19
12 = ??? ???? ???? ??? ?? by ??? at 2013-01-05
13 = ????? ???? ??? ????? ?? by ??? at 2013-01-15
18 = DWC ???? ???? ??? ??? ?? by ??? at 2013-02-19
19~20 = ??? ?? ???? ??? ?? ?? by ??? at 2013-04-03
21 = ???? ???? ?? ?? by ??? at 2013-07-23
**/

CREATE PROCEDURE [dbo].[P_MoveIntoNewServer2]
	  @inbCharacterID bigint --// ??? ID
	, @intLastMapID int --// ? ID
	, @intLastVillageMapID int = NULL --// (optional) ??? ? ID
	, @intLastDarkLairVillageMapID int = NULL --// (optional) ???? ??? ? ID
	, @inyLastVillageGateNumber tinyint = NULL --// (optional) ??? ??? ??
	, @intPositionX int --// X ??
	, @intPositionY int --// Y ??
	, @intPositionZ int --// Z ??
	, @fltRotate float --// ????
	, @vchObjectTypeCodes varchar(8000) = NULL --// (optional) ,? ?????? ?? ?? ?? ?? ?? ???. ? ??? ???? ???? NULL ?????.
	, @vchObjectValues varchar(8000) = NULL --// (optional) ,? ?????? ?? ?? ?? ? ???. ? ??? ???? ???? NULL ?????.
	, @vchSkillIDs varchar(8000) = NULL --// (optional) ,? ?????? 1? ?? ID ???. ? ??? ??? ??? ??? NULL ?????.
	, @vchSkillCoolTimes varchar(8000) = NULL --// (optional) ,? ?????? 1? ?? ? ?? ???. ? ??? ??? ??? ??? NULL ?????.
	, @vchItemSerials varchar(8000) = NULL --// (optional) ,? ?????? ??? ?? ?? ???. ? ??? ??? ???? ??? NULL ?????.
	, @vchItemCoolTimes varchar(8000) = NULL --// (optional) ,? ?????? ??? ? ?? ???. ? ??? ??? ???? ??? NULL ?????.
	, @vchEquipmentCodes varchar(8000) = NULL --// (optional) ,? ?????? ?? ?? ???
	, @vchEquipmentAttributeCode varchar(8000) = NULL --// (optional) ,? ?????? ?? ?? ???
	, @vchAttributeValues varchar(8000) = NULL --// (optional) ,? ?????? ?? ?? ? ???
	, @vchSkillIDs2 varchar(8000) = NULL --// (optional) ,? ?????? 2? ?? ID ???. ? ??? ??? ??? ??? NULL ?????.
	, @vchSkillCoolTimes2 varchar(8000) = NULL --// (optional) ,? ?????? 2? ?? ? ?? ???. ? ??? ??? ??? ??? NULL ?????.
	, @intViewCashEquipmentBitmap int = NULL --// (optional) ??? ??? ?? (?? or ??) ???
	, @inyVersion tinyint = 0 --// ?? ?? (0=??, 1=P_ExpireMaterializedItem2 ?? ??(???? ?? ??))
	, @bitExecuteScheduledTask bit = 1 --// ???? ??? ?? (?? ??, ??? ??? ??)? ?? ?? (0=??????, 1=???)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblTempC table (seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemSerial bigint NOT NULL);
DECLARE @tblTempD table (seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, CoolTime int NOT NULL);
DECLARE @tblItems table (ItemSerial bigint NOT NULL PRIMARY KEY, CoolTime int NOT NULL);
DECLARE @intLogTableSuffix int
	, @i int = 1
	, @j int
	, @intMailScheduleID int
	, @dtsSendDate smalldatetime
	, @nvcSenderName nvarchar(50)
	, @nvcSubject nvarchar(50)
	, @nvcContent nvarchar(300)
	, @inbAttachCoin bigint
	, @inbMailID bigint
	, @dt2CreateDate datetime2(0)
	, @dt2Now datetime2(0) = GETDATE()
	, @bitNewFlag bit
	, @sdtEventFatigueGainDate smalldatetime
	, @dtmPrevDate datetime
	, @dtmLaterDate datetime
	, @inyAccountLevelCode tinyint
	, @intMailLifespan int
	, @inyMailTabCode tinyint;

DECLARE @tblMailSchedules table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	MailScheduleID int NOT NULL,
	SendDate smalldatetime NOT NULL,
	SenderName nvarchar(50) NOT NULL,
	[Subject] nvarchar(50) NOT NULL,
	Content nvarchar(300) NOT NULL,
	AttachCoin bigint NOT NULL,
	NewFlag bit null,
	MailLifespan int NULL,
	MailTabCode tinyint NULL
);
DECLARE @tblEventFatigueSchedules table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	EventFatigueScheduleID int NOT NULL,
	GiveDate datetime NOT NULL,
	FatigueValue int NOT NULL,
	GiveFlag bit null
);
DECLARE @tblItemSerials table (Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemSerial bigint NOT NULL, ItemID int NOT NULL, ItemCount smallint NOT NULL);

DECLARE @tblAttachItems table (
	Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	ItemID int NOT NULL,
	ItemCount smallint NOT NULL,
	ItemDurability smallint NOT NULL,
	RandomSeed int NOT NULL,
	ItemLevel tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	ItemLifespan int NULL,
	EternityFlag bit NOT NULL,
	DragonJewelType tinyint NULL
);

DECLARE @tblSkills table (
	CharacterID bigint NOT NULL,
	SkillCode tinyint NOT NULL,
	SkillID int NOT NULL,
	CoolTime int NOT NULL
);

DECLARE @tblEquipmentAttributes table (
	CharacterID bigint,
	EquipmentCode tinyint,
	EquipmentAttributeCode tinyint,
	AttributeValue int
);

DECLARE @tblItemCooltimes table (
	ItemSerial bigint,
	CoolTime int
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	/**_# [P_SetQuickSlot]? ???? ? ?? ??? ?????.*/
	IF (@vchObjectTypeCodes + @vchObjectValues) IS NOT NULL
	BEGIN
		EXEC @intReturnValue = dbo.P_SetQuickSlot
			  @inbCharacterID = @inbCharacterID
			, @vchObjectTypeCodes = @vchObjectTypeCodes
			, @vchObjectValues = @vchObjectValues;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	/**_# ?? ? ??? ?????.*/
	IF (@vchSkillIDs + @vchSkillCoolTimes) > '' OR (@vchSkillIDs2 + @vchSkillCoolTimes2) > ''
	BEGIN
		INSERT @tblSkills (CharacterID, SkillCode, SkillID, CoolTime)
		SELECT @inbCharacterID, CAST(1 AS tinyint), CAST(A.string AS int), CAST(B.string AS int)
		FROM dbo.TF_Split(@vchSkillIDs, ',') A
			INNER JOIN dbo.TF_Split(@vchSkillCoolTimes, ',') B ON A.seq = B.seq
		WHERE A.string > '' AND B.string > ''
		UNION ALL
		SELECT @inbCharacterID, CAST(2 AS tinyint), CAST(A.string AS int), CAST(B.string AS int)
		FROM dbo.TF_Split(@vchSkillIDs2, ',') A
			INNER JOIN dbo.TF_Split(@vchSkillCoolTimes2, ',') B ON A.seq = B.seq
		WHERE A.string > '' AND B.string > ''
		OPTION (KEEPFIXED PLAN);

		WITH T AS (
			SELECT CharacterID, SkillCode, SkillID, CoolTime
			FROM dbo.Skills WITH (INDEX(PK_Skills), FORCESEEK)
			WHERE CharacterID = @inbCharacterID
		)
		MERGE T
		USING @tblSkills S
		ON T.CharacterID = S.CharacterID AND T.SkillID = S.SkillID AND T.SkillCode = S.SkillCode
		WHEN MATCHED THEN
			UPDATE SET CoolTime = S.CoolTime
		WHEN NOT MATCHED BY SOURCE AND T.CoolTime > 0 THEN
			UPDATE SET CoolTime = 0
		OPTION (KEEPFIXED PLAN, LOOP JOIN);
	END

	/**_# ?? ?? ?? ?????.*/
	IF (@vchEquipmentCodes + @vchEquipmentAttributeCode + @vchAttributeValues) IS NULL OR (@vchEquipmentCodes + @vchEquipmentAttributeCode + @vchAttributeValues) = ''
		DELETE dbo.EquipmentAttributes
		FROM dbo.EquipmentAttributes WITH (INDEX(PK_EquipmentAttributes), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);

	ELSE
	BEGIN
		INSERT @tblEquipmentAttributes (CharacterID, EquipmentCode, EquipmentAttributeCode, AttributeValue)
		SELECT @inbCharacterID, CAST(A.string AS tinyint), CAST(B.string AS tinyint), CAST(C.string AS int)
		FROM dbo.TF_Split(@vchEquipmentCodes, ',') A
			INNER jOIN dbo.TF_Split(@vchEquipmentAttributeCode, ',') B ON A.seq = B.seq
			INNER JOIN dbo.TF_Split(@vchAttributeValues, ',') C ON A.seq = C.seq
		WHERE A.string > '' AND B.string > '' AND C.string > ''
		OPTION (KEEPFIXED PLAN);

		WITH T AS (
			SELECT CharacterID, EquipmentCode, EquipmentAttributeCode, AttributeValue
			FROM dbo.EquipmentAttributes WITH (INDEX(PK_EquipmentAttributes), FORCESEEK)
			WHERE CharacterID = @inbCharacterID
		)
		MERGE T
		USING @tblEquipmentAttributes S
		ON T.CharacterID = S.CharacterID AND T.EquipmentCode = S.EquipmentCode AND T.EquipmentAttributeCode = S.EquipmentAttributeCode
		WHEN MATCHED THEN
			UPDATE SET AttributeValue = S.AttributeValue
		WHEN NOT MATCHED BY TARGET THEN
			INSERT (CharacterID, EquipmentCode, EquipmentAttributeCode, AttributeValue)
			VALUES (@inbCharacterID, S.EquipmentCode, S.EquipmentAttributeCode, S.AttributeValue)
		WHEN NOT MATCHED BY SOURCE THEN
			DELETE
		OPTION (KEEPFIXED PLAN, LOOP JOIN);
	END

	IF (@vchItemSerials + @vchItemCoolTimes) > ''
	BEGIN
		INSERT @tblItemCooltimes (ItemSerial, CoolTime)
		SELECT CAST(A.string AS bigint), CAST(B.string AS int)
		FROM dbo.TF_Split(@vchItemSerials, ',') A
			INNER jOIN dbo.TF_Split(@vchItemCoolTimes, ',') B ON A.seq = B.seq;
	
		UPDATE dbo.MaterializedItems
		SET CoolTime = A.CoolTime
		FROM @tblItemCooltimes A
			INNER JOIN dbo.MaterializedItems B WITH(INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON B.ItemSerial = A.ItemSerial;

	END

	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET LastMapID = @intLastMapID
		, LastVillageMapID = ISNULL(@intLastVillageMapID, LastVillageMapID)
		, LastDarkLairVillageMapID = ISNULL(@intLastDarkLairVillageMapID, LastDarkLairVillageMapID)
		, LastVillageGateNumber = ISNULL(@inyLastVillageGateNumber, LastVillageGateNumber)
		, PositionX = @intPositionX
		, PositionY = @intPositionY
		, PositionZ = @intPositionZ
		, Rotate = @fltRotate
		, ViewCashEquipmentBitmap = ISNULL(@intViewCashEquipmentBitmap, ViewCashEquipmentBitmap)
	FROM dbo.CharacterStatus WITH (INDEX(PK_CharacterStatus), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	IF @bitExecuteScheduledTask = 1
	BEGIN
		/**_# ??? ??? ??? ??? ?????.*/
		SELECT @dt2CreateDate = CreateDate, @inyAccountLevelCode = AccountLevelCode
		FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);
	
		IF @inyAccountLevelCode <> 110
		BEGIN
			INSERT @tblMailSchedules (MailScheduleID, SendDate, SenderName, [Subject], Content, AttachCoin, NewFlag, MailLifespan, MailTabCode)
			SELECT MS.MailScheduleID, MS.SendDate, MS.SenderName, MS.[Subject], MS.Content, MS.AttachCoin, MS.NewFlag, MS.MailLifespan, MS.MailTabCode
			FROM dbo.MailSchedules MS WITH (NOLOCK, INDEX(IX_NNF_MailSchedules_SendDate), FORCESEEK)
			WHERE MS.MailTypeCode = 0
				AND MS.DeleteFlag = CAST(0 AS bit)
				AND MS.SendDate >= @dt2CreateDate
				AND MS.SendDate <= GETDATE()
				AND ISNULL(MS.MailScheduleExpireDate, DATEADD(DAY, 30, SendDate)) >= GETDATE()
				AND NOT EXISTS (
					SELECT *
					FROM dbo.SendedMails SM WITH (NOLOCK, INDEX(PK_SendedMails), FORCESEEK)
					WHERE SM.MailScheduleID = MS.MailScheduleID
						AND SM.CharacterID = @inbCharacterID
				)
			UNION ALL
			SELECT MS.MailScheduleID, MS.SendDate, MS.SenderName, MS.[Subject], MS.Content, MS.AttachCoin, MS.NewFlag, MS.MailLifespan, MS.MailTabCode
			FROM dbo.MailSchedulesTarget MT WITH (NOLOCK, INDEX(IX_NNF_MailSchedulesTarget_CharacterID), FORCESEEK)
				INNER LOOP JOIN dbo.MailSchedules MS WITH (NOLOCK, INDEX(PK_MailSchedules), FORCESEEK) ON MT.MailScheduleID = MS.MailScheduleID
			WHERE MT.CharacterID = @inbCharacterID
				AND MT.DeleteFlag = CAST(0 AS bit)
				AND MS.MailTypeCode > 0
				AND MS.DeleteFlag = CAST(0 AS bit)
				AND MS.SendDate >= @dt2CreateDate
				AND MS.SendDate <= GETDATE()
				AND ISNULL(MS.MailScheduleExpireDate, DATEADD(DAY, 30, SendDate)) >= GETDATE()
				AND NOT EXISTS (
					SELECT *
					FROM dbo.SendedMails SM WITH (NOLOCK, INDEX(PK_SendedMails), FORCESEEK)
					WHERE SM.MailScheduleID = MS.MailScheduleID
						AND SM.CharacterID = @inbCharacterID
				);

			SET @j = @@ROWCOUNT;
		END

		WHILE @i <= @j
		BEGIN
			DELETE @tblItemSerials;
			DELETE @tblAttachItems;

			SELECT @intMailScheduleID = MailScheduleID, @dtsSendDate = SendDate, @nvcSenderName = SenderName, @nvcSubject = [Subject], @nvcContent = Content, @inbAttachCoin = AttachCoin, @bitNewFlag = NewFlag, @intMailLifespan = MailLifespan, @inyMailTabCode = MailTabCode
			FROM @tblMailSchedules
			WHERE Seq = @i;
			INSERT @tblAttachItems (ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, DragonJewelType)
			SELECT ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, DragonJewelType
			FROM dbo.AttachItemsForMailSchedule WITH (NOLOCK)
			WHERE MailScheduleID = @intMailScheduleID;

			INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, [Subject], Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, NewFlag, MailExpireDate, MailTabCode)
			VALUES (NULL, @inbCharacterID, 3, 2, @nvcSubject, @nvcContent, @dtsSendDate, 0, 0, @inbAttachCoin, 0, @nvcSenderName, 0, @bitNewFlag, DATEADD(MINUTE, CASE WHEN @intMailLifespan IS NULL THEN 43200 ELSE @intMailLifespan END, @dt2Now), @inyMailTabCode)

			SET @inbMailID = SCOPE_IDENTITY();

			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ItemLocationCode, DragonJewelType)
			OUTPUT inserted.ItemSerial, inserted.ItemID, inserted.ItemRemainCount INTO @tblItemSerials (ItemSerial, ItemID, ItemCount)
			SELECT ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint)), ItemID, ItemCount, NULL, 10, @inbMailID, ItemDurability, RandomSeed, 0, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, @dt2Now, ItemLifespan, EternityFlag, NULL, NULL, NULL, 15, DragonJewelType
			FROM @tblAttachItems;

			/**_# [MailAttachItems] ???? INSERT???.*/
			INSERT dbo.MailAttachItems (MailID, ItemSerial, ItemID, FirstAttachItemFlag, ReceiveFlag, AttachItemCount)
			SELECT @inbMailID, ItemSerial, ItemID, CASE Seq WHEN 1 THEN 1 ELSE 0 END, 0, ItemCount
			FROM @tblItemSerials;

			/**_# [ItemLogs] ???? INSERT???.*/
			IF @intLogTableSuffix = 0
				INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, T.ItemSerial, T.ItemID, 10, NULL, NULL, T.ItemCount, 0, @inbMailID, NULL, NULL, NULL
				FROM @tblItemSerials T
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 10));

			ELSE IF @intLogTableSuffix = 1
				INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, T.ItemSerial, T.ItemID, 10, NULL, NULL, T.ItemCount, 0, @inbMailID, NULL, NULL, NULL
				FROM @tblItemSerials T
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 10));

			ELSE
				INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, T.ItemSerial, T.ItemID, 10, NULL, NULL, T.ItemCount, 0, @inbMailID, NULL, NULL, NULL
				FROM @tblItemSerials T
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 10));

			/**_# [SendedMails] ???? INSERT???.*/
			INSERT dbo.SendedMails (CharacterID, MailScheduleID, SendDate)
			VALUES (@inbCharacterID, @intMailScheduleID, GETDATE());

			SET @i += 1;
		END

		/**_# ??? ??? ???? ?????.*/
		IF @dt2Now < DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0))) AND @dt2Now > DATEADD(DAY, -1, DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0))))
		BEGIN
			SET @dtmPrevDate = DATEADD(DAY, -1, DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0))))
			SET @dtmLaterDate = DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0)))
		END
		IF @dt2Now >= DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0))) AND @dt2Now < DATEADD(DAY, 1, DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0))))
		BEGIN
			SET @dtmPrevDate = DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0)))
			SET @dtmLaterDate = DATEADD(DAY, 1, DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0))))
		END
		SELECT @sdtEventFatigueGainDate = EventFatigueGainDate FROM dbo.CharacterStatus WITH (NOLOCK) WHERE CharacterID = @inbCharacterID;

		INSERT @tblEventFatigueSchedules (EventFatigueScheduleID, GiveDate, FatigueValue, GiveFlag)
		SELECT TOP 1 EventFatigueScheduleID, GiveDate, FatigueValue, GiveFlag
		FROM dbo.EventFatigueSchedule FS WITH (NOLOCK)
		WHERE @sdtEventFatigueGainDate <= GiveDate    
			AND @dt2Now BETWEEN GiveDate AND DATEADD(DAY, 1, DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0)))) 
			AND GiveDate BETWEEN @dtmPrevDate AND @dtmLaterDate
			AND GiveFlag = 0
			AND GiveDate > @dt2CreateDate
			AND NOT EXISTS (SELECT * FROM dbo.GivenEventFatigue GF WITH (NOLOCK) WHERE GF.EventFatigueScheduleID = FS.EventFatigueScheduleID AND GF.CharacterID = @inbCharacterID)
		ORDER BY GiveDate;

		IF EXISTS (SELECT * FROM @tblEventFatigueSchedules)
		BEGIN
			DECLARE @intEventFatigue int;
			SELECT @intEventFatigue = FatigueValue FROM @tblEventFatigueSchedules;
			UPDATE dbo.CharacterStatus SET EventFatigue = @intEventFatigue, EventFatigueGainDate = @dt2Now WHERE CharacterID = @inbCharacterID;
		END 
	END

	/**_# ??? ??? ???? ?????.*/
	EXEC dbo.P_ExpireEffectItem @inbCharacterID = @inbCharacterID;
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
