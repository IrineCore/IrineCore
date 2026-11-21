USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetAuctionInfo]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-26
description : ?? ?? ??? ?????.
result set : ItemID/int, ItemLevel/tinyint, ItemOption/tinyint, ItemPotential/tinyint, ItemCount/smallint, ItemDurability/smallint, SealCount/tinyint, CategoryCode/tinyint, CharacterID/bigint, AuctionStatusCode/tinyint, BiddingEndDate/smalldatetime, BuynowPrice/bigint, StartPrice/bigint, MaxBiddingPrice/bigint, BiddingCount/int, ItemSerial/bigint, CharacterName/nvarchar, CoinCalculateFlag/bit, ItemCalculateFlag/bit, UpdateDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetAuctionInfo]
	  @intAuctionID int  --// ??ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Auctions] ???? SELECT ???.*/
	SELECT A.ItemID, A.ItemLevel, A.ItemOption, A.ItemPotential, A.ItemCount, A.ItemDurability, A.SealCount, A.CategoryCode, A.CharacterID, 
		   CASE WHEN A.AuctionStatusCode = 0 AND A.BiddingEndDate > @dt2Now THEN 0
				WHEN (A.AuctionStatusCode = 2 OR (A.BiddingEndDate <= @dt2Now AND ISNULL(AC.BiddingCount, 0) > 0)) THEN 1
				WHEN A.BiddingEndDate <= @dt2Now AND ISNULL(AC.BiddingCount, 0) = 0 THEN 2
                ELSE 0 END AS AuctionStatusCode, 
		   A.BiddingEndDate, A.BuynowPrice, A.StartPrice, AC.MaxBiddingPrice, AC.BiddingCount, A.ItemSerial, CH.CharacterName, A.CoinCalculateFlag, A.ItemCalculateFlag, A.UpdateDate
	FROM dbo.Auctions A WITH (NOLOCK) LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount AC ON A.AuctionID = AC.AuctionID
	     INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON A.CharacterID = CH.CharacterID
	WHERE A.AuctionID = @intAuctionID;
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
