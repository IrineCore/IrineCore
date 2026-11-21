USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddAuctionItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-18
description : ?????? ??? ?????.(???, ?????? ??-??, ??, ?? ???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103156 = ItemSerial ?????.
1030000061 = ?? ?? ??? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DLS_AddAuctionItem]
	  @intItemID int --// ??? ID
	, @insItemCount smallint --// ?? ??			
	, @insItemDurability smallint --// ??? ??? / ?? ??? ???? ?? ???
	, @intRandomSeed int --// ?? ??
	, @intCoolTime int --// ? ?? (?)
	, @inyItemLevel tinyint --// ??? ??
	, @inyItemPotential tinyint --// ??? ???
	, @bitSoulBoundFlag bit --// 0=??, 1=?????
	, @inySealCount tinyint --// ?? ???
	, @inyItemOption tinyint --// ??? ??
	, @intItemLifespan int --// ??? ?? (?)	
	, @bitEternityFlag bit --// 0=???? ?? ??, 1=?? ???		
	, @bitCashItemFlag bit --// 0=?? ???, 1=?? ???
	, @inyDragonJewelType tinyint = NULL --// ??????	
	, @inyItemCategoryID tinyint --// ??? ???? ??
	, @nvcItemName nvarchar(50) --// ??? ??
	, @inyLevelLimit tinyint --// ??? ?? ??
	, @inyJobCode tinyint --// ??? ???? (???? ?? ?? ???)
	, @inyItemRankCode tinyint --// ??? ??
	, @inbStartPrice bigint --// ?? ???
	, @inbBuynowPrice bigint --// ?? ?????
	, @sdtBiddingEndDate smalldatetime --// ?? ????
	, @inbOwnerCharacterID bigint --// ???? ??? ??? ID	
	, @dt2AuctionCountStartDate datetime2(0) = NULL --// ?? ?? ?? ?? ?? ????
	, @dt2AuctionCountEndDate datetime2(0) = NULL --// ?? ?? ?? ?? ?? ????
	, @intAuctionCount int = NULL --//  ?? ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @inbMaterializedItemID bigint
	  , @inbItemSerial bigint
	  , @intBeforeAuctionCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	IF @dt2AuctionCountStartDate IS NOT NULL AND @dt2AuctionCountEndDate IS NOT NULL AND @intAuctionCount IS NOT NULL
	BEGIN
		SELECT @intBeforeAuctionCount = COUNT(*)
		FROM dbo.Auctions WITH (NOLOCK)
		WHERE RegistrationDate >= @dt2AuctionCountStartDate AND RegistrationDate <= @dt2AuctionCountEndDate AND ItemID = @intItemID;

		IF @intBeforeAuctionCount >= @intAuctionCount
		BEGIN
			SET @intReturnValue = 1030000061;
			GOTO ErrorHandler;
		END
	END

	SET @inbItemSerial = ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint));

	/**_# [MaterializedItems] ???? INSERT???.*/
	INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, PayMethodCode, ItemLocationCode, ItemLocationIndex, ItemPotentialMoveCount, ItemCount, ItemPrice, DragonJewelType)
	VALUES (@inbItemSerial, @intItemID, @insItemCount, NULL, 10, NULL, @insItemDurability, @intRandomSeed, 0, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2Now, @intItemLifespan, @bitEternityFlag, DATEADD(MINUTE, CASE @bitEternityFlag WHEN 0 THEN @intItemLifespan ELSE 525600 END, @dt2Now), NULL, @dt2Now, 5, 22, 0, 0, @insItemCount, 0, @inyDragonJewelType);
	
	/**_# [Auctions] ???? INSERT???.*/
	INSERT dbo.Auctions (CharacterID, CategoryCode, StartPrice, Commission, BuynowPrice, AuctionStatusCode, BiddingEndDate, RegistrationDate, UpdateDate, ItemID, ItemSerial, ItemLevel, ItemCount, ItemOption, ItemPotential, SealCount, ItemDurability, ItemPotentialMoveCount, RandomSeed)
	VALUES (@inbOwnerCharacterID, @inyItemCategoryID, @inbStartPrice, 0, @inbBuynowPrice, 0, @sdtBiddingEndDate, @dt2Now, @dt2Now, @intItemID, @inbItemSerial, @inyItemLevel, @insItemCount, @inyItemOption, @inyItemPotential, @inySealCount, @insItemDurability, 0, @intRandomSeed);

	/**_# [Items] ???? MERGE???.*/
	MERGE dbo.AuctionItems AS T
	USING (SELECT @intItemID AS ItemID, @nvcItemName AS ItemName, @inyLevelLimit AS LevelLimit, @inyJobCode AS JobCode, @inyItemRankCode AS ItemRankCode, @bitCashItemFlag AS CashItemFlag, @inyItemCategoryID AS ItemCategoryID) AS S
	ON T.ItemID = S.ItemID
	WHEN MATCHED THEN
		UPDATE SET ItemName = S.ItemName, LevelLimit = S.LevelLimit, JobCode = S.JobCode, ItemRankCode = S.ItemRankCode, CashItemFlag = S.CashItemFlag, ItemCategoryID = S.ItemCategoryID
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (ItemID, ItemName, LevelLimit, JobCode, ItemRankCode, CashItemFlag, ItemCategoryID)
		VALUES (S.ItemID, S.ItemName, S.LevelLimit, S.JobCode, S.ItemRankCode, S.CashItemFlag, S.ItemCategoryID)
	OPTION (KEEPFIXED PLAN, LOOP JOIN);

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNI_MaterializedItems_ItemSerial%'
		SET @intReturnValue = 103156;	

	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
