USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetMyAuctionCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-22
description : ?? ?? ??? ???? ?? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetMyAuctionCount]
	  @inbCharacterID bigint --// ???ID
	, @inyAuctionStatusCode tinyint = NULL --// ?? ?? ?? (NULL=??, 0=???, 1=????, 2=??)
	, @intAuctionCount int = NULL OUTPUT --// ??ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Auctions] ???? SELECT???.*/
	IF @inyAuctionStatusCode = 1			
		SELECT @intAuctionCount = COUNT(*)
		FROM dbo.Auctions A WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)
			 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount AC WITH (NOLOCK) ON A.AuctionID = AC.AuctionID
		WHERE CharacterID = @inbCharacterID AND (AuctionStatusCode = 2 OR (BiddingEndDate <= @dt2Now AND ISNULL(AC.BiddingCount, 0) > 0));
	ELSE IF @inyAuctionStatusCode = 2
		SELECT @intAuctionCount = COUNT(*)
		FROM dbo.Auctions A WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)	
			 LEFT OUTER JOIN dbo.VIX_AuctionBiddingCount AC WITH (NOLOCK) ON A.AuctionID = AC.AuctionID
		WHERE A.CharacterID = @inbCharacterID AND A.BiddingEndDate <= @dt2Now AND ISNULL(AC.BiddingCount, 0) = 0 AND A.AuctionStatusCode <> 4;			  
	ELSE IF @inyAuctionStatusCode = 0
		SELECT @intAuctionCount = COUNT(*)
		FROM dbo.Auctions WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)		
		WHERE CharacterID = @inbCharacterID AND AuctionStatusCode = 0 AND BiddingEndDate > @dt2Now AND AuctionStatusCode <> 4;
	ELSE
		SELECT @intAuctionCount = COUNT(*)
		FROM dbo.Auctions WITH (NOLOCK, INDEX(IX_NN_Auctions_CharacterID_AuctionStatusCode), FORCESEEK)		
		WHERE CharacterID = @inbCharacterID AND AuctionStatusCode <> 4;
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
