USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_MakeGiftByDOLIS]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_MakeGiftByDOLIS]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_MakeGiftByDOLIS] AS' 
END
GO
/**
version : 15
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-27
description : DOLIS?? ?? ???? ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101170 = @vchCertKey? ?? ????.
101172 = ?? ??? ??? ???? ????.
history :
2 = ?? ?? ??? ?????. at 2010-10-11
3 = GiftRejectFlag ?? ??? ?? ?? at 2010-10-14
4 = ?? ?? ?? ?? at 2010-11-02
5 = while?? interval? ?? ?? ? ??? ?????? ?? at 2010-12-23
6 = while??? ??? ???? ??? ?? 50??? 200?? & interval? 0.5??? 0.006?? ?? at 2011-01-03
7 = intput param. @bitNewFlag ?? at 2011-04-06
8 = ?? ??? ?? ?? ?? (@inyReceiverTarget = 3)     by 20110502.wgkim
9 = ?? ??? ?? ?? ?? (@inyReceiverTarget = 3) ?? GiftTypeCode, PayMethodCode ??     by 20110503.wgkim
10 = ?? ??? ?? ?? ?? (@inyReceiverTarget = 3) ?? NewFlag ??     by 20110503.wgkim
13 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
14 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

ALTER PROCEDURE [dbo].[P_MakeGiftByDOLIS]
	  @inyReceiverTarget tinyint --// 1=P_StoreTargetCharacter? ???? ?? ??? ??, 2=??? ?? ??, 3=?? ??? ?? ??
	, @vchCertKey char(32) = NULL --// P_StoreTargetCharacter ??? ??? @vchCertKey ?. ???? ?? ???? ???? ?? ? ?? NULL? ?????.
	, @vchCharacterIDs varchar(max) --// ?? ?? ??? ID ???. ???? ,???. ???? ???? ???? NULL ?????.
	, @nvcCharacterNames nvarchar(max) --// ?? ?? ??? ?? ???. ???? ,???. ???? ???? ???? NULL ?????.
	, @nvcGiftSenderName nvarchar(50) --// ??? ?? ??
	, @intGiftLifespan int --// ?????? ?? ??. ?? : ?
	, @vchIP varchar(15) --// ????? ?? IP
	, @intProductID int --// ?? ID
	, @vchSelectedItemIDs varchar(8000) --// ,? ?????? ??? ID ???.
	, @nvcGiftMemo nvarchar(100) --// ?? ?? ????? ??? ??? ??.
	, @bitNewFlag bit = 0 --// ??? ??? ??? ??
	, @dt2ReserveSendDate datetime2 = NULL --// ???? ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE(), @inbCharacterID bigint = 0, @i int = 1, @nvcCollationName nvarchar(128);
DECLARE @tblItems table (ItemID int NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblItems (ItemID) SELECT CAST(string AS int) FROM dbo.TF_Split(@vchSelectedItemIDs, ',') WHERE string > '';
	CREATE TABLE #tmp1 (PurchaseOrderID bigint NOT NULL PRIMARY KEY, ReceiverCharacterID bigint NOT NULL);
	CREATE TABLE #tmp2 (PurchaseOrderDetailID bigint NOT NULL PRIMARY KEY);
	CREATE TABLE #tmp3 (Seq int NOT NULL IDENTITY(1, 1) PRIMARY KEY, ReceiverCharacterID bigint NOT NULL);

	IF @inyReceiverTarget = 1
	BEGIN
		IF OBJECT_ID(N'TargetCharacters', N'U') IS NULL OR OBJECT_ID(N'TargetCharacterDetails', N'U') IS NULL
		BEGIN
			SET @intReturnValue = 101172;
			GOTO ErrorHandler;
		END

		IF NOT EXISTS (SELECT * FROM dbo.TargetCharacters WHERE CertKey = @vchCertKey)
		BEGIN
			SET @intReturnValue = 101170;
			GOTO ErrorHandler;
		END

		WHILE 1 = 1
		BEGIN
			TRUNCATE TABLE #tmp1;
			TRUNCATE TABLE #tmp2;

			BEGIN TRANSACTION;

			INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, OrderResult, OrderRollbackResult, GiftReceiveFlag, GiftMemo, GiftExpireDate, GiftSenderName, GiftRejectFlag, NewFlag)
			OUTPUT inserted.PurchaseOrderID, inserted.ReceiverCharacterID INTO #tmp1 (PurchaseOrderID, ReceiverCharacterID)
			SELECT NULL, CharacterID, 1, 0, 0, dbo.FN_IP2IPNumber(@vchIP), @dt2Now, 2, 5, NULL, NULL, NULL, 0, @nvcGiftMemo, DATEADD(MINUTE, @intGiftLifespan, @dt2Now), @nvcGiftSenderName, 0, @bitNewFlag
			FROM dbo.TargetCharacterDetails WITH (NOLOCK)
			WHERE Seq >= @i AND Seq < (@i + 200) AND CompleteFlag = CAST(0 AS bit)
			OPTION (MAXDOP 1);

			IF @@ROWCOUNT = 0
			BEGIN
				COMMIT TRANSACTION;
				BREAK;
			END
			INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
			OUTPUT inserted.PurchaseOrderDetailID INTO #tmp2 (PurchaseOrderDetailID)
			SELECT PurchaseOrderID, @intProductID, 0
			FROM #tmp1
			OPTION (MAXDOP 1);
			INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, GiveFailFlag)
			SELECT T1.PurchaseOrderDetailID, T2.ItemID, 0
			FROM #tmp2 T1
				INNER JOIN @tblItems T2 ON 1 = 1
			OPTION (MAXDOP 1);

			UPDATE dbo.TargetCharacterDetails
			SET CompleteFlag = 1
			WHERE Seq >= @i AND Seq < (@i + 200)
			OPTION (MAXDOP 1)

			COMMIT TRANSACTION;

			WAITFOR DELAY '00:00:00.003';

			SET @i += 200;
		END

		DROP VIEW dbo.VIX_TargetCharacterDetails;
		DROP TABLE dbo.TargetCharacters;
		DROP TABLE dbo.TargetCharacterDetails;
	END

	ELSE IF @inyReceiverTarget = 2
	BEGIN
		IF @vchCharacterIDs > ''
			INSERT #tmp3 (ReceiverCharacterID)
			SELECT CH.CharacterID
			FROM dbo.TF_Split(@vchCharacterIDs, ',') F
				INNER JOIN dbo.Characters CH WITH (NOLOCK, FORCESEEK) ON CAST(F.string AS bigint) = CH.CharacterID
			WHERE CH.DeleteFlag = CAST(0 AS bit) AND F.string > '';

		ELSE
			INSERT #tmp3 (ReceiverCharacterID)
			SELECT CH.CharacterID
			FROM dbo.TF_NSplit(@nvcCharacterNames, N',') F
				INNER JOIN dbo.Characters CH WITH (NOLOCK, FORCESEEK) ON F.string = CH.CharacterName
			WHERE CH.DeleteFlag = CAST(0 AS bit) AND F.string > '';

		WHILE 1 = 1
		BEGIN
			TRUNCATE TABLE #tmp1;
			TRUNCATE TABLE #tmp2;

			BEGIN TRANSACTION;

			INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, OrderResult, OrderRollbackResult, GiftReceiveFlag, GiftMemo, GiftExpireDate, GiftSenderName, GiftRejectFlag, NewFlag)
			OUTPUT inserted.PurchaseOrderID, inserted.ReceiverCharacterID INTO #tmp1 (PurchaseOrderID, ReceiverCharacterID)
			SELECT NULL, ReceiverCharacterID, 1, 0, 0, dbo.FN_IP2IPNumber(@vchIP), @dt2Now, 2, 5, NULL, NULL, NULL, 0, @nvcGiftMemo, DATEADD(MINUTE, @intGiftLifespan, @dt2Now), @nvcGiftSenderName, 0, @bitNewFlag
			FROM #tmp3
			WHERE Seq >= @i AND Seq < (@i + 200)
			OPTION (MAXDOP 1);

			IF @@ROWCOUNT = 0
			BEGIN
				COMMIT TRANSACTION;
				BREAK;
			END

			INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
			OUTPUT inserted.PurchaseOrderDetailID INTO #tmp2 (PurchaseOrderDetailID)
			SELECT PurchaseOrderID, @intProductID, 0
			FROM #tmp1
			OPTION (MAXDOP 1);

			INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, GiveFailFlag)
			SELECT T1.PurchaseOrderDetailID, T2.ItemID, 0
			FROM #tmp2 T1
				INNER JOIN @tblItems T2 ON 1 = 1
			OPTION (MAXDOP 1);

			COMMIT TRANSACTION;

			WAITFOR DELAY '00:00:00.006';

			SET @i += 200;
		END
	END

	ELSE IF @inyReceiverTarget = 3
	BEGIN

		INSERT dbo.GiftSendSchedule (GiftExpireDate, GiftSendDate, GiftTypeCode, GiftSenderName, GiftLifespan, IP, ProductID, SelectedItemIDs, GiftMemo, PayMethodCode, NewFlag)
		SELECT DATEADD(MI, @intGiftLifespan, COALESCE(@dt2ReserveSendDate, @dt2Now)), COALESCE(@dt2ReserveSendDate, @dt2Now), 0, @nvcGiftSenderName, @intGiftLifespan, @vchIP, @intProductID, @vchSelectedItemIDs, @nvcGiftMemo, 5, @bitNewFlag

	END

	DROP TABLE #tmp1, #tmp2, #tmp3;
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
