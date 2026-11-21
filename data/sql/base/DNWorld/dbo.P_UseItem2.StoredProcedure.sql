USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UseItem2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 21
version : 20
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ???? ?? ?? ??????. (??? ???, ??? ??? SP? ?????.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
103152 = @inyItemUseTypeCode ? ??
103292 = ???? ???? ?? ??????.
history :
3 = ?? ???? ? ?? ??? ???? ????? ??? ???, ?? 3???? ?? ????? ??? ??? ?? 1?? ?? ?? ????? ?? at 2011-10-19
4 = ver.3? ?? & ItemCopyCheck ???? ?? ??? ????? ?? at 2011-10-19
5 = 3, 4?? ??? ?? ????? ?? at 2011-10-19
6~9 = ???? ?? ??? ?? ItemID ? ItemLevel ? ??? ItemLogFKey2 ? ???? ?? ??     by ??? at 2011-11-30
10 = ?? ??? ?? ??? Code? ?? by ??? at 2011-12-01
11~13 = @inyItemUseTypeCode=7 ??     by ??? at 2012-01-25
14 = @bitAllFlag ?? 1??? ?? ??? ???? ??? ???? ?? ?? 103151? ????? ?? by ??? at 2012-08-20
16 = ??? ??? ??? ?????? ???? by ??? at 2013-03-22
21 = ??? ??? ??? ?? ?? ???? ?? ?? by ??? at 2013-06-03
**/

CREATE PROCEDURE [dbo].[P_UseItem2]
	  @inbItemSerial bigint --// ??? ???
	, @inyItemUseTypeCode tinyint --// 1=??? ??? ??, 2=???, 4=??, 5=NPC?? ??, 6=?? ??? ??(?? ?? ? ??), 7=????(DOORS), 8=???????????, 9=???????
	, @insUsedItemCount smallint --// ??? ??
	, @intChannelID int --// ?? ID. (?, @inyItemUseTypeCode=28 ? ???? NPC ID)
	, @intMapID int --// ? ID (?, @inyItemUseTypeCode=7 ? ???? AuctionID)
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @bitAllFlag bit = NULL --// 0=?? ?? ??, 1=?? ?? ??
	, @inbTargetItemID bigint = NULL --// ?? ???ID (???? ??? ???? ?? ??)
	, @inbTargetItemLevel bigint = NULL --// ?? ??? ?? ?? (???? ??? ???? ?? ??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2LogDate datetime2(0) = GETDATE()
	, @insRemainItemCount smallint
	, @insItemRemainCountBefore smallint
	, @inbOwnerCharacterID bigint
	, @intItemID int
	, @dt2OwnershipStartDate datetime2(0)
	, @inyPayMethodCode tinyint
	, @inbOwnerCharacterIDAfter bigint
	, @inyItemLogCode tinyint
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @inbItemLogFKey2 bigint = CASE @inyItemUseTypeCode WHEN 42 THEN @intChannelID END
	, @inbMaterializedItemID bigint
	, @inySealCount tinyint
	, @bitSoulBoundFlag bit
	, @inyItemOption tinyint
	, @inyItemPotential tinyint
	, @inyItemLevel tinyint
	, @i int = 1
	, @j int
	, @dt2Now datetime2(0) = GETDATE()
	, @intDragonJewelItemID int
	, @dt2DragonJewelOwnershipStartDate datetime2(0)
	, @insDragonJewelItemCount smallint
	, @bitDragonJewelSoulBoundFlag bit
	, @inyDragonJewelSealCount tinyint
	, @inyDragonJewelItemOption tinyint
	, @inyDragonJewelItemPotential tinyint
	, @inbDragonJewelItemSerial bigint
	, @inyDragonJewelItemPotentialMoveCount tinyint
	, @inyDragonJewelDragonJewelType tinyint;

DECLARE @tblDeletedDragonJewelIDs table (
	MaterializedItemID bigint NOT NULL
);

DECLARE @tblDeletedDragonJewels table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	MaterializedItemID bigint NOT NULL,
	ItemID int NOT NULL,
	ItemSerial bigint NOT NULL,
	OwnerCharacterID bigint NULL,
	OwnershipStartDate datetime2(0) NULL,
	ItemCount smallint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	ItemPotentialMoveCount tinyint NULL,
	DragonJewelType tinyint NULL
);

-- ???? ??? ???? ?? (@@intTargetItemID ? ????? ??)
IF @inbTargetItemID IS NOT NULL
BEGIN
	SET @inbItemLogFKey2 = (@inbTargetItemID * 100) + ISNULL(@inbTargetItemLevel, 0);
END

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyItemUseTypeCode NOT IN (1, 2, 4, 5, 6, 7, 8, 9)
	BEGIN
		SET @intReturnValue = 103152;
		GOTO ErrorHandler;
	END
	/**_# ???? ???? ?? ????? ?? ?????.*/
	UPDATE dbo.Trade
	SET TradeStatusCode = 3
		, TradeDate = @dt2LogDate
	WHERE ItemSerial = @inbItemSerial AND TradeStatusCode = 0
	OPTION (KEEPFIXED PLAN);

	SET @inyItemLogCode =
		CASE @inyItemUseTypeCode
			WHEN 1 THEN 20
			WHEN 2 THEN 21
			WHEN 4 THEN 23
			WHEN 5 THEN 42
			WHEN 6 THEN 56
			WHEN 7 THEN 58
			WHEN 8 THEN 77
			WHEN 9 THEN 79
		END;

	/**_# ???? ??? ?????.*/
	IF @bitAllFlag = 0
	BEGIN
		UPDATE dbo.MaterializedItems
		SET @inbMaterializedItemID = MaterializedItemID
			, @insItemRemainCountBefore = ItemRemainCount
			, @inbOwnerCharacterID = OwnerCharacterID
			, @intItemID = ItemID
			, @inyPayMethodCode = PayMethodCode
			, @dt2OwnershipStartDate = OwnershipStartDate
			, @insRemainItemCount = ItemRemainCount -= @insUsedItemCount
			, @inbOwnerCharacterIDAfter = OwnerCharacterID = CASE WHEN ItemRemainCount > @insUsedItemCount THEN OwnerCharacterID ELSE NULL END
			, @inySealCount = SealCount
			, @bitSoulBoundFlag = SoulBoundFlag
			, @inyItemOption = ItemOption
			, @inyItemPotential = ItemPotential
			, @inyItemLevel = ItemLevel
		WHERE ItemSerial = @inbItemSerial;

		/**_# ???? ???? ??? 103151? ?????.*/
		IF @@ROWCOUNT = 0 OR @insRemainItemCount < 0
		BEGIN
			SET @intReturnValue = 103151;
			GOTO ErrorHandler;
		END

		/**_# ?? ??? 0?? ???? ?? (??, ???? ?)?? ?????.*/
		IF @insRemainItemCount = 0
			DELETE dbo.MaterializedItems
			WHERE MaterializedItemID = @inbMaterializedItemID
			OPTION (KEEPFIXED PLAN);
	END

	ELSE
	BEGIN
		SELECT @inbMaterializedItemID = MaterializedItemID
			, @insItemRemainCountBefore = ItemRemainCount
			, @inbOwnerCharacterID = OwnerCharacterID
			, @intItemID = ItemID
			, @inyPayMethodCode = PayMethodCode
			, @dt2OwnershipStartDate = OwnershipStartDate
			, @insRemainItemCount = ItemRemainCount - @insUsedItemCount
			, @inbOwnerCharacterIDAfter = NULL
			, @inySealCount = SealCount
			, @bitSoulBoundFlag = SoulBoundFlag
			, @inyItemOption = ItemOption
			, @inyItemPotential = ItemPotential
			, @inyItemLevel = ItemLevel
		FROM dbo.MaterializedItems WITH (NOLOCK)
		WHERE ItemSerial = @inbItemSerial;

		IF @@ROWCOUNT = 0 OR @insRemainItemCount < 0
		BEGIN
			SET @intReturnValue = 103151;
			GOTO ErrorHandler;
		END

		DELETE dbo.MaterializedItems	
		WHERE MaterializedItemID = @inbMaterializedItemID
		OPTION (KEEPFIXED PLAN);

		DELETE dbo.Fairies
		WHERE FairyItemSerial = @inbMaterializedItemID
		OPTION (KEEPFIXED PLAN);
		
		DELETE dbo.DragonJewels 
		OUTPUT deleted.MaterializedItemID
		INTO @tblDeletedDragonJewelIDs (MaterializedItemID)
		WHERE ParentItemSerial = @inbItemSerial;

		IF @@ROWCOUNT > 0
		BEGIN
			DELETE dbo.MaterializedItems
			OUTPUT deleted.MaterializedItemID, deleted.ItemID, deleted.ItemSerial, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotential, deleted.DragonJewelType, deleted.ItemPotentialMoveCount
			INTO @tblDeletedDragonJewels (MaterializedItemID, ItemID, ItemSerial, OwnerCharacterID, OwnershipStartDate, ItemCount, SoulBoundFlag, SealCount, ItemOption, ItemPotential, DragonJewelType, ItemPotentialMoveCount)
			FROM @tblDeletedDragonJewelIDs A INNER JOIN dbo.MaterializedItems B WITH (NOLOCK) ON A.MaterializedItemID = B.MaterializedItemID
			
			SET @j = @@ROWCOUNT;

			IF @j > 0
			BEGIN
				WHILE @i <= @j
				BEGIN
					SELECT @intDragonJewelItemID = ItemID, @dt2DragonJewelOwnershipStartDate = OwnershipStartDate, @insDragonJewelItemCount = ItemCount
						, @bitDragonJewelSoulBoundFlag = SoulBoundFlag, @inyDragonJewelSealCount = SealCount, @inyDragonJewelItemOption = ItemOption
						, @inyDragonJewelItemPotential = ItemPotential, @inbDragonJewelItemSerial = ItemSerial, @inyDragonJewelItemPotentialMoveCount = ItemPotentialMoveCount
						, @inyDragonJewelDragonJewelType  = DragonJewelType
					FROM @tblDeletedDragonJewels
					WHERE Seq = @i;
	
					EXEC dbo.P_AddItemLog
						  @dt2LogDate = @dt2Now
						, @inbItemSerial = @inbDragonJewelItemSerial
						, @intItemID = @intDragonJewelItemID
						, @inyItemLogCode = @inyItemLogCode
						, @inbOwnerCharacterID = NULL
						, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
						, @insItemCount = 0
						, @insItemCountBefore = @insDragonJewelItemCount
						, @inbItemLogFKey1 = 0
						, @inbItemLogFKey2 = NULL
						, @dt2OwnershipStartDate = @dt2DragonJewelOwnershipStartDate
						, @intIP = NULL
						, @inySealCount = @inyDragonJewelSealCount
						, @bitSoulBoundFlag = @bitDragonJewelSoulBoundFlag
						, @inyItemOption = @inyDragonJewelItemOption
						, @inyItemPotential = @inyDragonJewelItemPotential
						, @inyItemPotentialMoveCount = @inyDragonJewelItemPotentialMoveCount						
						, @inyDragonJewelType = @inyDragonJewelDragonJewelType;

					SET @i += 1;	
				END						
			END					
		END

	END

	/**_# ??? ??? ?????.*/
	SET @dt2OwnershipStartDate = CASE @insRemainItemCount WHEN 0 THEN @dt2OwnershipStartDate ELSE NULL END;

	EXEC @intReturnValue = dbo.P_AddItemLog
		  @dt2LogDate = @dt2LogDate
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = @inyItemLogCode
		, @inbOwnerCharacterID = @inbOwnerCharacterIDAfter
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = @insRemainItemCount
		, @insItemCountBefore = @insItemRemainCountBefore
		, @inbItemLogFKey1 = @intMapID
		, @inbItemLogFKey2 = @inbItemLogFKey2
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemLevel = @inyItemLevel;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_MaterializedItems_ItemRemainCount%'
	BEGIN
		INSERT dbo.TableAuditLogs (tableName, operationCode, columnNames, beforeImages, modifier, occurredDate)
		VALUES (N'P_UseItem2', 1, '', CAST(@inbItemSerial AS varchar(50)) + ' / ' + CAST(@insUsedItemCount AS varchar(50)), 'SYS', GETDATE());

		SELECT TOP (1) @inbItemSerial = ItemSerial
		FROM dbo.MaterializedItems WITH (NOLOCK)
		WHERE OwnerCharacterID = @inbOwnerCharacterID AND ItemMaterializeDate > DATEADD(SECOND, -3, GETDATE());

		IF @@ROWCOUNT > 0
		BEGIN
			EXEC dbo.P_DelMaterializedItem
				  @inbItemSerial = @inbItemSerial
				, @intAuditLogID = 0;
		END

		ELSE
		BEGIN
			UPDATE dbo.ItemCopyCheck
			SET ItemMaterializeDate = GETDATE()
			FROM dbo.ItemCopyCheck WITH (INDEX(PK_ItemCopyCheck), FORCESEEK)
			WHERE CharacterID = @inbOwnerCharacterID
			OPTION (KEEPFIXED PLAN);

			IF @@ROWCOUNT = 0
				INSERT dbo.ItemCopyCheck (CharacterID, ItemMaterializeDate)
				VALUES (@inbOwnerCharacterID, GETDATE());
		END

		SET @intReturnValue = 103151;
	END

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
