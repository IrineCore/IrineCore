USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_LoginCharacter2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ???? ???? ???? ???? ?? ??     BY 20110929.wgkim
3 = ????? ?? ?? ??	BY 20120102.dailykim
4 = ItemLog ???? SkipItem ?? ?? ??     by ??? at 2012-01-15
5 = ???? ???? ??? ?? ?? by ??? at 2012-06-19
6 = DWC ???? ???? ??? ??? ?? by ??? at 2013-02-19
7~8 = ??? ?? ?? ?? ??? ?? ?? by ??? at 2013-04-03
9 = ???? ???? ?? ?? by ??? at 2013-07-23
**/

CREATE PROCEDURE [dbo].[P_LoginCharacter2]
	  @inbCharacterID bigint --// ??? ID
	, @intSessionID int --// ?? ID
	, @vchIP varchar(15) --// IP ??
	, @bitPCRoomFlag bit = 0 --// 0=????, 1=?????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intLogTableSuffix int
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

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	UPDATE dbo.CharacterStatus
	SET LastSessionID = @intSessionID
		, LastLoginIP = dbo.FN_IP2IPNumber(@vchIP)
		, LastLoginDate = @dt2Now
	WHERE CharacterID = @inbCharacterID;

	UPDATE dbo.TeacherCharacters
	SET ConnectFlag = 1
	WHERE TeacherCharacterID = @inbCharacterID;	

	/**_# ??? ??? ??? ??? ?????. DWC ???? ?? ??*/
	SELECT @dt2CreateDate = CreateDate, @inyAccountLevelCode = AccountLevelCode
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

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

	/**_# ??? ??? ???? ??? ???? ?????. */
	
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

	/**_# ??? ??? ??? ?????.*/
	IF @bitPCRoomFlag IS NOT NULL
	BEGIN
		IF @intLogTableSuffix = 0
			INSERT dbo.CharacterLoginLogs0 (LoginDate, CharacterID, LoginIP, PCRoomFlag)
			VALUES (@dt2Now, @inbCharacterID, dbo.FN_IP2IPNumber(@vchIP), @bitPCRoomFlag);
			
		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.CharacterLoginLogs1 (LoginDate, CharacterID, LoginIP, PCRoomFlag)
			VALUES (@dt2Now, @inbCharacterID, dbo.FN_IP2IPNumber(@vchIP), @bitPCRoomFlag);

		ELSE
			INSERT dbo.CharacterLoginLogs2 (LoginDate, CharacterID, LoginIP, PCRoomFlag)
			VALUES (@dt2Now, @inbCharacterID, dbo.FN_IP2IPNumber(@vchIP), @bitPCRoomFlag);
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
