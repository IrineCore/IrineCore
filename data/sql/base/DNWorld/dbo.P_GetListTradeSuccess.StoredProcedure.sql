USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTradeSuccess]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-10-13

description :
	??? ???? ?? ?? (DOLIS)
		- ????? ???? DNStaging DB ?? ??
		- ?? ???? DNWorld DB ?? ??
result set : 

return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	5 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListTradeSuccess]
	  @inbCharacterID bigint = NULL --// ??? ??? ID
	, @inbBuyerCharacterID bigint = NULL --// ??? ??? ID
	, @inyTradeStatusCode tinyint = NULL --// ???? (1=???, 2=??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @dtToDay date = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT T.TradeDate
		 , C1.AccountName AS BuyerAccountName, C1.CharacterID AS BuyerCharacterID, C1.CharacterName AS BuyerCharacterName
		 , C2.AccountName, C2.CharacterID, C2.CharacterName
		 , T.ItemID
		 , N'' AS ItemName
		 , NULL AS ItemRankCode
		 , T.ItemCount
		 , NULL AS LevelLimit
		 , T.ItemLevel
		 , T.SealCount
		 , T.Price
	     , T.TradeStatusCode
	  FROM dbo.Trade T WITH (NOLOCK, INDEX(IX_NN_Trade_TradeDate))
		   INNER JOIN dbo.Characters C1 WITH (NOLOCK) ON (T.BuyerCharacterID = C1.CharacterID)
		   INNER JOIN dbo.Characters C2 WITH (NOLOCK) ON (T.CharacterID = C2.CharacterID)
	 WHERE (T.TradeDate >= CONVERT(datetime2(0), @dtToDay))
	   AND ((@inyTradeStatusCode IS NULL AND T.TradeStatusCode IN (1, 2)) OR (@inyTradeStatusCode IS NOT NULL AND T.TradeStatusCode = @inyTradeStatusCode))
	   AND (@inbCharacterID IS NULL OR T.CharacterID = @inbCharacterID)
	   AND (@inbBuyerCharacterID IS NULL OR T.BuyerCharacterID = @inbBuyerCharacterID)
	 ORDER BY T.TradeDate;

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
