USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SendSNDAEventItemMail]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-27
description : ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103123 = ??? ?????.
103161 = ??? ??? ??? ?? ????.
103162 = ?? ??? ???? ???? ????.
103163 = ?? ?? ???? ???? ????.
103164 = GM ?? DWC ?????? ??? ?? ? ????.
103421 = ?? ???? ??? ?????.
103422 = ????? ????? ????. (?????? ??)
103420 = ?? ?? ??? ???? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_SendSNDAEventItemMail]
	  @inbSenderCharacterID bigint --// ?? ??? ID
	, @nvcReceiverCharacterName nvarchar(30) --// ?? ??? ??
	, @inyDeliveryTypeCode tinyint --// 1=??, 2=????, 3=?? ??
	, @nvcSubject nvarchar(50) --// ??
	, @nvcContent nvarchar(300) --// ??
	, @inbAttachCoin bigint --// ?? ??

	, @vchAttachItemSerials varchar(8000) --// ???? ???? ItemSerial ?? ,? ???? ??. ?) 1,2,3
	, @vchAttachNewItemSerials varchar(8000) --// ???? ?? ??? ???? ?? ?? ???? ? ItemSerial ?? ,? ???? ??. ?? ??? ?? ??? 0? ????. ?) 0,4,0
	, @vchAttachItemCount varchar(8000) --// ,? ?????? ?? ??? ?? ???. ?) 10,1,5

	, @inbSendMailFee bigint --// ?? ???
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID

	, @inbReceiverCharacterID bigint OUTPUT --// ?? ??? ID
	, @intReceiverAccountID int OUTPUT --// ?? ???? ?? ID
	, @intTotalMailCount int OUTPUT --// ? ?? ?? ?
	, @intNotReadMailCount int OUTPUT --// ?? ?? ?? ?
	, @int7DaysLeftMailCount int OUTPUT --// ?? ??? 7? ?? ?? ?
	, @intBasicMailCount int OUTPUT --// ?? ?? ?? ?? ?

	, @intMailLifespan int = 43200 --// ?? ???? (?, ?? 30?)	
	, @bitAttachCashItem bit = 0 --// ????? ?? ?? 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inyAccountLevelCode tinyint
	, @intAttachItemCount int = 0
	, @intLogTableSuffix int
	, @intMailID int
	, @intPremiumMailCount int
	, @dt2LastLoginDate datetime2(0)
	, @dt2FirstCharacterCreateDate datetime2(0)
	, @inyCharacterLevel tinyint
	, @bitReturnCharacter bit = CAST (0 AS bit)
	, @bitNewbieCharacter bit = CAST (0 AS bit);
	
DECLARE @tblTempA table (Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemSerial bigint NOT NULL);
DECLARE @tblTempB table (Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, ItemCount smallint NOT NULL);
DECLARE @tblTempC table (Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY, NewItemSerial bigint NOT NULL);
DECLARE @tblNewItems table (NewItemSerial bigint NOT NULL, ItemCount smallint NOT NULL, OldMaterializedItemID bigint NOT NULL, NewMaterializedItemID bigint NULL);

DECLARE @tblAttachItems table (
	Seq tinyint IDENTITY(1, 1) NOT NULL,
	ItemSerial bigint NOT NULL,
	NewItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	AttachItemCount smallint NOT NULL,
	OriginalItemCount smallint NOT NULL,
	OwnershipStartDate datetime2(0) NULL,
	MaterializedItemID bigint NOT NULL PRIMARY KEY
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ?? ??? ??? ?????.*/
	SELECT @inbReceiverCharacterID = CharacterID, @inyAccountLevelCode = AccountLevelCode, @intReceiverAccountID = AccountID
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterName = @nvcReceiverCharacterName AND DeleteFlag = CAST(0 AS bit);

	/**_## ?? ?? ???? ???? ??? 103163? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103163;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ???? GM??? DWC????? 103164? ?????.*/
	IF (@inyAccountLevelCode BETWEEN 10 AND 39) OR @inyAccountLevelCode = 110
	BEGIN
		SET @intReturnValue = 103164;
		GOTO ErrorHandler;
	END
	
	/**_# ????? ?????? ?????.*/
	SELECT @inyCharacterLevel = CharacterLevel
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbReceiverCharacterID;

	SELECT @dt2FirstCharacterCreateDate = MIN(C.CreateDate)
		 , @dt2LastLoginDate = MAX(CS.LastLoginDate)
	  FROM dbo.Characters C WITH (NOLOCK)
		   INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON (C.CharacterID = CS.CharacterID)
	 WHERE AccountID = @intReceiverAccountID;
	
	IF @dt2LastLoginDate < '2013-08-09' AND @inyCharacterLevel >= 40
		SET @bitReturnCharacter = CAST(1 AS bit);
	ELSE
		SET @bitReturnCharacter = CAST(0 AS bit);

	IF @dt2FirstCharacterCreateDate >= '2013-09-12'
		SET @bitNewbieCharacter = CAST(1 AS bit);
	ELSE
		SET @bitNewbieCharacter = CAST(0 AS bit);
	
	IF @bitNewbieCharacter = CAST(0 AS bit) AND @bitReturnCharacter = CAST(0 AS bit)
	BEGIN
		SET @intReturnValue = 103422;
		GOTO ErrorHandler;
	END
	
	/**_# ?? ???? ??? ????? ?????.*/
	IF EXISTS (SELECT * FROM dbo.SNDAEventItemHistory WITH (NOLOCK) WHERE SenderCharacterID = @inbSenderCharacterID AND ReceiveCharacterID = @inbReceiverCharacterID)
	BEGIN
		SET @intReturnValue = 103421;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ??? ???? ???? ?? */
	IF EXISTS (SELECT * FROM dbo.SNDAEventItemHistory WITH (NOLOCK) WHERE ReceiveCharacterID = @inbReceiverCharacterID AND ReceiveFlag = CAST (1 AS bit))
	BEGIN
		SET @intReturnValue = 103420;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ??? ???? ??? ??? (???, ?? ?) 103161? ?????.*/
	INSERT @tblTempA (ItemSerial)
	SELECT CAST(string AS bigint)
	FROM dbo.TF_Split(@vchAttachItemSerials, ',')
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);
	SET @intAttachItemCount = @@ROWCOUNT

	IF @intAttachItemCount > 0
	BEGIN
		INSERT @tblTempB (ItemCount)
		SELECT CAST(string AS smallint)
		FROM dbo.TF_Split(@vchAttachItemCount, ',')
		WHERE string > ''
		OPTION (KEEPFIXED PLAN);

		INSERT @tblTempC (NewItemSerial)
		SELECT CAST(string AS bigint)
		FROM dbo.TF_Split(@vchAttachNewItemSerials, ',')
		WHERE string > ''
		OPTION (KEEPFIXED PLAN);

		INSERT @tblAttachItems (ItemSerial, NewItemSerial, ItemID, AttachItemCount, OwnershipStartDate, OriginalItemCount, MaterializedItemID)
		SELECT A.ItemSerial, CASE C.NewItemSerial WHEN 0 THEN A.ItemSerial ELSE C.NewItemSerial END, MI.ItemID, B.ItemCount, MI.OwnershipStartDate, MI.ItemRemainCount, MI.MaterializedItemID
		FROM @tblTempA A
			INNER JOIN @tblTempB B ON A.Seq = B.Seq
			INNER JOIN @tblTempC C ON A.Seq = C.Seq
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON A.ItemSerial = MI.ItemSerial
		WHERE MI.OwnerCharacterID = @inbSenderCharacterID AND MI.ItemRemainCount >= B.ItemCount
		OPTION (FORCE ORDER, LOOP JOIN);

		IF @intAttachItemCount <> @@ROWCOUNT
		BEGIN
			SET @intReturnValue = 103161;
			GOTO ErrorHandler;
		END
	END

	BEGIN TRANSACTION;

	/**_# [Mails] ???? INSERT???.*/
	INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, [Subject], Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, MailExpireDate, MailTabCode)
	VALUES (@inbSenderCharacterID, @inbReceiverCharacterID, 1, @inyDeliveryTypeCode, @nvcSubject, @nvcContent, @dt2Now, 0, 0, @inbAttachCoin, 0, DATEADD(MINUTE, @intMailLifespan, @dt2Now), 1);

	SET @intMailID = SCOPE_IDENTITY();

	/**_# ??? ??? ?? ?? ???? ?? ??? ?????.*/
	/**_## ?? ??? ???? 103123? ?????.*/
	IF @inbAttachCoin > 0
	BEGIN
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbSenderCharacterID
			, @inyCoinChangeCode = 6
			, @inbCoinChangeFKey = @intMailID
			, @inbCoin = @inbAttachCoin
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END
	
	/**_# [SNDAEventItemHistory] ???? INSERT???.*/
	INSERT dbo.SNDAEventItemHistory (SenderCharacterID, ReceiveCharacterID, ReceiveFlag, SendDate, ReceiveDate, MailID)
	VALUES (@inbSenderCharacterID, @inbReceiverCharacterID, 0, @dt2Now, NULL, @intMailID);

	COMMIT TRANSACTION;

	IF @intAttachItemCount > 0
	BEGIN
		INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ItemLocationCode, ItemLocationIndex)
		OUTPUT inserted.ItemMaterializeFKey, inserted.ItemSerial, inserted.ItemRemainCount, inserted.MaterializedItemID
		INTO @tblNewItems (OldMaterializedItemID, NewItemSerial, ItemCount, NewMaterializedItemID)
		SELECT T.NewItemSerial, MI.ItemID, T.AttachItemCount, MI.SenderCharacterID, MI.ItemMaterializeCode, T.MaterializedItemID, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.ItemMaterializeDate, MI.ItemLifespan, MI.EternityFlag
			 , CASE WHEN @bitAttachCashItem = 1 AND MI.EternityFlag = 0 THEN NULL 
					ELSE MI.ItemExpireDate END
			 , NULL, NULL, 15, NULL
		FROM dbo.MaterializedItems MI WITH (NOLOCK)
			INNER JOIN @tblAttachItems T ON MI.MaterializedItemID = T.MaterializedItemID
		WHERE T.ItemSerial <> T.NewItemSerial;

		IF @@ROWCOUNT > 0
			UPDATE dbo.MaterializedItems
			SET ItemRemainCount -= T.ItemCount
			FROM dbo.MaterializedItems MI
				INNER JOIN @tblNewItems T ON MI.MaterializedItemID = T.OldMaterializedItemID;

		/**_# [MaterializedItems]???? UPDATE???.*/
		UPDATE dbo.MaterializedItems
		SET OwnerCharacterID = NULL
			, ItemLocationCode = 15
			, ItemLocationIndex = NULL
		FROM dbo.MaterializedItems MI WITH (NOLOCK)
			INNER JOIN @tblAttachItems T ON MI.MaterializedItemID = T.MaterializedItemID
		WHERE T.ItemSerial = T.NewItemSerial;

		/**_# [MailAttachItems] ???? INSERT???.*/
		INSERT dbo.MailAttachItems (MailID, ItemSerial, ReceiveFlag, ItemID, FirstAttachItemFlag, AttachItemCount)
		SELECT @intMailID, NewItemSerial, 0, ItemID, CASE Seq WHEN 1 THEN 1 ELSE 0 END, AttachItemCount 
		FROM @tblAttachItems 
		ORDER BY Seq;
		/**_# [ItemLogs] ???? INSERT???.*/
		SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);
		IF @intLogTableSuffix = 0
			INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, ItemSerial, ItemID, 35, @inbSenderCharacterID, @inbSenderCharacterID, OriginalItemCount - AttachItemCount, OriginalItemCount, NewItemSerial, NULL, NULL, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 35)) AND ItemSerial <> NewItemSerial
			UNION ALL
			SELECT @dt2Now, NewItemSerial, ItemID, 36, @inbSenderCharacterID, @inbSenderCharacterID, AttachItemCount, 0, ItemSerial, NULL, NULL, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 36)) AND ItemSerial <> NewItemSerial
			UNION ALL
			SELECT @dt2Now, NewItemSerial, ItemID, 30, NULL, @inbSenderCharacterID, AttachItemCount, AttachItemCount, @intMailID, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 30));

		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, ItemSerial, ItemID, 35, @inbSenderCharacterID, @inbSenderCharacterID, OriginalItemCount - AttachItemCount, OriginalItemCount, NewItemSerial, NULL, NULL, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 35)) AND ItemSerial <> NewItemSerial
			UNION ALL
			SELECT @dt2Now, NewItemSerial, ItemID, 36, @inbSenderCharacterID, @inbSenderCharacterID, AttachItemCount, 0, ItemSerial, NULL, NULL, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 36)) AND ItemSerial <> NewItemSerial
			UNION ALL
			SELECT @dt2Now, NewItemSerial, ItemID, 30, NULL, @inbSenderCharacterID, AttachItemCount, AttachItemCount, @intMailID, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 30));

		ELSE
			INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, ItemSerial, ItemID, 35, @inbSenderCharacterID, @inbSenderCharacterID, OriginalItemCount - AttachItemCount, OriginalItemCount, NewItemSerial, NULL, NULL, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 35)) AND ItemSerial <> NewItemSerial
			UNION ALL
			SELECT @dt2Now, NewItemSerial, ItemID, 36, @inbSenderCharacterID, @inbSenderCharacterID, AttachItemCount, 0, ItemSerial, NULL, NULL, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 36)) AND ItemSerial <> NewItemSerial
			UNION ALL
			SELECT @dt2Now, NewItemSerial, ItemID, 30, NULL, @inbSenderCharacterID, AttachItemCount, AttachItemCount, @intMailID, NULL, CASE ItemSerial WHEN NewItemSerial THEN OwnershipStartDate ELSE @dt2Now END, NULL
			FROM @tblAttachItems A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 30));
	END

	/**_# ?? ???? ?????.*/
	IF @inbSendMailFee > 0
		EXEC dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbSenderCharacterID
			, @inyCoinChangeCode = 20
			, @inbCoinChangeFKey = @intMailID
			, @inbCoin = @inbSendMailFee
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

	EXEC dbo.P_GetCountReceiveMail
		  @inbReceiverCharacterID = @inbReceiverCharacterID
		, @intTotalMailCount = @intTotalMailCount OUTPUT
		, @intNotReadMailCount = @intNotReadMailCount OUTPUT
		, @int7DaysLeftMailCount = @int7DaysLeftMailCount OUTPUT;

	EXEC dbo.P_GetCountSendMail
		  @inbSenderCharacterID = @inbSenderCharacterID
		, @intPremiumMailCount = @intPremiumMailCount OUTPUT
		, @intBasicMailCount = @intBasicMailCount OUTPUT;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_CharacterStatus_Coin%'
		SET @intReturnValue = 103123;	

	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
