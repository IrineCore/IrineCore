USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetAuctionItemCount]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-25
description : ???? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetAuctionItemCount]
	  @dt2AuctionCountStartDate datetime2(0) --// ?? ?? ?? ?? ?? ????
	, @dt2AuctionCountEndDate datetime2(0) --// ?? ?? ?? ?? ?? ????
	, @intItemID int --// ??? ID
	, @intItemCount int = NULL OUTPUT --// ??? ??
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
	/**_# [Auctions] ???? SELECT ???.*/
	SELECT @intItemCount = COUNT(*)
	FROM dbo.Auctions WITH (NOLOCK)
	WHERE RegistrationDate >= @dt2AuctionCountStartDate AND RegistrationDate <= @dt2AuctionCountEndDate AND ItemID = @intItemID;

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
