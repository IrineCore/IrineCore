USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ResellMultiItems]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-09-05
description : ???? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ResellMultiItems]
	  @xmlItemSerials xml = NULL --// ??? ?? ??   <root><Item ItemSerial="" /></root>
	, @inbCharacterID bigint --// ???ID  
	, @inbResellPrice bigint --// ??? ??
	, @intNPCID int --// ??? NPC ID
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @intLogTableSuffix int
      , @intDeleteItemCount int
      , @inbCurrentCoin bigint
      , @dt2LogDate datetime2(0) = GETDATE();
      
DECLARE @tblItemSerial TABLE 
(
	ItemSerial bigint
)

DECLARE @tblDeletedItems TABLE 
(
	MaterializedItemID bigint
  , ItemSerial bigint
  , ItemID int
  , ItemRemainCount smallint
  , SealCount tinyint
  , SoulBoundFlag bit
  , ItemOption tinyint
  , ItemPotential tinyint
  , ItemLevel tinyint
  , OwnershipStartDate datetime2(0)
  , ItemDurability smallint
)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	/**_# xml???? ?????? ?????.*/
	INSERT @tblItemSerial (ItemSerial)
	SELECT T.col.value('@ItemSerial', 'bigint') AS ItemSerial
	FROM @xmlItemSerials.nodes('/root/Item') T(col)

	SET @intDeleteItemCount = @@ROWCOUNT
	
	/**_# ??? ??? ??? ?????.*/
	INSERT @tblDeletedItems (MaterializedItemID, ItemSerial, ItemID, ItemRemainCount, SealCount, SoulBoundFlag, ItemOption, ItemPotential, ItemLevel, OwnershipStartDate, ItemDurability)
	SELECT B.MaterializedItemID
		 , B.ItemSerial
	     , B.ItemID
	     , B.ItemRemainCount
	     , B.SealCount
	     , B.SoulBoundFlag
	     , B.ItemOption
	     , B.ItemPotential
	     , B.ItemLevel
	     , B.OwnershipStartDate
	     , B.ItemDurability
	FROM @tblItemSerial A
		INNER JOIN dbo.MaterializedItems B WITH(NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON A.ItemSerial = B.ItemSerial
	WHERE B.OwnerCharacterID = @inbCharacterID
      AND B.ItemLocationCode = 2
      
	IF @intDeleteItemCount <> @@ROWCOUNT
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	/**_# ???? ?????.*/
	DELETE dbo.MaterializedItems
	FROM @tblDeletedItems A
		INNER JOIN dbo.MaterializedItems B WITH(INDEX(PK_MaterializedItems), FORCESEEK) ON A.MaterializedItemID = B.MaterializedItemID
		
	/**_# [CharacterStatus].[Coin]? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @inbCurrentCoin = Coin += @inbResellPrice
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);
	
	/**_# ??? ?????.,*/
	IF @intLogTableSuffix = 0
		INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevelBefore)
		SELECT @dt2LogDate, ItemSerial, ItemID, 22, NULL, @inbCharacterID, 0, ItemRemainCount, @intMapID, @intNPCID, OwnershipStartDate, dbo.FN_IP2IPNumber(@vchIP), SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevel
		FROM @tblDeletedItems;

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevelBefore)
		SELECT @dt2LogDate, ItemSerial, ItemID, 22, NULL, @inbCharacterID, 0, ItemRemainCount, @intMapID, @intNPCID, OwnershipStartDate, dbo.FN_IP2IPNumber(@vchIP), SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevel
		FROM @tblDeletedItems;

	ELSE
		INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevelBefore)
		SELECT @dt2LogDate, ItemSerial, ItemID, 22, NULL, @inbCharacterID, 0, ItemRemainCount, @intMapID, @intNPCID, OwnershipStartDate, dbo.FN_IP2IPNumber(@vchIP), SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevel
		FROM @tblDeletedItems;
		
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
