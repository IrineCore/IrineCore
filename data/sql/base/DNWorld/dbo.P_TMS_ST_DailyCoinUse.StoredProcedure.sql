USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_ST_DailyCoinUse]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-08-23
description : (TIMOSE) Coin (??) ?? SSIS Snapshot (dbo.ST_DailyCoinUse)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-08-23
**/

CREATE PROCEDURE [dbo].[P_TMS_ST_DailyCoinUse]
	@dtmBasisDate datetime = NULL 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intWorldID int, @inbChangedCoin bigint;
	
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dtmBasisDate = CAST(ISNULL(@dtmBasisDate, GETDATE()) AS date);
	--?? ??
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID OPTION (MAXDOP 1);
	SELECT @inbChangedCoin = SUM(AttachCoin) FROM dbo.Mails WITH (NOLOCK, INDEX(IX_NNF_Mails_AttachCoin)) WHERE DeleteFlag = 0 AND AttachCoinReceiveFlag = 0 AND AttachCoin > 0 OPTION (MAXDOP 1);
	SELECT CAST(DATEADD(DD, -1, @dtmBasisDate) AS date) AS BasisDate, @intWorldID AS WorldID, 226 AS CoinChangeCode, 1 AS CoinTypeCode
		, CASE WHEN @inbChangedCoin >= 0 AND @inbChangedCoin < 10000 THEN 1
				WHEN @inbChangedCoin >= 10000 AND @inbChangedCoin < 100000 THEN 2
				WHEN @inbChangedCoin >= 100000 AND @inbChangedCoin < 1000000 THEN 3
				WHEN @inbChangedCoin >= 1000000 AND @inbChangedCoin < 10000000 THEN 4
				WHEN @inbChangedCoin >= 10000000 AND @inbChangedCoin < 100000000 THEN 5
				WHEN @inbChangedCoin >= 100000000 AND @inbChangedCoin < 1000000000 THEN 6
				WHEN @inbChangedCoin >= 1000000000 AND @inbChangedCoin < 10000000000 THEN 7
				WHEN @inbChangedCoin >= 10000000000 THEN 8 
				ELSE 0 END AS CoinGroupCode
		, @inbChangedCoin AS ChangedCoin
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
