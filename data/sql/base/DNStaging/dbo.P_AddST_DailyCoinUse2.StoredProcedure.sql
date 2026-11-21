USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyCoinUse2]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-08-23
description : ?? ?? ??? (???, ??) ? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ???? by ??? at 2013-08-23
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyCoinUse2]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
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
	SET @datBasisDate = CAST(ISNULL(@datBasisDate, GETDATE()) AS date);

	--??? ?? ??	
	IF OBJECT_ID('tempdb..#c') IS NOT NULL DROP TABLE #c;
	SELECT WorldID, CharacterLevel, JobCode
		, SUM(Coin) AS Coin
		, SUM(WarehouseCoin) AS WarehouseCoin
		, SUM(CAST(RebirthCoin AS bigint)) AS RebirthCoin
		, SUM(CAST(PCRoomRebirthCoin AS bigint)) AS PCRoomRebirthCoin
	INTO #c
	FROM dbo.DimCharacters WITH (NOLOCK)
	WHERE DeleteFlag = 0 AND AccountLevelCode = 0
	GROUP BY WorldID, CharacterLevel, JobCode
	OPTION (MAXDOP 1);
	;WITH c AS (
		SELECT WorldID, CharacterLevel, JobCode, ChangedCoin
			, CASE WHEN CoinTypeCode = 'Coin' THEN 1 WHEN CoinTypeCode = 'WarehouseCoin' THEN 2 WHEN CoinTypeCode = 'RebirthCoin' THEN 3 WHEN CoinTypeCode = 'PCRoomRebirthCoin' THEN 4 END AS CoinTypeCode
		FROM (SELECT WorldID, CharacterLevel, JobCode, Coin, WarehouseCoin, RebirthCoin, PCRoomRebirthCoin FROM #c) a
		UNPIVOT (ChangedCoin FOR CoinTypeCode IN (Coin, WarehouseCoin, RebirthCoin, PCRoomRebirthCoin)) AS b
	)
	INSERT dbo.ST_DailyCoinUse (BasisDate, WorldID, CharacterLevel, JobCode, CoinChangeCode, CoinTypeCode, CoinGroupCode, ChangedCoin)
	SELECT CAST(DATEADD(DD, -1, @datBasisDate) AS date) AS BasisDate, WorldID, CharacterLevel, JobCode, 224 AS CoinChangeCode, CoinTypeCode
		, CASE WHEN ChangedCoin >= 0 AND ChangedCoin < 10000 THEN 1
				WHEN ChangedCoin >= 10000 AND ChangedCoin < 100000 THEN 2
				WHEN ChangedCoin >= 100000 AND ChangedCoin < 1000000 THEN 3
				WHEN ChangedCoin >= 1000000 AND ChangedCoin < 10000000 THEN 4
				WHEN ChangedCoin >= 10000000 AND ChangedCoin < 100000000 THEN 5
				WHEN ChangedCoin >= 100000000 AND ChangedCoin < 1000000000 THEN 6
				WHEN ChangedCoin >= 1000000000 AND ChangedCoin < 10000000000 THEN 7
				WHEN ChangedCoin >= 10000000000 THEN 8 
				ELSE 0 END AS CoinGroupCode
		, ChangedCoin
	FROM c
	OPTION (MAXDOP 1);
	
	--?? ?? ??
	;WITH gc AS (SELECT WorldID, SUM(GuildCoin) AS ChangedCoin FROM dbo.SS_Guilds WITH (NOLOCK) WHERE CloseFlag = 0 GROUP BY WorldID)
	INSERT dbo.ST_DailyCoinUse (BasisDate, WorldID, CoinChangeCode, CoinTypeCode, CoinGroupCode, ChangedCoin)
	SELECT CAST(DATEADD(DD, -1, @datBasisDate) AS date) AS BasisDate, WorldID, 225 AS CoinChangeCode, 1 AS CoinTypeCode
		, CASE WHEN ChangedCoin >= 0 AND ChangedCoin < 10000 THEN 1
				WHEN ChangedCoin >= 10000 AND ChangedCoin < 100000 THEN 2
				WHEN ChangedCoin >= 100000 AND ChangedCoin < 1000000 THEN 3
				WHEN ChangedCoin >= 1000000 AND ChangedCoin < 10000000 THEN 4
				WHEN ChangedCoin >= 10000000 AND ChangedCoin < 100000000 THEN 5
				WHEN ChangedCoin >= 100000000 AND ChangedCoin < 1000000000 THEN 6
				WHEN ChangedCoin >= 1000000000 AND ChangedCoin < 10000000000 THEN 7
				WHEN ChangedCoin >= 10000000000 THEN 8 
				ELSE 0 END AS CoinGroupCode
		, ChangedCoin
	FROM gc
	OPTION (MAXDOP 1);
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
