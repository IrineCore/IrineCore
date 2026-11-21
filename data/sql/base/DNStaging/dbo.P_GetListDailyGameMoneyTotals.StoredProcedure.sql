USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDailyGameMoneyTotals]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-22
description : ?? ?? ?? ??? ?????.
result set : WorldID, BasisDate, Stage, Quest, QuestCnt, MissionMailAttach, MissionMailAttachCnt, ResellItem, ResellItemCnt, AdminMailAttach, AdminMailAttachCnt, EventMailAttach, EventMailAttachCnt, GeneralUse, GeneralUseCnt, ItemRepair, ItemRepairCnt, BuyItem, BuyItemCnt, ExchangeFee, ExchangeFeeCnt, EnchantFee, EnchantFeeCnt, SkillUnlock, SkillUnlockCnt, DisassembleFee, DisassembleFeeCnt, MailFee, MailFeeCnt, TradeRegisterFee, TradeRegisterFeeCnt, TradeFee, TradeFeeCnt, MakeItemFee, MakeItemFeeCnt, DeleteCharacter, DeleteCharacterCnt, Cheat, CheatCnt, AdminAdjust, AdminAdjustCnt, Donate, DonateCnt
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = ???? ?? by ??? at 2012-05-31
**/

CREATE PROCEDURE [dbo].[P_GetListDailyGameMoneyTotals]
	  @datStartDate date --// ?? ?? ??
	, @datEndDate date --// ?? ? ??
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
	SELECT WorldID, BasisDate
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 2 THEN GameMoneyAmount END), 0) AS Stage
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 1 THEN GameMoneyAmount END), 0) AS Quest
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 1 THEN GameMoneyChangeCount END), 0) AS QuestCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 80 THEN GameMoneyAmount END), 0) AS MissionMailAttach
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 80 THEN GameMoneyChangeCount END), 0) AS MissionMailAttachCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 10 THEN GameMoneyAmount END), 0) AS ResellItem
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 10 THEN GameMoneyChangeCount END), 0) AS ResellItemCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 81 THEN GameMoneyAmount END), 0) AS AdminMailAttach
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 81 THEN GameMoneyChangeCount END), 0) AS AdminMailAttachCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 82 THEN GameMoneyAmount END), 0) AS EventMailAttach
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 82 THEN GameMoneyChangeCount END), 0) AS EventMailAttachCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 5 THEN GameMoneyAmount END), 0) AS GeneralUse
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 5 THEN GameMoneyChangeCount END), 0) AS GeneralUseCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 14 THEN GameMoneyAmount END), 0) AS ItemRepair
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 14 THEN GameMoneyChangeCount END), 0) AS ItemRepairCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 18 THEN GameMoneyAmount END), 0) AS BuyItem
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 18 THEN GameMoneyChangeCount END), 0) AS BuyItemCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 15 THEN GameMoneyAmount END), 0) AS ExchangeFee
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 15 THEN GameMoneyChangeCount END), 0) AS ExchangeFeeCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 16 THEN GameMoneyAmount END), 0) AS EnchantFee
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 16 THEN GameMoneyChangeCount END), 0) AS EnchantFeeCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 17 THEN GameMoneyAmount END), 0) AS SkillUnlock
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 17 THEN GameMoneyChangeCount END), 0) AS SkillUnlockCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 19 THEN GameMoneyAmount END), 0) AS DisassembleFee
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 19 THEN GameMoneyChangeCount END), 0) AS DisassembleFeeCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 20 THEN GameMoneyAmount END), 0) AS MailFee
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 20 THEN GameMoneyChangeCount END), 0) AS MailFeeCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 21 THEN GameMoneyAmount END), 0) AS TradeRegisterFee
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 21 THEN GameMoneyChangeCount END), 0) AS TradeRegisterFeeCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 24 THEN GameMoneyAmount END), 0) AS TradeFee
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 24 THEN GameMoneyChangeCount END), 0) AS TradeFeeCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 25 THEN GameMoneyAmount END), 0) AS MakeItemFee
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 25 THEN GameMoneyChangeCount END), 0) AS MakeItemFeeCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 26 THEN GameMoneyAmount END), 0) AS DeleteCharacter
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 26 THEN GameMoneyChangeCount END), 0) AS DeleteCharacterCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 3 THEN GameMoneyAmount END), 0) AS Cheat
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 3 THEN GameMoneyChangeCount END), 0) AS CheatCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 4 THEN GameMoneyAmount END), 0) AS AdminAdjust
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 4 THEN GameMoneyChangeCount END), 0) AS AdminAdjustCnt
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 34 THEN GameMoneyAmount END), 0) AS Donate
		, ISNULL(SUM(CASE GameMoneyChangeCode WHEN 34 THEN GameMoneyChangeCount END), 0) AS DonateCnt
	FROM dbo.ST_DailyGameMoney
	WHERE BasisDate BETWEEN @datStartDate AND @datEndDate
	GROUP BY WorldID, BasisDate
	ORDER BY WorldID, BasisDate;
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
