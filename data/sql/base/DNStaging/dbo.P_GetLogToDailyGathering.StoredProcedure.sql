USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetLogToDailyGathering]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 26
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2012-03-30
description : Daily Data Gathering ETL ?? Staging DB? Log ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ?? ?? by ??? at 2012-03-30
4 = PurchaseOrders..OrderDate? ?????? Char(8)->Datetime2? ?? by ??? at 2012-04-26
5 = MAXDOP ?? ?? by ??? at 2012-04-26
8 = ??? ??? ?? ?? by ??? at 2012-05-16
9 = SQL ??? ???? ?? by ??? at 2012-05-22
10 = ??? MapID ?? ?? by ??? at 2012-05-29
11 = ?? ?? Bitmap ?? ?? by ??? at 2012-05-30
12 = ??? ?? ?? ?? ?? by ??? at 2012-05-31
13 = ??? ?? ?? ??? ?? ?? ?? by ??? at 2012-06-01
14 = ??? ??? ?? by ??? at 2012-07-13
15 = PVPEndLogs ?? by ??? at 2012-07-24
16 = MapStayLogs, DimAccount, PlayTimeLogs ?? by ??? at 2012-08-06
17 = PVPLogs ?? by ??? at 2012-08-10
19 = DailyPVPLadderScores ? JobCode ?? by ??? at 2012-11-08
20 = DailyPVPLadderScores ? TodayWin, TodayLose, TodayDraw by ??? at 2012-11-08
21 = NestDeathLogs, NestGateClearLogs ?? by ??? at 2012-12-10
22 = ? ?????? ?? by ??? at 2013-05-13
23 = StageEndLogs ?? ? ??? ???? ???? ?? by ??? at 2013-06-18
24 = CharacterPlayTime ?? by ??? at 2013-06-19
25 = CharacterPlayTime..BasisDate ?? ?? by ??? at 2013-06-28
26 = StageEndLogs ?? ? ??? ??? ?? ?? ?? ?? by ??? at 2013-07-03
**/

CREATE PROCEDURE [dbo].[P_GetLogToDailyGathering]
	  @dtmBasisDate datetime --// ?? ??
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int
	, @nvcStmt nvarchar(max) = N''
	, @nvcYesterday nvarchar(8) = CONVERT(NVARCHAR(8), DATEADD(DD, -1, @dtmBasisDate), 112);

DECLARE @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'StageEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DD, -30, CAST(@dtmBasisDate AS DATE)), 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	

	--??? ??
	SET @nvcStmt = CAST(N'' AS nvarchar(max)) + N'
		--?? ?? ??
		DECLARE @inyMaxLevel tinyint;
		SELECT @inyMaxLevel = MAX(CharacterLevel) FROM dbo.DimCharacters WITH (NOLOCK) WHERE AccountLevelCode = 0;

		--DNStaging..DimItems
		SELECT *
		INTO DNGathering.dbo.DimItems
		FROM dbo.DimItems WITH (NOLOCK);
		
		--DNStaging..StageEndLogs
		SELECT StageStartDate, StageStartID, CharacterID, JobCode, CharacterLevel, CAST(ClearFlag AS tinyint) AS [ClearFlag]
			 , ClearTime, PartyStartID, PartySize, MapID, DifficultyCode, WorldID, ClearRound
		INTO DNGathering.dbo.StageEndLogs_' + @nvcYesterday + N'
		FROM DNStaging.dbo.StageEndLogs_' + @nvcYesterday + N' WITH (NOLOCK) 
		WHERE CharacterLevel = CASE WHEN MapID IN (14071, 14077, 14081, 14090, 14200, 14217) THEN CharacterLevel ELSE @inyMaxLevel END;
		
		--DNStaging..CharacterLogoutLogs
		SELECT cl.CharacterID, dc.AccountID, dc.CharacterLevel, dc.JobCode, cl.LoginDate, cl.LogoutDate
		INTO DNGathering.dbo.CharacterLogoutLogs_' + @nvcYesterday + N'
		FROM dbo.CharacterLogoutLogs_' + @nvcYesterday + N' AS cl WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters AS dc WITH (NOLOCK) ON dc.CharacterID = cl.CharacterID
		WHERE dc.AccountLevelCode = 0;
		
		--DNStaging..TradeSuccessLogs
		SELECT t.TradeDate, t.TradeID, t.BuyerCharacterID, t.ItemID, t.ItemSerial, t.ItemLevel, t.SealCount
			 , t.ItemDurability, t.ItemCount, t.Price, t.WorldID, t.CharacterID, t.PayMethodCode, t.TradeStatusCode
			 , t.RandomSeed, i.ItemOption, i.ItemPotential
		INTO DNGathering.dbo.TradeSuccessLogs_' + @nvcYesterday + N'
		FROM dbo.TradeSuccessLogs_' + @nvcYesterday + N' AS t WITH (NOLOCK)
		LEFT OUTER JOIN dbo.ItemLogs_' + @nvcYesterday + N' AS i WITH (NOLOCK) ON i.ItemLogFKey1 = t.TradeID
		WHERE i.ItemLogCode = 28;

		--DNStaging..DimCharacters		
		SELECT AccountID, CharacterID, JobCode, Coin, WorldID, CAST(DeleteFlag AS TINYINT) AS [DeleteFlag], CharacterName, CharacterLevel
			, 0
			| ISNULL((SELECT TOP 1 POWER(2,0) FROM dbo.PurchaseOrders WHERE CharacterID = dc.CharacterID), 0)
			| CASE WHEN LastLoginDate >= DATEADD(DD, -7, CAST(GETDATE() AS DATE)) THEN POWER(2,1) ELSE 0 END
			| CASE WHEN LastLoginDate >= DATEADD(DD, -30, CAST(GETDATE() AS DATE)) THEN POWER(2,2) ELSE 0 END
			| CASE WHEN LastLoginDate >= DATEADD(DD, -90, CAST(GETDATE() AS DATE)) THEN POWER(2,3) ELSE 0 END
			| ISNULL((SELECT SUM(CASE WHEN MapID = 14085 THEN POWER(2, 4) --?????? ??? (CG_MinoNest)
									WHEN MapID = 14002 THEN POWER(2, 5) --???? ??? (SH_BossStage1)
									WHEN MapID = 14003 THEN POWER(2, 6) --???? ???(???) (SH_BossStage1Hard)
									WHEN MapID = 14011 THEN POWER(2, 7) --???? ??? (SH_MantiNest)
									WHEN MapID = 14021 THEN POWER(2, 8) --???? ???(???) (SH_MantiNest_Hardcore)
									WHEN MapID = 14032 THEN POWER(2, 9) --????? ??? (SH_ApocalypseNest_BossA)
									WHEN MapID = 14034 THEN POWER(2, 10) --????? ???(???) (SH_ApocalypseNest_BossA_Hell)
									WHEN MapID = 14042 THEN POWER(2, 11) --???? ??? (LM_BishopNestB)
									WHEN MapID = 14052 THEN POWER(2, 12) --???? ???(???) (LM_BishopNestB_Hell)
									WHEN MapID = 14096 THEN POWER(2, 13) --???? ??? (LM_GigantesNestA)
									WHEN MapID = 14097 THEN POWER(2, 14) --???? ???(???) (LM_GigantesNestA_Hell)
									WHEN MapID = 14073 THEN POWER(2, 15) --???? ??? (SH_SeaDragonNest_BossA)
									WHEN MapID = 14079 THEN POWER(2, 16) --???? ???(????) (Hell_SH_SeaDragonNest_BossA)
									WHEN MapID = 14083 THEN POWER(2, 17) --????? ??? (LM_G_DragonNest_BossA)
									WHEN MapID = 14092 THEN POWER(2, 18) --????? ???(????) (Hell_LM_G_DragonNest_BossA)
							ELSE 0 END)
			FROM (' + @nvcStmtOUT + N') V 
			WHERE V.CharacterID = dc.CharacterID AND V.StageStartDate >= DATEADD(DD, -30, CAST(GETDATE() AS DATE))
			AND V.MapID IN (14085 --?????? ??? (CG_MinoNest)
						, 14002 --???? ??? (SH_BossStage1)
						, 14003 --???? ???(???) (SH_BossStage1Hard)
						, 14011 --???? ??? (SH_MantiNest)
						, 14021 --???? ???(???) (SH_MantiNest_Hardcore)
						, 14032 --????? ??? (SH_ApocalypseNest_BossA)
						, 14034 --????? ???(???) (SH_ApocalypseNest_BossA_Hell)
						, 14042 --???? ??? (LM_BishopNestB)
						, 14052 --???? ???(???) (LM_BishopNestB_Hell)
						, 14096 --???? ??? (LM_GigantesNestA)
						, 14097 --???? ???(???) (LM_GigantesNestA_Hell)
						, 14073 --???? ??? (SH_SeaDragonNest_BossA)
						, 14079 --???? ???(????) (Hell_SH_SeaDragonNest_BossA)
						, 14083 --????? ??? (LM_G_DragonNest_BossA)
						, 14092 --????? ???(????) (Hell_LM_G_DragonNest_BossA)
				) AND ClearFlag = 1), 0) AS CharacterGroupBitmap
		INTO DNGathering.dbo.DimCharacters_' + CONVERT(NVARCHAR(8), @dtmBasisDate, 112) + N'
		FROM dbo.DimCharacters AS dc WITH (NOLOCK)
		WHERE CharacterLevel = @inyMaxLevel AND AccountLevelCode = 0
		AND (DeleteDate IS NULL OR (DeleteFlag = 1 AND DeleteDate >= DATEADD(DAY, -30, CAST(CAST(GETDATE() AS date) AS datetime2(0)))));
				
		--DNStaging..PurchaseOrders
		SELECT a.OrderDate, a.CharacterID, a.PayMethodCode, b.ProductID, b.ListPrice, c.AccountID
		INTO DNGathering.dbo.PurchaseOrders_' + @nvcYesterday + N'
		FROM dbo.PurchaseOrders AS a WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters c WITH (NOLOCK) ON (c.CharacterID = a.CharacterID)
		INNER JOIN dbo.PurchaseOrderDetails b WITH (NOLOCK) ON (b.PurchaseOrderID = a.PurchaseOrderID)
		WHERE a.OrderDate >= ''' + CONVERT(NVARCHAR(20), DATEADD(DD, -1, @dtmBasisDate), 120) + N''' AND a.OrderDate < ''' + CONVERT(NVARCHAR(20), @dtmBasisDate, 120) + N'''
		  AND a.OrderStatusCode = 2
		  AND a.PayMethodCode IN (2, 3, 4)
		  AND c.AccountLevelCode = 0;

		--DNStaging..ItemLogs
		SELECT LogDate, ItemSerial, ItemID, OwnerCharacterIDBefore AS OwnerCharacterID, ItemLogFKey1, WorldID, ItemLevel, ItemPotential, SealCount, ItemOption, ItemLevelBefore
			, CASE WHEN ItemLevelBefore >= ItemLevel THEN 2 WHEN OwnerCharacterID IS NULL THEN 3 ELSE 1 END AS [ItemEnchantCode] --1:??,2:??,3:??
		INTO DNGathering.dbo.ItemEnchantLogs_' + @nvcYesterday + N'
		FROM dbo.ItemLogs_' + @nvcYesterday + N' WITH (NOLOCK) WHERE ItemLogCode = 12
		  		
		--DNStaging..PVPEndLogs
		SELECT p.PVPStartDate, p.PVPEndDate, p.PVPStartID, p.CharacterID, p.TeamCode, p.BreakIntoFlag, p.PVPResultCode, p.PVPPlayTime, p.PVPKillPoint, p.PVPAssistPoint, p.PVPSupportPoint
			, p.WorldID, p.OccupationCount, p.OccupationTimeLimitWinFlag
			, dc.JobCode, dc.CharacterLevel, dc.AccountID
		INTO DNGathering.dbo.PVPEndLogs_' + @nvcYesterday + N'
		FROM dbo.PVPEndLogs_' + @nvcYesterday + N' AS p WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters AS dc WITH (NOLOCK) ON dc.CharacterID = p.CharacterID
		WHERE dc.AccountLevelCode = 0;

		--DNStaging..MapStayLogs
		SELECT m.ExitDate, m.MapID, m.CharacterID, m.EntranceDate, dc.AccountID
		INTO DNGathering.dbo.MapStayLogs_' + @nvcYesterday + N'
		FROM dbo.MapStayLogs_' + @nvcYesterday + N' AS m WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters AS dc WITH (NOLOCK) ON dc.CharacterID = m.CharacterID
		WHERE dc.AccountLevelCode = 0
		  AND dc.CharacterLevel = @inyMaxLevel;

		--DNStaging..DimAccounts
		SELECT AccountID, RegisterDate
		INTO DNGathering.dbo.DimAccounts
		FROM dbo.DimAccounts WITH (NOLOCK);

		--DNStaging..PlayTimeLogs
		SELECT dc.AccountID, SUM(DATEDIFF(mi, m.EntranceDate, m.ExitDate)) AS PlayTimeMin
		INTO DNGathering.dbo.PlayTimeLogs_' + @nvcYesterday + N'
		FROM dbo.MapStayLogs_' + @nvcYesterday + N' AS m WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters AS dc WITH (NOLOCK) ON dc.CharacterID = m.CharacterID
		WHERE dc.AccountLevelCode = 0
		GROUP BY dc.AccountID;

		--DNStaging..CharacterPlayTime
		SELECT CAST(m.ExitDate AS date) AS BasisDate, dc.AccountID, m.CharacterID
			, SUM(DATEDIFF(MI, m.EntranceDate, m.ExitDate)) AS PlayTime, dc.CharacterLevel
		INTO DNGathering.dbo.CharacterPlayTime_' + @nvcYesterday + N'
		FROM dbo.MapStayLogs_' + @nvcYesterday + N' m WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters dc WITH (NOLOCK) ON dc.CharacterID = m.CharacterID
		WHERE dc.AccountLevelCode = 0
		GROUP BY CAST(m.ExitDate AS date), dc.AccountID, m.CharacterID, dc.CharacterLevel;

		--DNStaging..PVPLogs
		SELECT PVPStartDate, PVPEndDate, PVPStartID, PVPModeCode, PVPModeDetailCode, BreakIntoFlag, ItemUseFlag, SecreteFlag, MapID, WorldID, RoomOpenCharacterID
		INTO DNGathering.dbo.PVPLogs_' + @nvcYesterday + N'
		FROM dbo.PVPLogs_' + @nvcYesterday + N' WITH (NOLOCK);
		
		--DNStaging..DailyPVPLadderScores
		SELECT a.WorldID, a.BasisDate, a.CharacterID, a.PVPLadderCode, a.PVPLadderGradePoint, a.HiddenPVPLadderGradePoint, a.Win, a.Lose, a.Draw, c.AccountID, a.TodayWin, a.TodayLose, a.TodayDraw, c.JobCode
		INTO DNGathering.dbo.DailyPVPLadderScores_' + @nvcYesterday + N'
		FROM dbo.DailyPVPLadderScores AS a WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters c WITH (NOLOCK) ON (c.CharacterID = a.CharacterID)
		WHERE a.BasisDate >= ''' + CONVERT(NVARCHAR(20), DATEADD(DD, -1, @dtmBasisDate), 120) + N''' AND a.BasisDate < ''' + CONVERT(NVARCHAR(20), @dtmBasisDate, 120) + N'''
		  AND c.AccountLevelCode = 0;

		--DNStaging..DailyPVPLadderScoresByJob
		SELECT a.WorldID, a.BasisDate, a.CharacterID, a.JobCode, a.Win, a.Lose, a.Draw, c.AccountID
		INTO DNGathering.dbo.DailyPVPLadderScoresByJob_' + @nvcYesterday + N'
		FROM dbo.DailyPVPLadderScoresByJob AS a WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters c WITH (NOLOCK) ON (c.CharacterID = a.CharacterID)
		WHERE a.BasisDate >= ''' + CONVERT(NVARCHAR(20), DATEADD(DD, -1, @dtmBasisDate), 120) + N''' AND a.BasisDate < ''' + CONVERT(NVARCHAR(20), @dtmBasisDate, 120) + N'''
		  AND c.AccountLevelCode = 0;

		--DNStaging..DailyPVPLadderKillDeathByJob
		SELECT a.WorldID, a.BasisDate, a.CharacterID, a.JobCode, a.KOWin, a.KOLose, c.AccountID
		INTO DNGathering.dbo.DailyPVPLadderKillDeathByJob_' + @nvcYesterday + N'
		FROM dbo.DailyPVPLadderKillDeathByJob AS a WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters c WITH (NOLOCK) ON (c.CharacterID = a.CharacterID)
		WHERE a.BasisDate >= ''' + CONVERT(NVARCHAR(20), DATEADD(DD, -1, @dtmBasisDate), 120) + N''' AND a.BasisDate < ''' + CONVERT(NVARCHAR(20), @dtmBasisDate, 120) + N'''
		  AND c.AccountLevelCode = 0;		
		  
		--DNStaging..NestDeathLogs
		SELECT n.LogDate, n.CharacterID, n.CharacterLevel, n.JobCode, n.MonsterSkillCode, n.MonsterCode, n.StageStartID, n.MapID, n.WorldID, n.PartySize, dc.AccountID
		INTO DNGathering.dbo.NestDeathLogs_' + @nvcYesterday + N'
		FROM dbo.NestDeathLogs_' + @nvcYesterday + N' AS n WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters AS dc WITH (NOLOCK) ON dc.CharacterID = n.CharacterID
		WHERE dc.AccountLevelCode = 0;  
		
		--DNStaging..NestGateClearLogs
		SELECT n.NestGateStartDate, n.NestGateEndDate, n.NestGateStartID, n.StageStartID, n.CharacterID, n.JobCode, n.CharacterLevel, n.ClearFlag, n.NestRound, n.StartPartySize, n.EndPartySize, n.MapID, n.WorldID, dc.AccountID
		INTO DNGathering.dbo.NestGateClearLogs_' + @nvcYesterday + N'
		FROM dbo.NestGateClearLogs_' + @nvcYesterday + N' AS n WITH (NOLOCK)
		INNER JOIN dbo.DimCharacters AS dc WITH (NOLOCK) ON dc.CharacterID = n.CharacterID
		WHERE dc.AccountLevelCode = 0;  
		';

	EXEC sp_executesql @nvcStmt;

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
