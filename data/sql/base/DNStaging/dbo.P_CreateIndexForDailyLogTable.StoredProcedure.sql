USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_CreateIndexForDailyLogTable]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 27
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-24
description : ? ??? ?? ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
18~19 = PVPEndLogs ??? ???? ?? ?? by ??? at 2012-03-08
20~21 = ?????, ???? ??? ?? ?? by ??? at 2012-07-27
**/

CREATE PROCEDURE [dbo].[P_CreateIndexForDailyLogTable]
	  @dtmBasisDate datetime --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @datBasisDate date
	, @nvcTableSuffix nchar(8)
	, @nvcStmt nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @datBasisDate  = DATEADD(DAY, DATEDIFF(DAY, 0, @dtmBasisDate), 0);
	SET @nvcTableSuffix = CONVERT(nchar(8), @datBasisDate, 112);

	EXEC dbo.P_CreateIndexForDailyLogTableSub @dtmBasisDate = @dtmBasisDate;

	SET @nvcStmt = N'CREATE INDEX IX_NN_AccountLoginLogs_' + @nvcTableSuffix + N'_AccountID ON dbo.AccountLoginLogs_' + @nvcTableSuffix + N' (AccountID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_AccountLoginLogs_' + @nvcTableSuffix + N'_LoginIP ON dbo.AccountLoginLogs_' + @nvcTableSuffix + N' (LoginIP)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_AccountLoginLogs_' + @nvcTableSuffix + N'_MachineID ON dbo.AccountLoginLogs_' + @nvcTableSuffix + N' (MachineID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_CharacterLogoutLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.CharacterLogoutLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_CharacterLogoutLogs_' + @nvcTableSuffix + N'_LoginIP ON dbo.CharacterLogoutLogs_' + @nvcTableSuffix + N' (LoginIP)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_MapStayLogs_' + @nvcTableSuffix + N'_MapID ON dbo.MapStayLogs_' + @nvcTableSuffix + N' (MapID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_MapStayLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.MapStayLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_AccountLogoutLogs_' + @nvcTableSuffix + N'_AccountID ON dbo.AccountLogoutLogs_' + @nvcTableSuffix + N' (AccountID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_AccountLogoutLogs_' + @nvcTableSuffix + N'_LoginIP ON dbo.AccountLogoutLogs_' + @nvcTableSuffix + N' (LoginIP)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_AccountLogoutLogs_' + @nvcTableSuffix + N'_MachineID ON dbo.AccountLogoutLogs_' + @nvcTableSuffix + N' (MachineID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_FatigueLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.FatigueLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_ItemAttributeLogs_' + @nvcTableSuffix + N'_ItemSerial ON dbo.ItemAttributeLogs_' + @nvcTableSuffix + N' (ItemSerial)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_CharacterLevelLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.CharacterLevelLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_CoinLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.CoinLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_CoinLogs_' + @nvcTableSuffix + N'_CoinTypeCode_CoinChangeCode_CoinChangeFKey ON dbo.CoinLogs_' + @nvcTableSuffix + N' (CoinTypeCode, CoinChangeCode, CoinChangeFKey)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_SkillLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.SkillLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_SkillPointLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.SkillPointLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_ItemLogs_' + @nvcTableSuffix + N'_ItemSerial ON dbo.ItemLogs_' + @nvcTableSuffix + N' (ItemSerial)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_ItemLogs_' + @nvcTableSuffix + N'_ItemID ON dbo.ItemLogs_' + @nvcTableSuffix + N' (ItemID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_ItemLogs_' + @nvcTableSuffix + N'_OwnerCharacterID ON dbo.ItemLogs_' + @nvcTableSuffix + N' (OwnerCharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_ItemLogs_' + @nvcTableSuffix + N'_OwnerCharacterIDBefore ON dbo.ItemLogs_' + @nvcTableSuffix + N' (OwnerCharacterIDBefore)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_ItemLogs_' + @nvcTableSuffix + N'_IP ON dbo.ItemLogs_' + @nvcTableSuffix + N' (IP)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_ItemLogs_' + @nvcTableSuffix + N'_ItemLogCode_ItemLogFKey1 ON dbo.ItemLogs_' + @nvcTableSuffix + N' (ItemLogCode, ItemLogFKey1)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_CashRebirthCoinLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.CashRebirthCoinLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_StageLogs_' + @nvcTableSuffix + N'_StageStartID ON dbo.StageLogs_' + @nvcTableSuffix + N' (StageStartID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_StageLogs_' + @nvcTableSuffix + N'_PartyStartID ON dbo.StageLogs_' + @nvcTableSuffix + N' (PartyStartID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_StageEndLogs_' + @nvcTableSuffix + N'_StageStartID ON dbo.StageEndLogs_' + @nvcTableSuffix + N' (StageStartID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_StageEndLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.StageEndLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_StageEndLogs_' + @nvcTableSuffix + N'_RewardItemSerial ON dbo.StageEndLogs_' + @nvcTableSuffix + N' (RewardItemSerial)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_PVPLogs_' + @nvcTableSuffix + N'_PVPStartID ON dbo.PVPLogs_' + @nvcTableSuffix + N' (PVPStartID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_PVPEndLogs_' + @nvcTableSuffix + N'_PVPStartID ON dbo.PVPEndLogs_' + @nvcTableSuffix + N' (PVPStartID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_PVPEndLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.PVPEndLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_MailLogs_' + @nvcTableSuffix + N'_SenderCharacterID ON dbo.MailLogs_' + @nvcTableSuffix + N' (SenderCharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_MailLogs_' + @nvcTableSuffix + N'_ReceiverCharacterID ON dbo.MailLogs_' + @nvcTableSuffix + N' (ReceiverCharacterID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_MailLogs_' + @nvcTableSuffix + N'_MailID ON dbo.MailLogs_' + @nvcTableSuffix + N' (MailID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_TradeSuccessLogs_' + @nvcTableSuffix + N'_TradeID ON dbo.TradeSuccessLogs_' + @nvcTableSuffix + N' (TradeID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_TradeSuccessLogs_' + @nvcTableSuffix + N'_BuyerCharacterID ON dbo.TradeSuccessLogs_' + @nvcTableSuffix + N' (BuyerCharacterID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_TradeSuccessLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.TradeSuccessLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_TradeRegisterLogs_' + @nvcTableSuffix + N'_TradeID ON dbo.TradeRegisterLogs_' + @nvcTableSuffix + N' (TradeID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_TradeRegisterLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.TradeRegisterLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_JobChangeLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.JobChangeLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_QuestCompleteLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.QuestCompleteLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_MissionAchieveLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.MissionAchieveLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_CharacterLoginLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.CharacterLoginLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_PointLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.PointLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_NestGateClearLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.NestGateClearLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_NestDeathLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.NestDeathLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_RepetitionQuestCompleteLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.RepetitionQuestCompleteLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);
	SET @nvcStmt = N'CREATE INDEX IX_NN_MobileAuthenticationLogs_' + @nvcTableSuffix + N'_AccountID ON dbo.MobileAuthenticationLogs_' + @nvcTableSuffix + N' (AccountID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_PCRoomCharacterLoginLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.PCRoomCharacterLoginLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_PCRoomCharacterLogoutLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.PCRoomCharacterLogoutLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_PCRoomCharacterLogoutLogs_' + @nvcTableSuffix + N'_LoginIP ON dbo.PCRoomCharacterLogoutLogs_' + @nvcTableSuffix + N' (LoginIP)';
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE INDEX IX_NN_HiddenCashShopLogs_' + @nvcTableSuffix + N'_CharacterID ON dbo.HiddenCashShopLogs_' + @nvcTableSuffix + N' (CharacterID)';
	EXEC (@nvcStmt);

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
