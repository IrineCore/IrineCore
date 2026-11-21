USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListTopAuctionBidder]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-14
description : ?? ???? ?????.
result set : CharacterID/bigint, CharacterName/nvarchar(30), BiddingDate/smalldatetimw, BiddingPrice/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetListTopAuctionBidder]
	  @intAuctionID int --// ??ID	
	, @inyListCount tinyint = 5 --// ??? ??? ???	
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
	/**_# [AuctionBidding] ???? SELECT???.*/
	;WITH A AS (
		SELECT ROW_NUMBER() OVER(ORDER BY BiddingDate DESC) AS RowNumber, AB.CharacterID, CH.CharacterName, AB.BiddingDate, AB.BiddingPrice
		FROM dbo.AuctionBidding AB WITH (NOLOCK, INDEX(PK_AuctionBidding)) 
			 INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
			 ON AB.CharacterID = CH.CharacterID
		WHERE AB.AuctionID = @intAuctionID
	)
	SELECT CharacterID, CharacterName, BiddingDate, BiddingPrice
	FROM A
	WHERE RowNumber <= @inyListCount;
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
