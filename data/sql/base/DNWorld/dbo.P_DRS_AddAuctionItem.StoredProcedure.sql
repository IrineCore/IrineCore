USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddAuctionItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-22
description : ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000026 = ??? ???? ????.
1030000029 = ???? ??? ??? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_AddAuctionItem]
	  @inbCharacterID bigint --// ???ID	
	, @inbStartPrice bigint --// ?????
	, @inbCommission bigint --// ?????
	, @inbBuynowPrice bigint --// ?????
	, @sdtBiddingEndDate smalldatetime --// ??????
	, @intItemID int --// ???ID
	, @nvcItemName nvarchar(50) --// ??? ??
	, @inyItemRankCode tinyint --// 0=??, 1=??, 2=??, 3=??, 4=???, 5=???
	, @inyLevelLimit tinyint --// ???? ???? ?? ?? ??? ??
	, @inyJobCode tinyint --// 1=???, 2=??, 3=????, 4=???, 5=????, 10=???, 11=?????, 12=????, 13=???, 14=?????, 15=????, 16=??, 17=??????, 18=????, 19=??, 20=???, 21=??, 22=????, 23=??????, 24=???, 25=????, 26=??????, 27=?????, 28=?????, 29=????, 30=????, 31=????, 32=????, 33=???, 34=?????, 35=????, 36=?????, 37=???, 38=????, 39=?????, 40=??????, 41=???, 42=?????, 43=???, 44=????, 45=?????, 46=????, 47=????, 48=?????, 49=?????, 50=???, 51=???
	, @bitCashItemFlag bit --// 0=?? ???, 1=?? ???
	, @inyItemCategoryID tinyint --// ???? ?? ???? ID
	, @inbItemSerial bigint --// ??????	
	, @inyItemLevel tinyint --// ??? ????
	, @insItemCount smallint --// ??? ??
	, @inyItemOption tinyint --// ??? ??
	, @inyItemPotential tinyint --// ??? ???
	, @inySealCount tinyint --// ??? ????
	, @insItemDurability smallint --// ??? ???
	, @inyItemPotentialMoveCount tinyint --// ??? ??????
	, @intRandomSeed int --// ??? ????
	, @intAuctionID int = NULL OUTPUT --// ??ID
	, @inyCharacterLevel tinyint = NULL OUTPUT --// ?????
	, @intCharacterExp int = NULL OUTPUT --// ??????
	, @inbCoin bigint = NULL OUTPUT --// ??? ??
	, @inbWarehouseCoin bigint = NULL OUTPUT --// ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
		, @dt2Now datetime2(0) = GETDATE()
		, @inbCurrentCoin bigint
		, @inbChangedCoin int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	/**_# ???? ??? ??? ??? ?? */
	SELECT @inbCurrentCoin = Coin
	FROM dbo.CharacterStatus WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK)
	WHERE CharacterID = @inbCharacterID;

	IF @inbCurrentCoin < @inbCommission
	BEGIN
		SET @intReturnValue = 1030000029;
		GOTO ErrorHandler;
	END

	/**_# ???? ????? ?? */
	IF NOT EXISTS (SELECT * FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) WHERE ItemSerial = @inbItemSerial AND ItemID = @intItemID AND ItemLocationCode IN (2,5))
	BEGIN
		SET @intReturnValue = 1030000026;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;

	/**_# [MaterializedItems] ???? ???? ???? ?? ?? ???. */
	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = 22, ItemLocationIndex = 0, OwnerCharacterID = NULL	
	WHERE ItemSerial = @inbItemSerial AND ItemID = @intItemID;



	/**_# [Auctions] ???? INSERT???.*/
	INSERT dbo.Auctions (CharacterID, CategoryCode, StartPrice, Commission, BuynowPrice, AuctionStatusCode, BiddingEndDate, RegistrationDate, UpdateDate, ItemID, ItemSerial, ItemLevel, ItemCount, ItemOption, ItemPotential, SealCount, ItemDurability, ItemPotentialMoveCount, RandomSeed)
	VALUES (@inbCharacterID, @inyItemCategoryID, @inbStartPrice, @inbCommission, @inbBuynowPrice, 0, @sdtBiddingEndDate, @dt2Now, @dt2Now, @intItemID, @inbItemSerial, @inyItemLevel, @insItemCount, @inyItemOption, @inyItemPotential, @inySealCount, @insItemDurability, @inyItemPotentialMoveCount, @intRandomSeed);
	SET @intAuctionID = SCOPE_IDENTITY();
	/**_# ?? ?? ???? ?????. */
	UPDATE dbo.CharacterStatus
	SET Coin = Coin - @inbCommission
		, @inyCharacterLevel = CharacterLevel
		, @intCharacterExp = CharacterExp
		, @inbCoin = @inbCurrentCoin - @inbCommission
		, @inbWarehouseCoin = WarehouseCoin
	WHERE CharacterID = @inbCharacterID;	
 
	COMMIT TRANSACTION;

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

	SET @inbChangedCoin = @inbCommission * -1;
	SET @inbCurrentCoin = @inbCurrentCoin - @inbCommission

	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dt2Now
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = 35
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @inbChangedCoin
		, @intChannelID = NULL
		, @intMapID = NULL
		, @inbCoinChangeFKey = NULL;

END TRY
BEGIN CATCH
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
