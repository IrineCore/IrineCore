USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelMaterializedItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 27
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-11
description : ???? ??/?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103202 = ??? ???? ???? ????.
103151 = ???? ???? ????.
history :
3 = IP? ????? ?? at 2010-06-28
4 = ?? ??? ?? ???? ???? version 3? ?? ??? ?????. at 2010-06-29
6 = ??? ?? ??? ?? (? ? ??, ?? ??) at 2010-10-15
7 = ??? ?? ?? at 2011-03-18
8 = ??? ??? ?? ?? at 2011-03-29
9 = ItemLocation.MaterializedItemID ?? ??? ?? ?? at 2011-03-31
10 = ??? ?? ?? ?? ?? (ItemPotential)     by 20110621.wgkim
12 = [ItemLocation] ??? ??? ?? ?? by ??? at 2011-12-20
13 = ??? ?? ?? ?? ?? (@insUsedItemCount IS NULL = ?? ??)     by ??? at 2012-01-30
16 = 13?? ??     by ??? at 2012-04-09
17 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-19
18~21 = ???? ??? ?? ?? ?? ? ??? ?? ??? ?????? ?? ???? ?? ?? by ??? at 2012-07-09
22 = ??? ??? ?? ??? ?? ?? ????? ?? by ??? at 2012-08-13
23 = ??? ?? ? (??, ??)? ???? ??? ?? by ??? at 2012-08-14
24 = ???? MaterializedItemID ?? ?? ?? by ??? at 2012-12-27
**/

CREATE PROCEDURE [dbo].[P_DelMaterializedItem]
	  @inbItemSerial bigint --// ??? ??? ?? ??
	, @intAuditLogID int --// ??? ?? ID
	, @insUsedItemCount smallint = NULL --// ????? ??? ??
	, @bitGuildDepotItem bit = NULL --// ????? ???? ??????? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intItemID int
	, @inbOwnerCharacterID bigint
	, @dt2OwnershipStartDate datetime2(0)
	, @insItemCount smallint
	, @dt2Now datetime2(0) = GETDATE()
	, @bitSoulBoundFlag bit
	, @inySealCount tinyint
	, @inyItemOption tinyint
	, @inyItemPotential tinyint
	, @insRemainItemCount smallint
	, @insItemRemainCountBefore smallint
	, @inyItemPotentialMoveCount tinyint
	, @inyItemLevelBefore tinyint
	, @inbMaterializedItemID bigint
	, @inbCharacterID bigint
	, @inyDragonJewelType tinyint
	, @i int = 1
	, @j int;

DECLARE @tblTemp table (
	ItemID int NOT NULL,
	OwnerCharacterID bigint NULL,
	OwnershipStartDate datetime2(0) NULL,
	ItemCount smallint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	ItemPotentialMoveCount tinyint NULL,
	ItemLevel tinyint NULL,
	MaterializedItemID bigint NOT NULL

);

DECLARE @tblDeletedDragonJewelIDs table ( MaterializedItemID bigint NOT NULL );
DECLARE @tblDeletedDragonJewels table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	ItemID int NOT NULL,
	OwnerCharacterID bigint NULL,
	OwnershipStartDate datetime2(0) NULL,
	ItemCount smallint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	ItemPotentialMoveCount tinyint NULL,
	ItemLevel tinyint NULL,
	MaterializedItemID bigint NOT NULL,
	ItemSerial bigint NOT NULL,	
	ItemRemainCount int NOT NULL,
	DragonJewelType tinyint NULL
);


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ??? ?? ?? */
	IF @insUsedItemCount IS NULL 
	BEGIN

		IF (@bitGuildDepotItem IS NULL OR @bitGuildDepotItem = 0)
			/**_# ???? ?????.*/
			DELETE dbo.MaterializedItems
			OUTPUT deleted.ItemID, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotential, deleted.ItemPotentialMoveCount, deleted.ItemLevel, deleted.MaterializedItemID
			  INTO @tblTemp (ItemID, OwnerCharacterID, OwnershipStartDate, ItemCount, SoulBoundFlag, SealCount, ItemOption, ItemPotential, ItemPotentialMoveCount, ItemLevel, MaterializedItemID)
			 WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID IS NOT NULL AND ( ItemLocationCode <> 1 OR ItemLocationIndex NOT IN(0, 1) );
		ELSE
			DELETE dbo.MaterializedItems
			OUTPUT deleted.ItemID, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotential, deleted.ItemPotentialMoveCount, deleted.ItemLevel
			  INTO @tblTemp (ItemID, OwnerCharacterID, OwnershipStartDate, ItemCount, SoulBoundFlag, SealCount, ItemOption, ItemPotential, ItemPotentialMoveCount, ItemLevel)
			 WHERE ItemSerial = @inbItemSerial;
		/**_# ???? ???? ??? ?? ????? ???? ?? ??? 103202? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103202
			GOTO ErrorHandler;
		END		

		DELETE dbo.DragonJewels
		OUTPUT deleted.MaterializedItemID
		INTO @tblDeletedDragonJewelIDs (MaterializedItemID)
		FROM @tblTemp A
			INNER JOIN dbo.DragonJewels DJ WITH (NOLOCK) ON A.MaterializedItemID = DJ.ParentItemMaterializedItemID;

		IF @@ROWCOUNT > 0
		BEGIN
			DELETE dbo.MaterializedItems
			OUTPUT deleted.ItemID, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemRemainCount, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotential, deleted.ItemPotentialMoveCount, deleted.ItemLevel, deleted.MaterializedItemID, deleted.ItemSerial, deleted.ItemRemainCount, deleted.DragonJewelType
			  INTO @tblDeletedDragonJewels (ItemID, OwnerCharacterID, OwnershipStartDate, ItemCount, SoulBoundFlag, SealCount, ItemOption, ItemPotential, ItemPotentialMoveCount, ItemLevel, MaterializedItemID, ItemSerial, ItemRemainCount, DragonJewelType)
			FROM @tblDeletedDragonJewelIDs A 
				INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(PK_MaterializedItems), FORCESEEK) ON A.MaterializedItemID = MI.MaterializedItemID;
		
			SET @j = @@ROWCOUNT;

			IF @j > 0
			BEGIN
				WHILE @i <= @j
				BEGIN
					SELECT @intItemID = ItemID, @dt2OwnershipStartDate = OwnershipStartDate, @insItemCount = ItemCount
						, @bitSoulBoundFlag = SoulBoundFlag, @inySealCount = SealCount, @inyItemOption = ItemOption, @inyItemPotential = ItemPotential
						, @inbCharacterID = OwnerCharacterID, @inyDragonJewelType = DragonJewelType
					FROM @tblDeletedDragonJewels
					WHERE Seq = @i;
	
					EXEC dbo.P_AddItemLog
						  @dt2LogDate = @dt2Now
						, @inbItemSerial = @inbItemSerial
						, @intItemID = @intItemID
						, @inyItemLogCode = 38
						, @inbOwnerCharacterID = NULL
						, @inbOwnerCharacterIDBefore = @inbCharacterID
						, @insItemCount = 0
						, @insItemCountBefore = @insItemCount
						, @inbItemLogFKey1 = 0
						, @inbItemLogFKey2 = NULL
						, @dt2OwnershipStartDate = @dt2OwnershipStartDate
						, @intIP = NULL
						, @inySealCount = @inySealCount
						, @bitSoulBoundFlag = @bitSoulBoundFlag
						, @inyItemOption = @inyItemOption
						, @inyItemPotential = @inyItemPotential
						, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount						
						, @inyDragonJewelType = @inyDragonJewelType;

					SET @i += 1;	
				END
			END				
		END

		SELECT @intItemID = ItemID
		     , @inbOwnerCharacterID = OwnerCharacterID
		     , @dt2OwnershipStartDate = OwnershipStartDate
		     , @insItemRemainCountBefore = ItemCount
		     , @insRemainItemCount = 0
		     , @bitSoulBoundFlag = SoulBoundFlag
		     , @inySealCount = SealCount
		     , @inyItemOption = ItemOption
		     , @inyItemPotential = ItemPotential
			 , @inyItemPotentialMoveCount = ItemPotentialMoveCount
			 , @inyItemLevelBefore = ItemLevel
			 , @inbMaterializedItemID = MaterializedItemID
		  FROM @tblTemp;

	END
	/**_# ??? ?? ?? */
	ELSE
	BEGIN
		IF (@bitGuildDepotItem IS NULL OR @bitGuildDepotItem = 0)
			/**_# ??? ??? ?????.*/
			UPDATE dbo.MaterializedItems
			   SET @intItemID = ItemID
				 , @inbOwnerCharacterID = OwnerCharacterID
				 , @dt2OwnershipStartDate = OwnershipStartDate
				 , @insItemRemainCountBefore = ItemRemainCount
				 , @insRemainItemCount = ItemRemainCount -= @insUsedItemCount
				 , @bitSoulBoundFlag = SoulBoundFlag
				 , @inySealCount = SealCount
				 , @inyItemOption = ItemOption
				 , @inyItemPotential = ItemPotential
			     , @inyItemLevelBefore = ItemLevel
				 , @inbMaterializedItemID = MaterializedItemID
			 WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID IS NOT NULL;
		ELSE
			UPDATE dbo.MaterializedItems
			   SET @intItemID = ItemID
				 , @inbOwnerCharacterID = OwnerCharacterID
				 , @dt2OwnershipStartDate = OwnershipStartDate
				 , @insItemRemainCountBefore = ItemRemainCount
				 , @insRemainItemCount = ItemRemainCount -= @insUsedItemCount
				 , @bitSoulBoundFlag = SoulBoundFlag
				 , @inySealCount = SealCount
				 , @inyItemOption = ItemOption
				 , @inyItemPotential = ItemPotential
			     , @inyItemLevelBefore = ItemLevel
				 , @inbMaterializedItemID = MaterializedItemID
			 WHERE ItemSerial = @inbItemSerial;

		/**_# ???? ???? ??? ?? ????? ???? ?? ??? 103202? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103202
			GOTO ErrorHandler;
		END

		/**_# ?? ??? 0?? ???? ??(??, ???? ?)?? ?????.*/
		IF @insRemainItemCount = 0
		BEGIN
			DELETE dbo.MaterializedItems WHERE MaterializedItemID = @inbMaterializedItemID;
			DELETE dbo.GuildDepots WHERE MaterializedItemID = @inbMaterializedItemID;
		END

	END

	/**_# ???? ???? ?? ??? ?? ?? */
	UPDATE dbo.Trade SET TradeStatusCode = 3 WHERE ItemSerial = @inbItemSerial;
	
	/**_# ?? ??? ??? ???? ?? ???? ??? ?? ?? */
	IF @insUsedItemCount IS NULL 
		DELETE dbo.GuildDepots WHERE MaterializedItemID = @inbMaterializedItemID;

	/**_# ? or ??? ??? ??? ?? ?? */
	DELETE dbo.PetEquipments WHERE PetEquipmentItemSerial = @inbItemSerial OR PetItemSerial = @inbItemSerial;

	/**_# ?????? ?? ?? ????? ?????.*/
	DELETE dbo.Fairies WHERE FairyItemSerial = @inbItemSerial;		

	/**_# ??? ??? ?????.*/
	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 38
		, @inbOwnerCharacterID = NULL
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = @insRemainItemCount
		, @insItemCountBefore = @insItemRemainCountBefore
		, @inbItemLogFKey1 = @intAuditLogID
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = NULL
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
	    , @inyItemLevelBefore = @inyItemLevelBefore 
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount;

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
