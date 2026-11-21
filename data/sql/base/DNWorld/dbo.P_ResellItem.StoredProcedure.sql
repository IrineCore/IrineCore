USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ResellItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 33
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-22
description : ???? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
103191 = ??? ???? ??? ?? ????.
history :
13 = TRANSACTION? ??? ItemLocation? ??? MaterializedItems? ?? ???? ?? at 2010-06-16
14 = ItemLocation???? MaterializedItems???? ?? ?? ?? at 2010-06-16
15 = IP? ????? ?? at 2010-06-28
16 = ???? ??? ????? ????? ?? at 2010-12-30
17 = ??? ?? ?? at 2011-03-18
18 = ??? ??? ?? ?? at 2011-03-29
20 = ItemLocation.MaterializedItemID ?? ??? ?? ?? at 2011-03-31
22 = ?? ??? ?? ???? validation ?? & input param @bitAllFlag ?? at 2011-06-21
23 = ??? ?? ?? ?? ?? (SealCount, SoulBoundFlag, ItemOption, ItemPotential)     by 20110621.wgkim
25 = ?? ?? at 2011-06-22
26~28 = ??? ??? ?? ??     by 20111019.wgkim
29 = 26~28 ???? ??     by 20111019.wgkim
30 = [ItemLocation] ??? ??? ?? ?? by ??? at 2011-12-20
31 = ??? ??? ?? ??? ?? ?? ?? by ???  at 2013-03-25
33 = ??? ??? ?? ??? ?? ?? ?? by ???  at 2013-06-03
**/

CREATE PROCEDURE [dbo].[P_ResellItem]
	  @inbCharacterID bigint --// ??? ID
	, @inbItemSerial bigint --// ??? ?? ??
	, @insItemCount smallint --// ???? ??? ??
	, @inbResellPrice bigint --// ??? ??
	, @intNPCID int --// ??? NPC ID
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @bitAllFlag bit = NULL --// 0=?? ?? ??, 1=?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2LogDate datetime2(0) = GETDATE()
	, @insRemainItemCount smallint
	, @inbCurrentCoin bigint
	, @intItemID int
	, @dt2OwnershipStartDate datetime2(0)
	, @inyPayMethodCode tinyint
	, @inbOwnerCharacterID bigint
	, @insRemainItemCountBefore smallint
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @inbMaterializedItemID bigint
	, @inySealCount tinyint
	, @bitSoulBoundFlag bit
	, @inyItemOption tinyint
	, @inyItemPotential tinyint
	, @inyDragonJewelType tinyint
	, @i int = 1
	, @j int
	, @dt2Now datetime2(0) = GETDATE()
	, @inbDeletedDragonJewelItemSerial bigint
	, @intDeletedDragonJewelItemID int
	, @dt2DeletedDragonJewelOwnershipStartDate datetime2(0)
	, @insDeletedDragonJewelItemCount smallint
	, @bitDeletedDragonJewelSoulBoundFlag bit
	, @inyDeletedDragonJewelSealCount tinyint
	, @inyDeletedDragonJewelItemOption tinyint
	, @inyDeletedDragonJewelItemPotential tinyint
	, @inyDeletedDragonJewelItemPotentialMoveCount tinyint;

DECLARE @tblTemp table (
	MaterializedItemID bigint NOT NULL,
	ItemRemainCount smallint NOT NULL,
	ItemID int NOT NULL,
	OwnershipStartDate datetime2(0) NULL,
	PayMethodCode tinyint NULL,
	ItemPotential	tinyint	NULL,
	SoulBoundFlag	bit	NULL,
	SealCount	tinyint	NULL,
	ItemOption	tinyint	NULL
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

DECLARE @tblDeleteDragonJewelIDs table (MaterializedItemID bigint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @bitAllFlag = 0 OR @bitAllFlag IS NULL
	BEGIN
		/**_# ???? ??? ?????.*/
		UPDATE dbo.MaterializedItems
		SET	@inbMaterializedItemID = MaterializedItemID
			, @insRemainItemCountBefore = ItemRemainCount
			, @intItemID = ItemID
			, @dt2OwnershipStartDate = OwnershipStartDate
			, @inyPayMethodCode = PayMethodCode
			, @insRemainItemCount = ItemRemainCount -= @insItemCount
			, @inbOwnerCharacterID = OwnerCharacterID = CASE WHEN ItemRemainCount > @insItemCount THEN OwnerCharacterID ELSE NULL END
			, @inySealCount = SealCount
			, @bitSoulBoundFlag = SoulBoundFlag
			, @inyItemOption = ItemOption
			, @inyItemPotential = ItemPotential
		WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID;

		/**_# ???? ???? ??? 103151? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103151;
			GOTO ErrorHandler;
		END

		/**_# ?? ??? 0?? ???? ?? (??, ???? ?)?? ?????.*/
		IF @insRemainItemCount <= 0
			DELETE dbo.MaterializedItems 
			WHERE MaterializedItemID = @inbMaterializedItemID
			OPTION (KEEPFIXED PLAN);
	END

	ELSE
	BEGIN
		DELETE dbo.MaterializedItems 
		OUTPUT deleted.MaterializedItemID, deleted.ItemRemainCount, deleted.ItemID, deleted.OwnershipStartDate, deleted.PayMethodCode, deleted.ItemPotential, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption
		INTO @tblTemp (MaterializedItemID, ItemRemainCount, ItemID, OwnershipStartDate, PayMethodCode, ItemPotential, SoulBoundFlag, SealCount, ItemOption)
		WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemRemainCount = @insItemCount
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103151;
			GOTO ErrorHandler;
		END		

		SELECT @inbMaterializedItemID = MaterializedItemID
			, @insRemainItemCountBefore = ItemRemainCount
			, @intItemID = ItemID
			, @dt2OwnershipStartDate = OwnershipStartDate
			, @inyPayMethodCode = PayMethodCode
			, @insRemainItemCount = 0
			, @inbOwnerCharacterID = NULL
			, @inySealCount = SealCount
			, @bitSoulBoundFlag = SoulBoundFlag
			, @inyItemOption = ItemOption
			, @inyItemPotential = ItemPotential
		FROM @tblTemp;

		DELETE dbo.Fairies
		WHERE FairyItemSerial = @inbItemSerial;

		DELETE dbo.DragonJewels
		OUTPUT deleted.MaterializedItemID
		INTO @tblDeleteDragonJewelIDs (MaterializedItemID)
		WHERE ParentItemSerial = @inbItemSerial;

		IF @@ROWCOUNT > 0
		BEGIN
			DELETE dbo.MaterializedItems
			OUTPUT deleted.MaterializedItemID, deleted.ItemID, deleted.ItemSerial, deleted.OwnerCharacterID, deleted.OwnershipStartDate, deleted.ItemCount, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemPotential, deleted.ItemPotentialMoveCount, deleted.DragonJewelType
			INTO @tblDeletedDragonJewels (MaterializedItemID, ItemID, ItemSerial, OwnerCharacterID, OwnershipStartDate, ItemCount, SoulBoundFlag, SealCount, ItemOption, ItemPotential, ItemPotentialMoveCount, DragonJewelType)
			FROM @tblDeleteDragonJewelIDs A 
				INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(PK_MaterializedItems), FORCESEEK) ON A.MaterializedItemID = MI.MaterializedItemID;

			SET @j = @@ROWCOUNT;

			IF @j > 0
			BEGIN
				WHILE @i <= @j
				BEGIN
					SELECT @intDeletedDragonJewelItemID = ItemID, @dt2DeletedDragonJewelOwnershipStartDate = OwnershipStartDate, @insDeletedDragonJewelItemCount = ItemCount
						, @bitDeletedDragonJewelSoulBoundFlag = SoulBoundFlag, @inyDeletedDragonJewelSealCount = SealCount, @inyDeletedDragonJewelItemOption = ItemOption
						, @inyDeletedDragonJewelItemPotential = ItemPotential, @inbDeletedDragonJewelItemSerial = ItemSerial, @inyDeletedDragonJewelItemPotentialMoveCount = ItemPotentialMoveCount
					FROM @tblDeletedDragonJewels
					WHERE Seq = @i;
	
					EXEC dbo.P_AddItemLog
						  @dt2LogDate = @dt2Now
						, @inbItemSerial = @inbDeletedDragonJewelItemSerial
						, @intItemID = @intDeletedDragonJewelItemID
						, @inyItemLogCode = 22
						, @inbOwnerCharacterID = NULL
						, @inbOwnerCharacterIDBefore = @inbCharacterID
						, @insItemCount = 0
						, @insItemCountBefore = @insDeletedDragonJewelItemCount
						, @inbItemLogFKey1 = 0
						, @inbItemLogFKey2 = NULL
						, @dt2OwnershipStartDate = @dt2DeletedDragonJewelOwnershipStartDate
						, @intIP = NULL
						, @inySealCount = @inyDeletedDragonJewelSealCount
						, @bitSoulBoundFlag = @bitDeletedDragonJewelSoulBoundFlag
						, @inyItemOption = @inyDeletedDragonJewelItemOption
						, @inyItemPotential = @inyDeletedDragonJewelItemPotential
						, @inyItemPotentialMoveCount = @inyDeletedDragonJewelItemPotentialMoveCount						
						, @inyDragonJewelType = NULL;

					SET @i += 1;	
				END						
			END		
		END
		
	END

	/**_# [CharacterStatus].[Coin]? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @inbCurrentCoin = Coin += @inbResellPrice
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	/**_# ??? ??? ?????.*/
	SET @dt2OwnershipStartDate = CASE @insRemainItemCount WHEN 0 THEN @dt2OwnershipStartDate ELSE NULL END;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2LogDate
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 22
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbCharacterID
		, @insItemCount = @insRemainItemCount
		, @insItemCountBefore = @insRemainItemCountBefore
		, @inbItemLogFKey1 = @intMapID
		, @inbItemLogFKey2 = @intNPCID
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential;

	/**_# [P_AddCoinLog]? ???? ?? ??? ?????.*/
	EXEC dbo.P_AddCoinLog
		  @dt2LogDate = @dt2LogDate
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = 10
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @inbResellPrice
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID
		, @inbCoinChangeFKey = @intNPCID;

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
