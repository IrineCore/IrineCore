USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCoinLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 17
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ?? ??? ?????. (??, ????, ????, PC?????)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
15 = input param. @vchIP ?? at 2010-10-08
16~17 = ?? ?? ?? ?? by ??? at 2012-01-10
**/

CREATE PROCEDURE [dbo].[P_AddCoinLog]
	  @dt2LogDate datetime2(0) --// ?? ??
	, @inbCharacterID bigint --// ??? ID
	, @inyCoinTypeCode tinyint --// 1=??, 2=?? ??, 3=?? ??, 4=PC? ?? ??
	, @inyCoinChangeCode tinyint --// 1=??? ??, 2=?? ??, 3=??, 4=??? ??, 5=?? ??, 6=??????, 7=??????, 8=?? ??, 9=?? ??, 10=??? ???, 11=?? ??, 12=?? ??, 13=??? ??, 14=??? ??, 15=?? ???, 16=?? ???, 17=?? ??, 18=?? ??, 19=?? ???, 20=?? ???, 21=?????? ???, 22=???? ??? ??, 23=???? ??? ???, 24=???? ?? ?? ???, 25=?? ???, 26=??? ??, 27=?? ??? ??, 28=NPC?? ??, 29=?? ??, 30=?? ?? ?, 31=?? ?? ??, 32=?? ?? ??, 33=?? ?? ??, 34=NPC?? ??
	, @inbCurrentCoin bigint --// ?? ?? ??
	, @inbChangedCoin bigint --// ?? ???
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @inbCoinChangeFKey bigint = NULL --// CoinChangeCode? ?? ?. 1:QuestID, 2:PartyID or NULL, 3:NULL, 4:AuditLogID, 5:NULL, 6:MailID, 7:MailID, 8:NULL, 9:NULL, 10:NPCID, 11:NULL, 12:NULL, 13:QuestID, 14:ItemSerial, 15:NULL, 16:ItemEnchantLogID, 17:SkillID, 18:ItemSerial, 19:NULL, 20:MailID, 21:TradeID, 22:TradeID, 23:TradeID, 24:TradeID, 25:NULL
	, @vchIP varchar(15) = NULL --// IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyCoinChangeCode = 0
		RETURN 0;

	SET @inbCoinChangeFKey = CASE @inbCoinChangeFKey WHEN 0 THEN NULL ELSE @inbCoinChangeFKey END;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	/**_# [CoinLogs0] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.CoinLogs0 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, IP)
		VALUES (@dt2LogDate, @inbCharacterID, @inyCoinTypeCode, @inyCoinChangeCode, @inbCurrentCoin, @inbChangedCoin, @intChannelID, @intMapID, @inbCoinChangeFKey, dbo.FN_IP2IPNumber(@vchIP));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.CoinLogs1 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, IP)
		VALUES (@dt2LogDate, @inbCharacterID, @inyCoinTypeCode, @inyCoinChangeCode, @inbCurrentCoin, @inbChangedCoin, @intChannelID, @intMapID, @inbCoinChangeFKey, dbo.FN_IP2IPNumber(@vchIP));

	ELSE
		INSERT dbo.CoinLogs2 (LogDate, CharacterID, CoinTypeCode, CoinChangeCode, CurrentCoin, ChangedCoin, ChannelID, MapID, CoinChangeFKey, IP)
		VALUES (@dt2LogDate, @inbCharacterID, @inyCoinTypeCode, @inyCoinChangeCode, @inbCurrentCoin, @inbChangedCoin, @intChannelID, @intMapID, @inbCoinChangeFKey, dbo.FN_IP2IPNumber(@vchIP));

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
