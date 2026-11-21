USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddAuctionItemsHistory]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-03-07

description :
	?? ??? ?? ? ??
		0=?? (???? ??? ?? : ????? + ??????)
		1=?? (???? ??? ?? : ?????)
		2=?? (???? ??? ?? : ??????)
		3=??:????? (???? ??? ?? : ??????+?????)
		4=??:????? (???? ??? ?? : ?????)
		5=??:????? (???? ??? ?? : ??????)
		6=??:????? (???? ??, ???? ??? ?? : ??????)
		7=0? ?? ??(??)
		8=1? ?? ??(??)
		9=2? ?? ??(??)
		10=3? ?? ??(??) - ???
		11=4? ?? ??(??) - ???
		12=5? ?? ??(??)
		13=6? ?? ??(??)
result set :
	@intItemID / int
	@insItemRemainCount / smallint
	@inyPayMethodCode / tinyint
	@inbSenderCharacterID / bigint
	@inyItemMaterializeCode / tinyint
	@inbItemMaterializeFKey / bigint
	@insItemDurability / smallint
	@intRandomSeed / int
	@intCoolTime / int
	@inyItemLevel / tinyint
	@inyItemPotential / tinyint
	@bitSoulBoundFlag / bit
	@inySealCount / tinyint
	@inyItemOption / tinyint
	@dt2ItemMaterializeDate / datetime2(0)
	@intItemLifespan / int
	@bitEternityFlag / bit
	@sdtItemExpireDate / smalldatetime
	@dt2OwnershipStartDate / datetime2(0)
	@bitExpireCompleteFlag / bit
	@bitTradeItemFlag / bit
	@intAdditiveItemID / int
	@inyItemLocationCode / tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103102 = ???? ???? ????.
	103123 = ??? ?????.
	103151 = ???? ???? ????.
	103364 = ?? ??? ??? ???????.
	103365 = ?? ??? ??? ???????.
	103202 = ??? ???? ???? ????.
	103366 = ?? ??? ?? ? ?? ??? ???? ????.
history :
	5 = ?????? ?? ??     by ??? at 2012-04-12
	6 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-19
	7~8 = dbo.MaterializedItems ???? ItemCount, ItemPrice ?? ??     by ??? at 2012-10-25
**/

CREATE PROCEDURE [dbo].[P_DRS_AddAuctionItemsHistory]
	  @inbItemSerial bigint --// ???????
	, @inbOwnerCharacterID bigint --// ???? ??? ??? ID
	, @intCoin int --// ?? (?? (+)? ??)
	, @inyAuctionItemsHistoryStatusCode tinyint --// 0=?? (???? ??? ?? : ????? + ??????), 1=?? (???? ??? ?? : ?????), 2=?? (???? ??? ?? : ??????), 3=??:????? (???? ??? ?? : ??????+?????), 4=??:????? (???? ??? ?? : ?????), 5=??:????? (???? ??? ?? : ??????), 6=??:????? (???? ??, ???? ??? ?? : ??????), 7=0? ?? ??(??), 8=1? ?? ??(??), 9=2? ?? ??(??), 10=3? ?? ??(??), 11=4? ?? ??(??), 12=5? ?? ??(??), 13=6? ?? ??(??)
	, @intAuctionItemsHistoryStatusFKey int --// @inyAuctionItemsHistoryStatusCode=(0,1) : AuctionID, @inyAuctionItemsHistoryStatusCode=(2) : BiddingID, @inyAuctionItemsHistoryStatusCode=(3,4,5,6) : ContractID

	, @intItemID int = NULL OUTPUT --// ???ID
	, @insItemRemainCount smallint = NULL OUTPUT --// ?????
	, @inyPayMethodCode tinyint = NULL OUTPUT --// 1=????, 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??, 9=VIP ??, 10=PVP ?? ???, 11=?? ?? ???, 12=?? ?? ???, 13=?? ?? ???, 14=??2, 15=Time Event ??, 16=??? ?? ???, 17=??? ??
	, @inbSenderCharacterID bigint = NULL OUTPUT --// ??? ?? ??? ??? ID
	, @inyItemMaterializeCode tinyint = NULL OUTPUT --// 1=??, 2=?? ??, 3=?????, 4=??, 5=?????? ??, 6=?? ??, 7=?? ??, 8=DNA??? ??, 9=???? ??, 10=???  ?? ??, 11=??? ???? ??, 13=??, 14=??, 15=?? ??, 16=PVP ??, 17=?? ??, 18=??? ?? ??, 19=Fixed Item, 20=??????, 21=??? ??, 22= ??, 23=??, 24=???, 25=PVP?????? ??, 26=?? ???? ??, 27=??? ??,  28=??? ?? ???? ??, 29=???, 30=?? ?? ??
	, @inbItemMaterializeFKey bigint = NULL OUTPUT --// ItemMaterializeCode ? ?? ? ?. 1:PartyID or NULL, 2:MissionID, 3:QuestID, 4:PurchaseID, 5:NPCID, 6/7/8:PurchaseID, 10:MailID, 11:NPCID, 13:NULL, 14:NULL, 15:NULL, 16:NULL, 17:NULL, 18:MailID, 19:ItemSerial, 22:CharacterID, 29:RepurchaseNo
	, @insItemDurability smallint = NULL OUTPUT --// ??? ??? / ?? ??? ???? ?? ???
	, @intRandomSeed int = NULL OUTPUT --// ?? ??
	, @intCoolTime int = NULL OUTPUT --// ? ?? (?)
	, @inyItemLevel tinyint = NULL OUTPUT --// ??? ??
	, @inyItemPotential tinyint = NULL OUTPUT --// ??? ???
	, @bitSoulBoundFlag bit = NULL OUTPUT --// 0=??, 1=?????
	, @inySealCount tinyint = NULL OUTPUT --// ?? ???
	, @inyItemOption tinyint = NULL OUTPUT --// ??? ??
	, @dt2ItemMaterializeDate datetime2(0) = NULL OUTPUT --// ??? ?? ??
	, @intItemLifespan int = NULL OUTPUT --// ??? ?? (?)
	, @bitEternityFlag bit = NULL OUTPUT --// 0=???? ?? ??, 1=?? ???
	, @sdtItemExpireDate smalldatetime = NULL OUTPUT --// ??? ?? ??
	, @dt2OwnershipStartDate datetime2(0) = NULL OUTPUT --// ??? ?? ??
	, @bitExpireCompleteFlag bit = NULL OUTPUT --// 0=?? ???, 1=?? ??
	, @bitTradeItemFlag bit = NULL OUTPUT --// 0=???? ?? ???? ??, 1=???? ?? ??? ?? ???
	, @intAdditiveItemID int = NULL OUTPUT --// ??? ??? ID
	, @inyItemLocationCode tinyint = NULL OUTPUT --// 1=????, 2=????, 3=??, 4=??? ????, 5=?? ????, 7=?? ????, 8=??, 9=??, 10=?, 11=?? ?? ??,12=???,13=?&?? ??,14=?? ?? ??,15=???,16=?? ??
	, @inyItemPotentialMoveCount tinyint = NULL OUTPUT --// ??????
	, @inyItemCount tinyint = NULL OUTPUT --// ?? ??? ??
	, @inbItemPrice bigint = NULL OUTPUT --// ??? ??

--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @dtNowDate datetime = GETDATE()
      , @inbCurrentCoin bigint
      , @inbChangedCoin bigint
      , @inbCoinBefore bigint
      , @intAuctionItemsHistoryID int
      , @insItemRemainCountBefore smallint
      , @inyCoinChangeCode tinyint
      , @inyItemLogCode tinyint
      , @inbOwnerCharacterIDBefore bigint
      , @intIP int = dbo.FN_IP2IPNumber('0.0.0.0');

DECLARE @tblAuctionItemsHistory TABLE
( ItemID int NOT NULL
, ItemRemainCount smallint NOT NULL
, PayMethodCode tinyint NULL
, SenderCharacterID bigint NULL
, ItemMaterializeCode tinyint NOT NULL
, ItemMaterializeFKey bigint NULL
, ItemDurability smallint NOT NULL
, RandomSeed int NOT NULL
, CoolTime int NOT NULL
, ItemLevel tinyint NOT NULL
, ItemPotential tinyint NOT NULL
, SoulBoundFlag bit NOT NULL
, SealCount tinyint NOT NULL
, ItemOption tinyint NOT NULL
, ItemMaterializeDate datetime2(0) NOT NULL
, ItemLifespan int NULL
, EternityFlag bit NOT NULL
, ItemExpireDate smalldatetime NULL
, OwnershipStartDate datetime2(0) NULL
, ExpireCompleteFlag bit NOT NULL
, TradeItemFlag bit NULL
, AdditiveItemID int NULL
, ItemLocationCode tinyint NULL
, ItemPotentialMoveCount tinyint NULL
, ItemCount	smallint NULL
, ItemPrice	bigint NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SET @inyCoinChangeCode = CASE WHEN @inyAuctionItemsHistoryStatusCode =  0 THEN 35 -- (DOORS) ?????
	                              WHEN @inyAuctionItemsHistoryStatusCode =  2 THEN 36 -- (DOORS) ?????
	                              WHEN @inyAuctionItemsHistoryStatusCode =  3 THEN 38 -- (DOORS) ????
	                              WHEN @inyAuctionItemsHistoryStatusCode =  5 THEN 39 -- (DOORS) ????
	                              WHEN @inyAuctionItemsHistoryStatusCode =  6 THEN 37 -- (DOORS) ?????
	                              WHEN @inyAuctionItemsHistoryStatusCode =  7 THEN 40 -- (DOORS) ???????
	                              WHEN @inyAuctionItemsHistoryStatusCode =  9 THEN 41 -- (DOORS) ???????
	                              WHEN @inyAuctionItemsHistoryStatusCode = 10 THEN 43 -- (DOORS) ??????
	                              WHEN @inyAuctionItemsHistoryStatusCode = 12 THEN 44 -- (DOORS) ??????
	                              WHEN @inyAuctionItemsHistoryStatusCode = 13 THEN 42 -- (DOORS) ???????
	                         ELSE NULL END;

	SET @inyItemLogCode = CASE WHEN @inyAuctionItemsHistoryStatusCode =  0 THEN 58 -- (DOORS) ????
	                           WHEN @inyAuctionItemsHistoryStatusCode =  1 THEN 58
	                           WHEN @inyAuctionItemsHistoryStatusCode =  3 THEN 59 -- (DOORS) ????
	                           WHEN @inyAuctionItemsHistoryStatusCode =  4 THEN 59
	                           WHEN @inyAuctionItemsHistoryStatusCode =  7 THEN 60 -- (DOORS) ??????
	                           WHEN @inyAuctionItemsHistoryStatusCode =  8 THEN 60
	                      ELSE NULL END;
	-- 0=?? (???? ??? ?? : ????? + ??????)
	-- 1=?? (???? ??? ?? : ?????)
	IF @inyAuctionItemsHistoryStatusCode IN (0, 1)
	BEGIN
		BEGIN TRANSACTION
			-- ??? ??
			IF @inyAuctionItemsHistoryStatusCode = 0
			BEGIN
				UPDATE dbo.CharacterStatus
				   SET @inbCoinBefore = Coin
					 , @inbCurrentCoin = Coin = Coin - @intCoin
				WHERE CharacterID = @inbOwnerCharacterID;

				IF @@ROWCOUNT = 0
				BEGIN
					SET @intReturnValue = 103102; -- ???? ???? ????.
					GOTO ErrorHandler;
				END
			END

			-- ??? ??
			DELETE dbo.MaterializedItems
			OUTPUT deleted.ItemID, deleted.ItemRemainCount, deleted.PayMethodCode, deleted.SenderCharacterID, deleted.ItemMaterializeCode, deleted.ItemMaterializeFKey, deleted.ItemDurability, deleted.RandomSeed, deleted.CoolTime, deleted.ItemLevel, deleted.ItemPotential, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemMaterializeDate, deleted.ItemLifespan, deleted.EternityFlag, deleted.ItemExpireDate, deleted.OwnershipStartDate, deleted.ExpireCompleteFlag, deleted.TradeItemFlag, deleted.AdditiveItemID, deleted.ItemLocationCode, deleted.ItemPotentialMoveCount, deleted.ItemCount, deleted.ItemPrice
			  INTO @tblAuctionItemsHistory (ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount, ItemCount, ItemPrice)
			 WHERE ItemSerial = @inbItemSerial
			   AND OwnerCharacterID = @inbOwnerCharacterID
			   AND ItemRemainCount > 0
			   AND ItemLocationCode IN (2, 5);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103151; -- ???? ???? ????.
				GOTO ErrorHandler;
			END

			SELECT @intItemID = ItemID
			     , @insItemRemainCount = ItemRemainCount
			     , @inyPayMethodCode = PayMethodCode
			     , @inbSenderCharacterID = SenderCharacterID
			     , @inyItemMaterializeCode = ItemMaterializeCode
			     , @inbItemMaterializeFKey = ItemMaterializeFKey
			     , @insItemDurability = ItemDurability
			     , @intRandomSeed = RandomSeed
			     , @intCoolTime = CoolTime
			     , @inyItemLevel = ItemLevel
			     , @inyItemPotential = ItemPotential
			     , @bitSoulBoundFlag = SoulBoundFlag
			     , @inySealCount = SealCount
			     , @inyItemOption = ItemOption
			     , @dt2ItemMaterializeDate = ItemMaterializeDate
			     , @intItemLifespan = ItemLifespan
			     , @bitEternityFlag = EternityFlag
			     , @sdtItemExpireDate = ItemExpireDate
			     , @dt2OwnershipStartDate = OwnershipStartDate
			     , @bitExpireCompleteFlag = ExpireCompleteFlag
			     , @bitTradeItemFlag = TradeItemFlag
			     , @intAdditiveItemID = AdditiveItemID
			     , @inyItemLocationCode = ItemLocationCode
			     , @inyItemPotentialMoveCount = ItemPotentialMoveCount
				 , @inyItemCount = ItemCount
				 , @inbItemPrice = ItemPrice
			  FROM @tblAuctionItemsHistory;

			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate, ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount, ItemCount, ItemPrice)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, @intCoin, @dtNowDate, @inbItemSerial, @intItemID, @insItemRemainCount, @inyPayMethodCode, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag, @sdtItemExpireDate, @inbOwnerCharacterID, @dt2OwnershipStartDate, @bitExpireCompleteFlag, @bitTradeItemFlag, @intAdditiveItemID, @inyItemLocationCode, @inyItemPotentialMoveCount, @inyItemCount, @inbItemPrice);

			SET @inbOwnerCharacterIDBefore = @inbOwnerCharacterID;
			SET @inbOwnerCharacterID = NULL;
			SET @insItemRemainCountBefore = @insItemRemainCount;
			SET @insItemRemainCount = 0

		COMMIT TRANSACTION
	END
	-- 2=?? (???? ??? ?? : ??????)
	-- 6=??:????? (???? ??, ???? ??? ?? : ??????)
	ELSE IF @inyAuctionItemsHistoryStatusCode IN (2, 6)
	BEGIN
		BEGIN TRANSACTION
			-- ??? ??
			UPDATE dbo.CharacterStatus
			   SET @inbCoinBefore = Coin
				 , @inbCurrentCoin = Coin = Coin - @intCoin
			WHERE CharacterID = @inbOwnerCharacterID;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103102; -- ???? ???? ????.
				GOTO ErrorHandler;
			END

			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, @intCoin, @dtNowDate);

		COMMIT TRANSACTION
	END
	-- 3=??:????? (???? ??? ?? : ??????+?????)
	-- 4=??:????? (???? ??? ?? : ?????)
	ELSE IF @inyAuctionItemsHistoryStatusCode IN (3, 4)
	BEGIN
		BEGIN TRANSACTION
			-- ??? ??
			IF @inyAuctionItemsHistoryStatusCode = 3
			BEGIN
				SET @inyPayMethodCode = 1;

				UPDATE dbo.CharacterStatus
				   SET @inbCoinBefore = Coin
					 , @inbCurrentCoin = Coin = Coin - @intCoin
				WHERE CharacterID = @inbOwnerCharacterID;

				IF @@ROWCOUNT = 0
				BEGIN
					SET @intReturnValue = 103102; -- ???? ???? ????.
					GOTO ErrorHandler;
				END
			END
			ELSE
			BEGIN
				SET @inyPayMethodCode = 3;
			END

			SET @inbSenderCharacterID = NULL;
			SET @inyItemMaterializeCode = 31;
			SET @inbItemMaterializeFKey = @intAuctionItemsHistoryStatusFKey;
			SET @dt2ItemMaterializeDate = @dtNowDate;
			SET @dt2OwnershipStartDate = @dtNowDate;
			SET @inbItemSerial = ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint));
			SET @inbOwnerCharacterIDBefore = NULL;
			SET @insItemRemainCountBefore = 0;

			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount, ItemCount, ItemPrice)
			VALUES (@inbItemSerial, @intItemID, @insItemRemainCount, @inyPayMethodCode, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag, @sdtItemExpireDate, @inbOwnerCharacterID, @dt2OwnershipStartDate, @bitExpireCompleteFlag, @bitTradeItemFlag, @intAdditiveItemID, @inyItemLocationCode, @inyItemPotentialMoveCount, @inyItemCount, @inbItemPrice);

			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate, ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount, ItemCount, ItemPrice)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, @intCoin, @dtNowDate, @inbItemSerial, @intItemID, @insItemRemainCount, @inyPayMethodCode, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag, @sdtItemExpireDate, @inbOwnerCharacterID, @dt2OwnershipStartDate, @bitExpireCompleteFlag, @bitTradeItemFlag, @intAdditiveItemID, @inyItemLocationCode, @inyItemPotentialMoveCount, @inyItemCount, @inbItemPrice);

		COMMIT TRANSACTION
	END
	-- 5=??:????? (???? ??? ?? : ??????)
	ELSE IF @inyAuctionItemsHistoryStatusCode = 5
	BEGIN
		BEGIN TRANSACTION
			-- ??? ??
			UPDATE dbo.CharacterStatus
			   SET @inbCoinBefore = Coin
				 , @inbCurrentCoin = Coin = Coin + @intCoin
			WHERE CharacterID = @inbOwnerCharacterID;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103102; -- ???? ???? ????.
				GOTO ErrorHandler;
			END

			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, @intCoin, @dtNowDate);

		COMMIT TRANSACTION
	END
	-- 7=0? ?? ??(??)
	ELSE IF @inyAuctionItemsHistoryStatusCode = 7
	BEGIN
		BEGIN TRANSACTION
			-- ?? ??? ?? ? ?? ?? ??
			SELECT @intAuctionItemsHistoryID = AuctionItemsHistoryID
			     , @intCoin = Coin
			  FROM dbo.AuctionItemsHistory WITH (NOLOCK)
			 WHERE (AuctionItemsHistoryStatusFKey = @intAuctionItemsHistoryStatusFKey)
			   AND (AuctionItemsHistoryStatusCode = 0);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103366; -- ?? ??? ?? ? ?? ??? ???? ????.
				GOTO ErrorHandler;
			END

			-- ??? ??
			UPDATE dbo.CharacterStatus
			   SET @inbCoinBefore = Coin
				 , @inbCurrentCoin = Coin = Coin + @intCoin
			WHERE CharacterID = @inbOwnerCharacterID;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103102; -- ???? ???? ????.
				GOTO ErrorHandler;
			END

			-- ??? ??
			SELECT @inbItemSerial = ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint))
			     , @intItemID = ItemID
			     , @insItemRemainCount = ItemRemainCount
			     , @inyPayMethodCode = PayMethodCode
			     , @inbSenderCharacterID = SenderCharacterID
			     , @inyItemMaterializeCode = ItemMaterializeCode
			     , @inbItemMaterializeFKey = ItemMaterializeFKey
			     , @insItemDurability = ItemDurability
			     , @intRandomSeed = RandomSeed
			     , @intCoolTime = CoolTime
			     , @inyItemLevel = ItemLevel
			     , @inyItemPotential = ItemPotential
			     , @bitSoulBoundFlag = SoulBoundFlag
			     , @inySealCount = SealCount
			     , @inyItemOption = ItemOption
			     , @dt2ItemMaterializeDate = ItemMaterializeDate
			     , @intItemLifespan = ItemLifespan
			     , @bitEternityFlag = EternityFlag
			     , @sdtItemExpireDate = ItemExpireDate
			     , @dt2OwnershipStartDate = OwnershipStartDate
			     , @bitExpireCompleteFlag = ExpireCompleteFlag
			     , @bitTradeItemFlag = TradeItemFlag
			     , @intAdditiveItemID = AdditiveItemID
			     , @inyItemLocationCode = ItemLocationCode
			     , @inyItemPotentialMoveCount = ItemPotentialMoveCount
				 , @inyItemCount = ItemCount
				 , @inbItemPrice = ItemPrice
			  FROM dbo.AuctionItemsHistory WITH (NOLOCK)
			 WHERE AuctionItemsHistoryID = @intAuctionItemsHistoryID;

			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount, ItemCount, ItemPrice)
			VALUES (@inbItemSerial, @intItemID, @insItemRemainCount, @inyPayMethodCode, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag, @sdtItemExpireDate, @inbOwnerCharacterID, @dt2OwnershipStartDate, @bitExpireCompleteFlag, @bitTradeItemFlag, @intAdditiveItemID, @inyItemLocationCode, @inyItemPotentialMoveCount, @inyItemCount, @inbItemPrice);

			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate, ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount, ItemCount, ItemPrice)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, @intCoin, @dtNowDate, @inbItemSerial, @intItemID, @insItemRemainCount, @inyPayMethodCode, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag, @sdtItemExpireDate, @inbOwnerCharacterID, @dt2OwnershipStartDate, @bitExpireCompleteFlag, @bitTradeItemFlag, @intAdditiveItemID, @inyItemLocationCode, @inyItemPotentialMoveCount, @inyItemCount, @inbItemPrice);

		COMMIT TRANSACTION
	END
	-- 8=1? ?? ??(??)
	ELSE IF @inyAuctionItemsHistoryStatusCode = 8
	BEGIN
		BEGIN TRANSACTION
			-- ??? ??
			SELECT @inbItemSerial = ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint))
			     , @intItemID = ItemID
			     , @insItemRemainCount = ItemRemainCount
			     , @inyPayMethodCode = PayMethodCode
			     , @inbSenderCharacterID = SenderCharacterID
			     , @inyItemMaterializeCode = ItemMaterializeCode
			     , @inbItemMaterializeFKey = ItemMaterializeFKey
			     , @insItemDurability = ItemDurability
			     , @intRandomSeed = RandomSeed
			     , @intCoolTime = CoolTime
			     , @inyItemLevel = ItemLevel
			     , @inyItemPotential = ItemPotential
			     , @bitSoulBoundFlag = SoulBoundFlag
			     , @inySealCount = SealCount
			     , @inyItemOption = ItemOption
			     , @dt2ItemMaterializeDate = ItemMaterializeDate
			     , @intItemLifespan = ItemLifespan
			     , @bitEternityFlag = EternityFlag
			     , @sdtItemExpireDate = ItemExpireDate
			     , @dt2OwnershipStartDate = OwnershipStartDate
			     , @bitExpireCompleteFlag = ExpireCompleteFlag
			     , @bitTradeItemFlag = TradeItemFlag
			     , @intAdditiveItemID = AdditiveItemID
			     , @inyItemLocationCode = ItemLocationCode
			     , @inyItemPotentialMoveCount = ItemPotentialMoveCount
				 , @inyItemCount = ItemCount
				 , @inbItemPrice = ItemPrice
			  FROM dbo.AuctionItemsHistory WITH (NOLOCK)
			 WHERE (AuctionItemsHistoryStatusFKey = @intAuctionItemsHistoryStatusFKey)
			   AND (AuctionItemsHistoryStatusCode = 1);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103366; -- ?? ??? ?? ? ?? ??? ???? ????.
				GOTO ErrorHandler;
			END
			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount, ItemCount, ItemPrice)
			VALUES (@inbItemSerial, @intItemID, @insItemRemainCount, @inyPayMethodCode, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag, @sdtItemExpireDate, @inbOwnerCharacterID, @dt2OwnershipStartDate, @bitExpireCompleteFlag, @bitTradeItemFlag, @intAdditiveItemID, @inyItemLocationCode, @inyItemPotentialMoveCount, @inyItemCount, @inbItemPrice);

			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate, ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount, ItemCount, ItemPrice)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, NULL, @dtNowDate, @inbItemSerial, @intItemID, @insItemRemainCount, @inyPayMethodCode, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag, @sdtItemExpireDate, @inbOwnerCharacterID, @dt2OwnershipStartDate, @bitExpireCompleteFlag, @bitTradeItemFlag, @intAdditiveItemID, @inyItemLocationCode, @inyItemPotentialMoveCount, @inyItemCount, @inbItemPrice);

		COMMIT TRANSACTION
	END
	-- 9=2? ?? ??(??)
	ELSE IF @inyAuctionItemsHistoryStatusCode = 9
	BEGIN
		BEGIN TRANSACTION
			-- ?? ??? ?? ? ?? ?? ??
			SELECT @intCoin = Coin
			  FROM dbo.AuctionItemsHistory WITH (NOLOCK)
			 WHERE (AuctionItemsHistoryStatusFKey = @intAuctionItemsHistoryStatusFKey)
			   AND (AuctionItemsHistoryStatusCode = 2);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103366; -- ?? ??? ?? ? ?? ??? ???? ????.
				GOTO ErrorHandler;
			END

			-- ??? ??
			UPDATE dbo.CharacterStatus
			   SET @inbCoinBefore = Coin
				 , @inbCurrentCoin = Coin = Coin + @intCoin
			WHERE CharacterID = @inbOwnerCharacterID;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103102; -- ???? ???? ????.
				GOTO ErrorHandler;
			END

			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, @intCoin, @dtNowDate);

		COMMIT TRANSACTION
	END
	-- 12=5? ?? ??(??)
	ELSE IF @inyAuctionItemsHistoryStatusCode = 12
	BEGIN
		BEGIN TRANSACTION
			-- ?? ??? ?? ? ?? ?? ??
			SELECT @intCoin = Coin
			  FROM dbo.AuctionItemsHistory WITH (NOLOCK)
			 WHERE (AuctionItemsHistoryStatusFKey = @intAuctionItemsHistoryStatusFKey)
			   AND (AuctionItemsHistoryStatusCode = 5);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103366; -- ?? ??? ?? ? ?? ??? ???? ????.
				GOTO ErrorHandler;
			END

			-- ??? ??
			UPDATE dbo.CharacterStatus
			   SET @inbCoinBefore = Coin
				 , @inbCurrentCoin = Coin = Coin - @intCoin
			WHERE CharacterID = @inbOwnerCharacterID;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103102; -- ???? ???? ????.
				GOTO ErrorHandler;
			END
			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, @intCoin, @dtNowDate);

		COMMIT TRANSACTION
	END
	-- 13=6? ?? ??(??)
	ELSE IF @inyAuctionItemsHistoryStatusCode = 13
	BEGIN
		BEGIN TRANSACTION
			-- ?? ??? ?? ? ?? ?? ??
			SELECT @intCoin = Coin
			  FROM dbo.AuctionItemsHistory WITH (NOLOCK)
			 WHERE (AuctionItemsHistoryStatusFKey = @intAuctionItemsHistoryStatusFKey)
			   AND (AuctionItemsHistoryStatusCode = 6);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103366; -- ?? ??? ?? ? ?? ??? ???? ????.
				GOTO ErrorHandler;
			END

			-- ??? ??
			UPDATE dbo.CharacterStatus
			   SET @inbCoinBefore = Coin
				 , @inbCurrentCoin = Coin = Coin + @intCoin
			WHERE CharacterID = @inbOwnerCharacterID;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103102; -- ???? ???? ????.
				GOTO ErrorHandler;
			END

			-- ?? ??? ?? ? ?? ?? ??
			INSERT dbo.AuctionItemsHistory (AuctionItemsHistoryStatusCode, AuctionItemsHistoryStatusFKey, Coin, RegisterDate)
			VALUES (@inyAuctionItemsHistoryStatusCode, @intAuctionItemsHistoryStatusFKey, @intCoin, @dtNowDate);

		COMMIT TRANSACTION
	END
	
	-- ?? ?? ??
	IF @intCoin <> 0
	BEGIN
		SET @inbChangedCoin = @inbCurrentCoin - @inbCoinBefore;

		SET @inbOwnerCharacterID = ISNULL(@inbOwnerCharacterID, @inbOwnerCharacterIDBefore)
		EXEC @intReturnValue = dbo.P_AddCoinLog
			  @dt2LogDate = @dtNowDate
			, @inbCharacterID = @inbOwnerCharacterID
			, @inyCoinTypeCode = 1
			, @inyCoinChangeCode = @inyCoinChangeCode 
			, @inbCurrentCoin = @inbCurrentCoin
			, @inbChangedCoin = @inbChangedCoin
			, @intChannelID = 0
			, @intMapID = 0
			, @inbCoinChangeFKey = @intAuctionItemsHistoryStatusFKey;
	END

	-- ??? ?? ??
	EXEC @intReturnValue = dbo.P_AddItemLog
		  @dt2LogDate = @dtNowDate
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = @inyItemLogCode
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterIDBefore
		, @insItemCount = @insItemRemainCount
		, @insItemCountBefore = @insItemRemainCountBefore
		, @inbItemLogFKey1 = @intAuctionItemsHistoryStatusFKey
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemLevel = @inyItemLevel
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_CharacterStatus_Coin%'
	BEGIN
		SET @intReturnValue = 103123; -- ??? ?????.
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
