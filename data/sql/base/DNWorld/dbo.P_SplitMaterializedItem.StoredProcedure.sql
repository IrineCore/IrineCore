USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SplitMaterializedItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 27
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-29
description : ???? ?? ??? ???? ? ItemSerial? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103156 = ItemSerial ?????.
103170 = ???? ???? ??? ??? ??? ?????.
103192 = @insItemCount ?? ?????.
history :
9 = @inyCallProc? ?? ?? (4) at 2010-10-05
10 = input param. @intGuildID ?? at 2010-10-06
11 = @inyCallProc? ?? ?? (5) at 2010-10-07
12 = input param. @inbNewOwnerCharacterID2 ?? at 2010-10-07
13 = @inyCallProc? ?? ?? (6) at 2010-10-07
14 = @inyCallProc ?? 5?? Split? ???? ??? ?? ?? ?? at 2010-10-13
15 = ??? ?? ?? at 2011-03-18
16 = ??? ?? ???? ?? ?? ?? at 2011-03-22
17 = ??? ??? ?? ?? at 2011-03-29
18 = ???? at 2011-03-29
20 = ItemLocation.MaterializedItemID ?? ??? ?? ?? at 2011-03-31
22~23 = ItemLog? Split ??? ?? ??? ?? by ??? at 2012-02-24
25 = ?????? ??? ???? ?? ???? ?? ?? ?? by ??? at 2012-07-18
**/

CREATE PROCEDURE [dbo].[P_SplitMaterializedItem]
	  @dt2SplitDate datetime2(0) --// ??? ??? ??
	, @inbMaterializedItemID bigint --// ?? ???
	, @inbNewItemSerial bigint --// ? ??? ItemSerial
	, @insItemCount smallint --// ? ??? ItemSerial? ??? ??? ?? (?? ???? ???? ??????.)
	, @bitInitOwnerCharacter bit --// ?? ??? ??? ??. 0=?? ???? ???? ??, 1=??? ??? ??? (?? ??, ?? ?)
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @inyCallProc tinyint = NULL --// ? SP? ??? SP. 1=P_AddTrade, 2=P_ExchangeProperty, 3=P_ChangeItemLocation, 4=P_MoveItemIntoGuildDepot, 5=P_ChangeItemLocationForGuildDepot, 6=P_SplitItemInGuildDepot
	, @intGuildID int = NULL --// ? SP? ??? SP? P_MoveItemIntoGuildDepot, P_ChangeItemLocationForGuildDepot? ?? Guild ID? ?????.
	, @inbNewOwnerCharacterID2 bigint = NULL --// ? SP? ??? SP? P_ChangeItemLocationForGuildDepot? ?? Split? ???? ??? CharacterID? ?????.
	, @inbNewMaterializedItemID bigint = NULL OUTPUT --// ? ??? MaterializedItemID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inbItemSerial bigint
	, @intItemID int
	, @insItemRemainCount smallint
	, @inyPayMethodCode tinyint
	, @inbSenderCharacterID bigint
	, @inyItemMaterializeCode tinyint
	, @inbItemMaterializeFKey bigint
	, @insItemDurability smallint
	, @intRandomSeed int
	, @intCoolTime int
	, @inyItemLevel tinyint
	, @inyItemPotential tinyint
	, @bitSoulBoundFlag bit
	, @inySealCount tinyint
	, @inyItemOption tinyint
	, @dt2ItemMaterializeDate datetime2(0)
	, @intItemLifespan int
	, @bitEternityFlag bit
	, @sdtItemExpireDate smalldatetime
	, @inbOwnerCharacterID bigint
	, @dt2OwnershipStartDate datetime2(0)
	, @bitExpireCompleteFlag bit
	, @bitTradeItemFlag bit
	, @insItemCountBefore smallint
	, @insItemCountAfter smallint
	, @inbNewOwnerCharacterID bigint
	, @insOriginalItemCountAfter smallint
	, @inyItemLocationCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @insItemCount <= 0
	BEGIN
		SET @intReturnValue = 103192;
		GOTO ErrorHandler;
	END

	SELECT @inbItemSerial = ItemSerial
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
		, @inbOwnerCharacterID = OwnerCharacterID
		, @dt2OwnershipStartDate = OwnershipStartDate
		, @bitExpireCompleteFlag = ExpireCompleteFlag
		, @bitTradeItemFlag = TradeItemFlag
		, @insItemCountBefore = ItemRemainCount
		, @insItemCountAfter = ItemRemainCount - @insItemCount
		, @inyItemLocationCode = ItemLocationCode
	FROM dbo.MaterializedItems WITH (NOLOCK)
	WHERE MaterializedItemID = @inbMaterializedItemID AND ItemRemainCount > @insItemCount;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103170;
		GOTO ErrorHandler;
	END

	INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, ItemLocationCode)
	VALUES (@inbNewItemSerial, @intItemID, @insItemCount, @inyPayMethodCode, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemSerial, @insItemDurability, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag, @sdtItemExpireDate, CASE @bitInitOwnerCharacter WHEN 0 THEN CASE WHEN @inyCallProc = 5 THEN @inbNewOwnerCharacterID2 ELSE @inbOwnerCharacterID END WHEN 1 THEN NULL END, CASE @bitInitOwnerCharacter WHEN 0 THEN @dt2SplitDate ELSE NULL END, @bitExpireCompleteFlag, @bitTradeItemFlag, @inyItemLocationCode);

	SET @inbNewMaterializedItemID = SCOPE_IDENTITY();

	UPDATE dbo.MaterializedItems
	SET @insOriginalItemCountAfter = ItemRemainCount -= @insItemCount
	WHERE MaterializedItemID = @inbMaterializedItemID;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 35
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = @insItemCountAfter
		, @insItemCountBefore = @insItemCountBefore
		, @inbItemLogFKey1 = @inbNewItemSerial
		, @inbItemLogFKey2 = @inyCallProc
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL;

	SET @inbNewOwnerCharacterID = CASE @bitInitOwnerCharacter WHEN 0 THEN @inbOwnerCharacterID WHEN 1 THEN NULL END;
	
	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbNewItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 36
		, @inbOwnerCharacterID = @inbNewOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = 0
		, @inbItemLogFKey1 = @inbItemSerial
		, @inbItemLogFKey2 = @inyCallProc
		, @dt2OwnershipStartDate = NULL
		, @intIP = NULL;

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
	EXEC @intReturnValue = dbo.P_AddErrorLog;
RETURN @intReturnValue;
GO
