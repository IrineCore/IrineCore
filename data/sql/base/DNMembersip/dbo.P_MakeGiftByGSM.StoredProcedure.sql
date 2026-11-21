USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_MakeGiftByGSM]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_MakeGiftByGSM]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_MakeGiftByGSM] AS' 
END
GO
/**
version : 16
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-09
description : GSM?? ?? ???? ???? ???? ?????.
result set : CharacterID/bigint, CharacterName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101155 = @inbCharacterIDs? ??? ?????.
101156 = @nvcCharacterNames? ??? ?????.
101157 = ???? ?? ??? ??? @nvcCharacterNames? ???? ????.
101158 = ???? ?? ??? ID? @inbCharacterID? ???? ????.
history :
7 = ?????? COLLATION? ???? ????? ??
8 = ????? COLLATION ?? ?? ??
9 = ?? ?? ?? ?? at 2010-10-11
10 = GiftRejectFlag ?? ??? ?? ?? at 2010-10-14
11 = ?? ?? ?? ?? at 2010-11-02
12 = intput param. @bitNewFlag ?? at 2011-04-06
13 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
14 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
15~16 = Option / Index Hint ??     by ??? at 2012-03-16
**/

ALTER PROCEDURE [dbo].[P_MakeGiftByGSM]
	  @inyReceiverTarget tinyint --// 1=??, 2=?? ??, 3=??? ??
	, @intWorldID int --// ?? ?? ?? ID. ??? ???? ???? NULL ?????.
	, @vchCharacterIDs varchar(max) --// ?? ?? ??? ID ???. ???? ,???. ???? ???? ???? NULL ?????.
	, @nvcCharacterNames nvarchar(max) --// ?? ?? ??? ?? ???. ???? ,???. ???? ???? ???? NULL ?????.
	, @nvcGiftSenderName nvarchar(50) --// ??? ?? ??
	, @intGiftLifespan int --// ?????? ?? ??. ?? : ?
	, @vchIP varchar(15) --// ????? ?? IP
	, @intProductID int --// ?? ID
	, @vchSelectedItemIDs varchar(8000) --// ,? ?????? ??? ID ???.
	, @nvcGiftMemo nvarchar(100) --// ?? ?? ????? ??? ??? ??.
	, @bitNewFlag bit = 0 --// ??? ??? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE(), @inbCharacterID bigint = 0, @i int, @nvcCollationName nvarchar(128);
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
	CREATE TABLE #tmp3 (ReceiverCharacterID bigint NOT NULL);

	IF @inyReceiverTarget IN (1, 2)
	BEGIN
		WHILE 1 = 1
		BEGIN
			TRUNCATE TABLE #tmp1;
			TRUNCATE TABLE #tmp2;

			BEGIN TRANSACTION;

			INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, OrderResult, OrderRollbackResult, GiftReceiveFlag, GiftMemo, GiftExpireDate, GiftSenderName, GiftRejectFlag, NewFlag)
			OUTPUT inserted.PurchaseOrderID, inserted.ReceiverCharacterID INTO #tmp1 (PurchaseOrderID, ReceiverCharacterID)
			SELECT TOP (1000) NULL, CharacterID, 1, 0, 0, dbo.FN_IP2IPNumber(@vchIP), @dt2Now, 2, 5, NULL, NULL, NULL, 0, @nvcGiftMemo, DATEADD(MINUTE, @intGiftLifespan, @dt2Now), @nvcGiftSenderName, 0, @bitNewFlag
			FROM dbo.Characters WITH (NOLOCK)
			WHERE DeleteFlag = CAST(0 AS bit) AND WorldID = COALESCE(@intWorldID, WorldID) AND CharacterID > @inbCharacterID
			ORDER BY CharacterID;

			IF @@ROWCOUNT = 0
			BEGIN
				ROLLBACK TRANSACTION;
				BREAK;
			END

			INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
			OUTPUT inserted.PurchaseOrderDetailID INTO #tmp2 (PurchaseOrderDetailID)
			SELECT PurchaseOrderID, @intProductID, 0
			FROM #tmp1;

			INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, GiveFailFlag)
			SELECT T1.PurchaseOrderDetailID, T2.ItemID, 0
			FROM #tmp2 T1
				INNER JOIN @tblItems T2 ON 1 = 1;

			COMMIT TRANSACTION;
			SELECT TOP (1) @inbCharacterID = ReceiverCharacterID FROM #tmp1 ORDER BY ReceiverCharacterID DESC;
		END
	END
	ELSE IF @inyReceiverTarget = 3
	BEGIN
		IF @vchCharacterIDs > ''
		BEGIN
			INSERT #tmp3 (ReceiverCharacterID) SELECT string FROM dbo.TF_Split(@vchCharacterIDs, ',') WHERE string > '';
			SET @i = @@ROWCOUNT;
			IF @i <> (SELECT COUNT(*) FROM dbo.Characters CH WITH (NOLOCK) INNER JOIN #tmp3 T ON CH.CharacterID = T.ReceiverCharacterID)
			BEGIN
				SELECT T.ReceiverCharacterID AS CharacterID, NULL
				FROM #tmp3 T
				WHERE NOT EXISTS (
					SELECT * FROM dbo.Characters WITH (NOLOCK) WHERE CharacterID = T.ReceiverCharacterID
				);

				SET @intReturnValue = 101158;

				GOTO ErrorHandler;
			END
		END
		ELSE
		BEGIN
			SELECT @i = COUNT(*)
			FROM dbo.TF_NSplit(@nvcCharacterNames, N',')
			WHERE string > N''
			OPTION (KEEPFIXED PLAN);

			INSERT #tmp3 (ReceiverCharacterID)
			SELECT CH.CharacterID
			FROM dbo.TF_NSplit(@nvcCharacterNames, N',') F
				INNER JOIN dbo.Characters CH WITH (NOLOCK, FORCESEEK, INDEX(IX_UNF_Characters_CharacterName)) ON F.string = CH.CharacterName
			WHERE F.string > ''
			  AND CH.CharacterName IS NOT NULL
			OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

			IF @i <> @@ROWCOUNT
			BEGIN
				SELECT NULL AS CharacterID, string AS CharacterName
				FROM dbo.TF_NSplit(@nvcCharacterNames, N',') T
				WHERE NOT EXISTS (
					SELECT * FROM dbo.Characters WITH (NOLOCK, FORCESEEK, INDEX(IX_UNF_Characters_CharacterName)) WHERE CharacterName = T.string AND CharacterName IS NOT NULL
				)
				OPTION (KEEPFIXED PLAN);

				SET @intReturnValue = 101157

				GOTO ErrorHandler;
			END
		END

		IF EXISTS (SELECT * FROM #tmp3 GROUP BY ReceiverCharacterID HAVING COUNT(*) > 1)
		BEGIN
			WITH C1 AS (
				SELECT ReceiverCharacterID
				FROM #tmp3
				GROUP BY ReceiverCharacterID
				HAVING COUNT(*) > 1
			)
			SELECT CH.CharacterID, CH.CharacterName
			FROM C1
				INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C1.ReceiverCharacterID = CH.CharacterID;

			SET @intReturnValue = 101155;

			GOTO ErrorHandler;
		END

		ALTER TABLE #tmp3 ADD PRIMARY KEY CLUSTERED (ReceiverCharacterID);

		WHILE 1 = 1
		BEGIN
			TRUNCATE TABLE #tmp1;
			TRUNCATE TABLE #tmp2;

			BEGIN TRANSACTION;

			INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, OrderResult, OrderRollbackResult, GiftReceiveFlag, GiftMemo, GiftExpireDate, GiftSenderName, GiftRejectFlag, NewFlag)
			OUTPUT inserted.PurchaseOrderID, inserted.ReceiverCharacterID INTO #tmp1 (PurchaseOrderID, ReceiverCharacterID)
			SELECT TOP (1000) NULL, ReceiverCharacterID, 1, 0, 0, dbo.FN_IP2IPNumber(@vchIP), @dt2Now, 2, 5, NULL, NULL, NULL, 0, @nvcGiftMemo, DATEADD(MINUTE, @intGiftLifespan, @dt2Now), @nvcGiftSenderName, 0, @bitNewFlag
			FROM #tmp3
			WHERE ReceiverCharacterID > @inbCharacterID
			ORDER BY ReceiverCharacterID;

			IF @@ROWCOUNT = 0
			BEGIN
				ROLLBACK TRANSACTION;
				BREAK;
			END

			INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
			OUTPUT inserted.PurchaseOrderDetailID INTO #tmp2 (PurchaseOrderDetailID)
			SELECT PurchaseOrderID, @intProductID, 0
			FROM #tmp1;

			INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, GiveFailFlag)
			SELECT T1.PurchaseOrderDetailID, T2.ItemID, 0
			FROM #tmp2 T1
				INNER JOIN @tblItems T2 ON 1 = 1;

			COMMIT TRANSACTION;

			SELECT TOP (1) @inbCharacterID = ReceiverCharacterID FROM #tmp1 ORDER BY ReceiverCharacterID DESC;
		END
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
