USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelMail]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 18
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-28
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
8 = SkipLoggingItems ???? ???? ?? ??? ????? ?? at 2010-08-09
9 = ??? ???? ?? ?? ?? ?? at 2010-10-06
10 = ???? at 2011-03-29
14 = 10?? ??     by ??? at 2012-04-09
15 = ??? ?? ?? by ??? at 2012-07-17
16 = ??? ??? ???? ?? ?? ?? ?? ?? by ??? at 2013-03-25
**/

CREATE PROCEDURE [dbo].[P_DelMail]
	  @inbReceiverCharacterID bigint --// ?? ?? ??? ID
	, @vchMailIDs varchar(8000) --// ,? ?????? ??? ?? ID ???
	, @intTotalMailCount int OUTPUT --// ??? ?? ? ?? ?? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intLogTableSuffix int
	, @dt2Now datetime2(0) = GETDATE();

DECLARE @tblMailIDs table (MailID int NOT NULL PRIMARY KEY);
DECLARE @tblDeletedItems table (ItemSerial bigint NOT NULL, ItemID int NOT NULL, ItemRemainCount smallint NOT NULL, PayMethodCode tinyint NULL);
DECLARE @tblDeletedDragonJewelIDs table (MaterializedItemID bigint NOT NULL);
DECLARE @tblDeletedDragonJewels table (ItemSerial bigint NOT NULL, ItemID int NOT NULL, ItemRemainCount smallint NOT NULL, PayMethodCode tinyint NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblMailIDs (MailID)
	SELECT CAST(SUBSTRING(',' + @vchMailIDs + ',', n + 1, CHARINDEX(',', ',' + @vchMailIDs + ',', n + 1) - n - 1) AS int)
	FROM dbo.Tally WITH (NOLOCK)
	WHERE n < DATALENGTH(',' + @vchMailIDs + ',') AND SUBSTRING(',' + @vchMailIDs + ',', n, 1) = ',' AND SUBSTRING(',' + @vchMailIDs + ',', n + 1, CHARINDEX(',', ',' + @vchMailIDs + ',', n + 1) - n - 1) > '';

	/**_# Mails.DeleteFlag? 1? UPDATE???.*/
	UPDATE dbo.Mails
	SET DeleteFlag = 1, DeleteDate = @dt2Now
	FROM dbo.Mails M
		INNER JOIN @tblMailIDs T ON M.MailID = T.MailID
	WHERE ReceiverCharacterID = @inbReceiverCharacterID AND DeleteFlag = CAST(0 AS bit);

	IF @@ROWCOUNT > 0
	BEGIN
		DELETE dbo.MaterializedItems
		OUTPUT deleted.ItemSerial, deleted.ItemID, deleted.ItemRemainCount, deleted.PayMethodCode
		INTO @tblDeletedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode)
		FROM @tblMailIDs T
			INNER JOIN dbo.MailAttachItems MAI WITH (NOLOCK, INDEX(PK_MailAttachItems), FORCESEEK) ON MAI.MailID = T.MailID
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON MAI.ItemSerial = MI.ItemSerial
		WHERE MAI.ReceiveFlag = CAST(0 AS bit);

		DELETE dbo.PetEquipments
		FROM @tblDeletedItems A INNER JOIN dbo.PetEquipments P ON A.ItemSerial = P.PetItemSerial;		

		SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);		

		DELETE dbo.DragonJewels
		OUTPUT deleted.MaterializedItemID
		INTO @tblDeletedDragonJewelIDs (MaterializedItemID)
		FROM @tblDeletedItems A 
			INNER JOIN dbo.DragonJewels DJ WITH (NOLOCK, INDEX(IX_UNI_DragonJewels_ParentItemSerial_DragonJewelLocationIndex)) ON A.ItemSerial = DJ.ParentItemSerial;

		IF @@ROWCOUNT > 0
		BEGIN
			DELETE dbo.MaterializedItems
			OUTPUT deleted.ItemSerial, deleted.ItemID, deleted.ItemRemainCount, deleted.PayMethodCode
			INTO @tblDeletedDragonJewels (ItemSerial, ItemID, ItemRemainCount, PayMethodCode)
			FROM @tblDeletedDragonJewelIDs A 
				 INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(PK_MaterializedItems), FORCESEEK) ON A.MaterializedItemID = MI.MaterializedItemID;

			IF @intLogTableSuffix = 0
				INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL
				FROM @tblDeletedDragonJewels T
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));

			ELSE IF @intLogTableSuffix = 1
				INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL
				FROM @tblDeletedDragonJewels T
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));

			ELSE
				INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL
				FROM @tblDeletedDragonJewels T
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));
		END		

		IF @intLogTableSuffix = 0
			INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL
			FROM @tblDeletedItems T
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));

		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL
			FROM @tblDeletedItems T
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));

		ELSE
			INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, T.ItemSerial, T.ItemID, 26, NULL, NULL, 0, ItemRemainCount, NULL, NULL, NULL, NULL
			FROM @tblDeletedItems T
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 26));
	END

	SELECT @intTotalMailCount = COUNT(*)
	FROM dbo.Mails WITH (NOLOCK, INDEX(IX_NNI_Mails_ReceiverCharacterID), FORCESEEK)
	WHERE ReceiverCharacterID = @inbReceiverCharacterID AND DeleteFlag = CAST(0 AS bit) AND SendDate >= CAST(CAST(DATEADD(DAY, -29, @dt2Now) AS date) AS datetime2(0));

	IF @@ROWCOUNT = 0
		SET @intTotalMailCount = 0;

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
