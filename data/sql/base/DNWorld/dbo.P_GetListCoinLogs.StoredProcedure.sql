USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCoinLogs]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-24
description : ??? ?? ??? ?????.
result set : LogDate/datetime, CharacterID/bigint, CoinTypeCode/tinyint, CoinChangeCode/tinyint, CurrentCoin/bigint, ChangedCoin/bigint, ChannelID/int, MapID/int, CoinChangeFKey/bigint, IP/varchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListCoinLogs]
	  @dt2StartLogDate datetime2(0) --// ?? ?? ?? ??
	, @dt2EndLogDate datetime2(0) --// ?? ?? ?? ??	
	, @inbCharacterID bigint --// ??? ID		
	, @vchCoinChangeCodes varchar(max) = NULL --// (optional) ???? ???? (','? ???? ???? ??????.1=??? ??, 2=?? ??, 3=??, 4=??? ??, 5=?? ??, 6=??????, 7=??????, 8=?? ??, 9=?? ??, 10=??? ???, 11=?? ??, 12=?? ??, 13=??? ??, 14=??? ??, 15=?? ???, 16=?? ???, 17=?? ??, 18=?? ??, 19=?? ???, 20=?? ???, 21=?????? ???, 22=???? ??? ??, 23=???? ??? ???, 24=???? ?? ?? ???, 25=?? ???, 26=??? ??, 27=?? ??? ??, 28=NPC?? ??, 29=?? ??, 30=?? ?? ?, 31=?? ?? ??, 32=?? ?? ??, 33=?? ?? ??,34=NPC?? ??, 35=(DOORS) ?????, 36=(DOORS) ?????, 37=(DOORS) ?????, 38=(DOORS) ????, 39=(DOORS) ????, 40=(DOORS) ???????, 41=(DOORS) ???????, 42=(DOORS) ???????, 43=(DOORS) ??????, 44=(DOORS) ??????, 45=????? ?? ?? ??, 46=?????? ?? ?? ??, 47=?????, 48=???? ?? ???, 49=C2C??, 50=C2C??, 51=C2C??, 52=????????, 53=???? ?? ???, 54=????? ??? ?????, 55=?? ???, 56=???????? 57=?????????, 58=?? ??????. 59=????? ??? ???? ???)
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
	IF CHARINDEX(',', @vchCoinChangeCodes) = 0 OR CHARINDEX(',', @vchCoinChangeCodes) IS NULL
		IF dbo.FN_LogTableSuffix(GETDATE()) = 0 			
			SELECT LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, dbo.FN_IPNumber2IP(IP) AS IP
			FROM dbo.CoinLogs0 WITH (NOLOCK)
			WHERE CharacterID = @inbCharacterID
			  AND CoinTypeCode = 1
			  AND CoinChangeCode = COALESCE(CAST (@vchCoinChangeCodes AS tinyint), CoinChangeCode)
		      AND LogDate >= @dt2StartLogDate AND LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
		ELSE IF dbo.FN_LogTableSuffix(GETDATE()) = 1 		
			SELECT LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, dbo.FN_IPNumber2IP(IP) AS IP
			FROM dbo.CoinLogs1 WITH (NOLOCK)
			WHERE CharacterID = @inbCharacterID
			  AND CoinTypeCode = 1
			  AND CoinChangeCode = COALESCE(CAST (@vchCoinChangeCodes AS tinyint), CoinChangeCode)
		      AND LogDate >= @dt2StartLogDate AND LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
		ELSE 
			SELECT LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, dbo.FN_IPNumber2IP(IP) AS IP
			FROM dbo.CoinLogs2 WITH (NOLOCK)
			WHERE CharacterID = @inbCharacterID
			  AND CoinTypeCode = 1
			  AND CoinChangeCode = COALESCE(CAST (@vchCoinChangeCodes AS tinyint), CoinChangeCode)
		      AND LogDate >= @dt2StartLogDate AND LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
	ELSE		
		IF dbo.FN_LogTableSuffix(GETDATE()) = 0 
			WITH LC AS (
				SELECT CAST (string AS tinyint) AS CoinChangeCode
				FROM dbo.TF_Split(@vchCoinChangeCodes,',')
			)
			SELECT LogDate, CharacterID, CoinTypeCode, IL.CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, dbo.FN_IPNumber2IP(IP) AS IP
			FROM dbo.CoinLogs0 IL WITH (NOLOCK) INNER JOIN LC ON IL.CoinChangeCode = LC.CoinChangeCode
			WHERE IL.CharacterID = @inbCharacterID
			  AND IL.CoinTypeCode = 1
		      AND IL.LogDate >= @dt2StartLogDate AND IL.LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
		ELSE IF dbo.FN_LogTableSuffix(GETDATE()) = 1 		
			WITH LC AS (
				SELECT CAST (string AS tinyint) AS CoinChangeCode
				FROM dbo.TF_Split(@vchCoinChangeCodes,',')
			)
			SELECT LogDate, CharacterID, CoinTypeCode, IL.CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, dbo.FN_IPNumber2IP(IP) AS IP
			FROM dbo.CoinLogs1 IL WITH (NOLOCK) INNER JOIN LC ON IL.CoinChangeCode = LC.CoinChangeCode
			WHERE IL.CharacterID = @inbCharacterID
			  AND IL.CoinTypeCode = 1
		      AND IL.LogDate >= @dt2StartLogDate AND IL.LogDate <= @dt2EndLogDate
			OPTION (MAXDOP 1);	
		ELSE 
			WITH LC AS (
				SELECT CAST (string AS tinyint) AS CoinChangeCode
				FROM dbo.TF_Split(@vchCoinChangeCodes,',')
			)
			SELECT LogDate, CharacterID, CoinTypeCode, IL.CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, dbo.FN_IPNumber2IP(IP) AS IP
			FROM dbo.CoinLogs2 IL WITH (NOLOCK) INNER JOIN LC ON IL.CoinChangeCode = LC.CoinChangeCode
			WHERE IL.CharacterID = @inbCharacterID
			  AND IL.CoinTypeCode = 1
		      AND IL.LogDate >= @dt2StartLogDate AND IL.LogDate <= @dt2EndLogDate
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
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
