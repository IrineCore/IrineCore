USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMiniTrade]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-08-09
description : ?? ???? ???? ??? ??? ?????.
result set : 
	@intVersion = 1 : TradeID/int, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, ListPrice/int, SealCount/tinyint, PremiumFlag/bit, PayMethodCode/tinyint, ItemPotentialMoveCount/tinyint
	@intVersion = 2 : Version 1 + DragonJewelType/tinyint, ItemSerial/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103171 = ??? ???? ???? ????.
history :
4 = PayMethodCode? 1? ??? ???? ????? ?? by ??? at 2012-11-08
5 = input param @inyPayMethodCode ?? by ??? at 2012-11-15
7~8 = ?? ?? ?? ??(@intVersion = 2) by ??? at 2013-04-04
**/

CREATE PROCEDURE [dbo].[P_GetListMiniTrade]
	  @intItemID int --// ??? ID
	, @inbCharacterID bigint = 0 --// ???? ???? ??? ID
	, @inyPayMethodCode tinyint = 1 --// 0=??, 1=????, 3=??
	, @intVersion int = 1 --// ??? SP? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyPayMethodCode = 0
	BEGIN
		IF @intVersion = 1 		
			WITH A AS (
				SELECT TradeID
					, ROW_NUMBER() OVER (PARTITION BY PremiumFlag ORDER BY ListPrice) AS Seq
				FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNIF_Trade_ItemID), FORCESEEK)
				WHERE ItemID = @intItemID
					AND TradeStatusCode = 0
					AND TradeExpireDate > GETDATE()
					AND CharacterID <> @inbCharacterID
			), B AS (
				SELECT TradeID, Seq
				FROM A
				WHERE Seq <= 25
			)
			SELECT TR.TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, ListPrice, SealCount, PremiumFlag, PayMethodCode, ItemPotentialMoveCount
			FROM B
				INNER LOOP JOIN dbo.Trade TR WITH (NOLOCK, INDEX(PK_Trade), FORCESEEK) ON B.TradeID = TR.TradeID
			ORDER BY PremiumFlag DESC, Seq
			OPTION (KEEPFIXED PLAN);
		ELSE IF @intVersion = 2
			WITH A AS (
				SELECT TradeID
					, ROW_NUMBER() OVER (PARTITION BY PremiumFlag ORDER BY ListPrice) AS Seq
				FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNIF_Trade_ItemID), FORCESEEK)
				WHERE ItemID = @intItemID
					AND TradeStatusCode = 0
					AND TradeExpireDate > GETDATE()
					AND CharacterID <> @inbCharacterID
			), B AS (
				SELECT TradeID, Seq
				FROM A
				WHERE Seq <= 25
			)
			SELECT TR.TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, ListPrice, SealCount, PremiumFlag, PayMethodCode, ItemPotentialMoveCount, DragonJewelType, ItemSerial
			FROM B
				INNER LOOP JOIN dbo.Trade TR WITH (NOLOCK, INDEX(PK_Trade), FORCESEEK) ON B.TradeID = TR.TradeID
			ORDER BY PremiumFlag DESC, Seq
			OPTION (KEEPFIXED PLAN);
	END
	ELSE
	BEGIN
		IF @intVersion = 1 	
			WITH A AS (
				SELECT TradeID
					, ROW_NUMBER() OVER (PARTITION BY PremiumFlag ORDER BY ListPrice) AS Seq
				FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNIF_Trade_ItemID), FORCESEEK)
				WHERE ItemID = @intItemID
					AND TradeStatusCode = 0
					AND TradeExpireDate > GETDATE()
					AND CharacterID <> @inbCharacterID
					AND PayMethodCode = @inyPayMethodCode
			), B AS (
				SELECT TradeID, Seq
				FROM A
				WHERE Seq <= 25
			)
			SELECT TR.TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, ListPrice, SealCount, PremiumFlag, PayMethodCode, ItemPotentialMoveCount
			FROM B
				INNER LOOP JOIN dbo.Trade TR WITH (NOLOCK, INDEX(PK_Trade), FORCESEEK) ON B.TradeID = TR.TradeID
			ORDER BY PremiumFlag DESC, Seq
			OPTION (KEEPFIXED PLAN);
		ELSE IF @intVersion = 2
			WITH A AS (
				SELECT TradeID
					, ROW_NUMBER() OVER (PARTITION BY PremiumFlag ORDER BY ListPrice) AS Seq
				FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNIF_Trade_ItemID), FORCESEEK)
				WHERE ItemID = @intItemID
					AND TradeStatusCode = 0
					AND TradeExpireDate > GETDATE()
					AND CharacterID <> @inbCharacterID
					AND PayMethodCode = @inyPayMethodCode
			), B AS (
				SELECT TradeID, Seq
				FROM A
				WHERE Seq <= 25
			)
			SELECT TR.TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, ListPrice, SealCount, PremiumFlag, PayMethodCode, ItemPotentialMoveCount, DragonJewelType, ItemSerial
			FROM B
				INNER LOOP JOIN dbo.Trade TR WITH (NOLOCK, INDEX(PK_Trade), FORCESEEK) ON B.TradeID = TR.TradeID
			ORDER BY PremiumFlag DESC, Seq
			OPTION (KEEPFIXED PLAN);
	END

	/**_## ??? ???? ?? ???? ???, 103171? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103171;
		GOTO ErrorHandler;
	END
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
