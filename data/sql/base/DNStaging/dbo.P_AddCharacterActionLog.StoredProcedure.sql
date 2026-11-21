USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCharacterActionLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 86
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-20
description : ?? ??? ???? ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
hisgory :
12 = CharacterActionLogCode 111 ~ 113 ?? at 2010-05-19
13 = CharacterActionLogCode 114 ~ 115 ?? at 2010-07-06
14 = CharacterActionLogCode 116 ~ 117 ?? at 2010-07-06
16 = CharacterActionLogCode 118 ?? at 2010-07-16
17 = CharacterActionLogCode 119 ?? at 2010-07-16
18 = CharacterActionLogCode 120 ~ 121 ?? at 2010-07-16
19 = ??? ???? ?? ?? at 2010-07-22
20 = CharacterActionLogCode 122 ~ 123 ?? at 2010-10-06
23 = CharacterActionLogCode 124 ~ 128 ?? at 2010-10-13
24 = CharacterActionLogCode 129 ~ 131 ?? at 2010-12-23
26 = ???? ??? ? ?? ?? ?? ?? (RewardFailReason) ?? at 2010-12-23
27 = RewardFailReason ?? string?? ???? ????? ?? at 2010-12-23
28 = ???? ?? ??? ??/?? ??? ??? ???? ?? ?? at 2011-01-05
29 = RewardFailReason ?? 400, 401, 405? ??? NULL? ????? ?? at 2011-01-08
30 = version 28 ??? ?? at 2011-01-08
31 = CharacterActionLogCode 132 ~ 133 ?? at 2011-02-10
32 = CharacterActionLogCode 134 ?? at 2011-03-03
33 = CharacterActionLogCode 135 ?? at 2011-03-22
34 = TradeRegisterLogs / TradeSuccessLogs ???? PayMethodCode ??     by 20110405.wgkim
35 = ItemLogCode = 53 ? ?? WHERE ? ??(??)     by 20110405.wgkim
39 = ?? ??? ?? (????)     by 20110520.wgkim
40 = ClearRound ??	by 20111011.ohtack
41 = ETL ?? ??	by 20111014.ohtack
42 = CharacterActionLogCode 136~142 ??     by ??? at 2012-01-25
43 = CharacterActionLogCode = 104 (??)? ?? LogFKey3 ??? ??? ItemID (?? ?? ??) ? ????? ??     by ??? at 2012-01-30
44~46 = CharacterActionLogCode = 143 ???? ?? ?? ?? ??  by ??? at 2012-02-08
47~48 = ????? VIEW ???? ???? ?? by ??? at 2012-02-14
49 = CharacterActionLogCode = 99 ? ??? Amount3 ??? RandomSeed? ????? ?? by ??? at 2012-02-15
51~52 = DarkLairLogs Logdate ??? ?? ?? by ??? at 2012-02-28
53~56 = PVP ?? ?? ?? by ??? at 2012-02-29
58 = PVP?? ?? ??? ??? ??? ?? ?? by ??? at 2012-03-13
59 = PVP?? ?? ??? ??? ??? ?? ?? by ??? at 2012-03-14
61 = ????, ????? Code ?? by ??? at 2012-03-27
62 = CoinLogs ?? ?? by ??? at 2012-03-30
63 = ??? ?? ???? CharacterActionLogCode = 148, 149 ?? by ??? at 2012-04-19
64 = ????? ?? CharacterActionLogCode = 150 ?? by ??? at 2012-05-30
64 = ?? ?? ?? ?? CharacterActionLogCode = 151 ?? by ??? at 2012-07-30
68 = ?????, ????,??????,?????? CharacterActionLogCode = 152,153,154,155 ?? by ??? at 2012-07-30
68 = ?????? CharacterActionLog(156=C2C??, 157=C2C??, 158=C2C???? by ??? at 2012-12-06
70 = ??? ?? ?? ??(159=?? ?? ??) by ??? at 2012-12-20
71 = ???? (160=????????, 161=???? ?? ???) by ??? at 2012-12-26
72 = ?? ?? ?? 162=??????, 163=??(??) by ??? at 2013-01-21
74 = ?? ?? ?? 167=?? ???(??), 168=?? ?? ??(??), 169=?? ??? ??(??) by ??? at 2013-01-28
79 = ??? ??? ?? ?? ?? ?? by ??? at 2013-03-25
82 = PC? ??? ?? by ??? at 2013-05-30
83 = PC? ????? ?? (178=PC????, 179=PC?????) by ??? at 2013-05-30
86 = 180=??? ?????? by ??? at 2013-08-30
**/

CREATE PROCEDURE [dbo].[P_AddCharacterActionLog]
	  @dtmBasisDate datetime --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcLogTableSuffix nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112);
DECLARE @nvcLogTableS nvarchar(128), @nvcLogTableT nvarchar(128) = N'CharacterActionLogs_' + @nvcLogTableSuffix;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'TradeRegisterLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(MM, -3, @dtmBasisDate), 112)
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
		
	SET @nvcLogTableS = N'CashRebirthCoinLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL, 1, CoinCountBefore, CoinCountAfter, MapID, IP, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK);'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'CharacterLevelLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL, CharacterLevelChangeCode + 6, CurrentCharacterLevel - ChangedCharacterLevel, CurrentCharacterLevel, MapID, NULL, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE CharacterLevelChangeCode BETWEEN 1 AND 3;'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'CharacterLoginLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LoginDate, NULL, 131, NULL, NULL, NULL, LoginIP, PCRoomFlag, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'CharacterLogoutLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogoutDate, LoginDate, 10, NULL, NULL, NULL, LoginIP, PCRoomFlag, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'CoinLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL
	, CASE
		WHEN CoinChangeCode BETWEEN 1 AND 12 THEN CoinChangeCode + 10
		WHEN CoinChangeCode BETWEEN 14 AND 25 THEN CoinChangeCode + 9
		WHEN CoinChangeCode = 26 THEN 112
		WHEN CoinChangeCode = 27 THEN 119
		WHEN CoinChangeCode = 28 THEN 121
		WHEN CoinChangeCode = 29 THEN 126
		WHEN CoinChangeCode = 30 THEN 127
		WHEN CoinChangeCode = 31 THEN 124
		WHEN CoinChangeCode = 32 THEN 125
		WHEN CoinChangeCode = 33 THEN 146
		WHEN CoinChangeCode = 34 THEN 147
		WHEN CoinChangeCode = 35 THEN 136
		WHEN CoinChangeCode = 36 THEN 137
		WHEN CoinChangeCode = 37 THEN 138
		WHEN CoinChangeCode = 45 THEN 144
		WHEN CoinChangeCode = 46 THEN 145
		WHEN CoinChangeCode = 47 THEN 152
		WHEN CoinChangeCode = 48 THEN 153
		WHEN CoinChangeCode = 49 THEN 156
		WHEN CoinChangeCode = 50 THEN 157
		WHEN CoinChangeCode = 51 THEN 158
		WHEN CoinChangeCode = 52 THEN 160
		WHEN CoinChangeCode = 53 THEN 161
		WHEN CoinChangeCode = 55 THEN 167
		WHEN CoinChangeCode = 56 THEN 168
		WHEN CoinChangeCode = 57 THEN 169
		WHEN CoinChangeCode = 54 THEN 170
		WHEN CoinChangeCode = 58 THEN 176
		WHEN CoinChangeCode = 59 THEN 171
		WHEN CoinChangeCode = 60 THEN 177
	END
	, CurrentCoin - ChangedCoin, CurrentCoin, MapID, IP
	, CASE WHEN CoinChangeCode IN (1, 6, 7, 11, 12, 16, 17, 18, 20, 21, 22, 23, 24, 29, 30, 31, 32, 35, 36, 37, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60) THEN CoinChangeFKey ELSE NULL END
	, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE CoinTypeCode = 1 AND (CoinChangeCode BETWEEN 1 AND 59 AND CoinChangeCode NOT IN (13, 38, 39, 40, 41, 42, 43, 44));'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'CoinLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL
	, CASE CoinChangeCode
		WHEN 8 THEN 35
		WHEN 9 THEN 36
		WHEN 26 THEN 113
	END
	, CurrentCoin - ChangedCoin, CurrentCoin, MapID, IP, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE CoinTypeCode = 2 AND (CoinChangeCode BETWEEN 8 AND 9 OR CoinChangeCode = 26);'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'CoinLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL
	, CASE CoinTypeCode
		WHEN 3 THEN 37
		WHEN 4 THEN 38
	END
	, CurrentCoin - ChangedCoin, CurrentCoin, MapID, IP, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE CoinTypeCode BETWEEN 3 AND 4 AND CoinChangeCode = 5;'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'FatigueLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL, FatigueTypeCode + 38, CurrentFatigue - ChangedFatigue, CurrentFatigue, MapID, NULL, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE FatigueTypeCode BETWEEN 1 AND 3;'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'ItemLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, Amount3, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT COALESCE(OwnerCharacterID, OwnerCharacterIDBefore), LogDate, CASE WHEN ItemLogCode = 65 THEN DATEADD(MINUTE, ItemLogFKey2, LogDate) ELSE NULL END
	, CASE ItemLogCode
		WHEN 1 THEN 42
		WHEN 3 THEN 43
		WHEN 4 THEN 44
		WHEN 5 THEN 45
		WHEN 6 THEN 46
		WHEN 7 THEN 47
		WHEN 9 THEN 107
		WHEN 10 THEN 48
		WHEN 13 THEN 49
		WHEN 14 THEN 50
		WHEN 15 THEN 51
		WHEN 16 THEN 52
		WHEN 17 THEN 53
		WHEN 18 THEN 54
		WHEN 20 THEN 55
		WHEN 21 THEN 56
		WHEN 22 THEN 57
		WHEN 23 THEN 58
		WHEN 24 THEN 59
		WHEN 26 THEN 60
		WHEN 27 THEN 61
		WHEN 28 THEN 62
		WHEN 29 THEN 64
		WHEN 30 THEN 66
		WHEN 31 THEN 67
		WHEN 33 THEN 68
		WHEN 34 THEN 69
		WHEN 35 THEN 70
		WHEN 36 THEN 71
		WHEN 37 THEN 72
		WHEN 38 THEN 73
		WHEN 12 THEN 104
		WHEN 39 THEN 105
		WHEN 40 THEN 106
		WHEN 41 THEN 111
		WHEN 42 THEN 120
		WHEN 43 THEN 118
		WHEN 45 THEN 122
		WHEN 46 THEN 123
		WHEN 47 THEN 128
		WHEN 48 THEN 129
		WHEN 49 THEN 130
		WHEN 50 THEN 132
		WHEN 51 THEN 133
		WHEN 52 THEN 134
		WHEN 53 THEN 135
		WHEN 54 THEN 142
		WHEN 62 THEN 148
		WHEN 63 THEN 149
		WHEN 65 THEN 150
		WHEN 66 THEN 151
		WHEN 67 THEN 159
		WHEN 68 THEN 154
		WHEN 69 THEN 155
		WHEN 73 THEN 162
		WHEN 74 THEN 163
		WHEN 70 THEN 164
		WHEN 71 THEN 165
		WHEN 72 THEN 166
		WHEN 75 THEN 172
		WHEN 76 THEN 173
		WHEN 77 THEN 174
		WHEN 78 THEN 175
		WHEN 57 THEN 180
	END
	, CASE WHEN ItemLogCode IN (12,66) THEN ItemLevel WHEN ItemLogCode = 40 THEN SealCount WHEN ItemLogCode IN (62,63) THEN ItemPotential ELSE ItemCountBefore END
	, CASE WHEN ItemLogCode IN (12,66) THEN ItemPotential WHEN ItemLogCode = 40 THEN SoulBoundFlag WHEN ItemLogCode IN (62,63) THEN ItemPotentialMoveCount ELSE ItemCount END
	, CASE WHEN ItemLogCode = 40 THEN ItemOption ELSE NULL END
	, CASE WHEN ItemLogCode IN (1, 15, 47, 48, 49, 62, 63, 68, 69) THEN ItemLogFKey1 ELSE NULL END, IP, ItemID, ItemSerial
	, CASE WHEN ItemLogCode IN (3, 4, 6, 7, 8, 18, 24, 26, 28, 30, 31, 33, 34, 35, 36, 37, 45, 46, 47, 62, 63) THEN ItemLogFKey2
		   WHEN ItemLogCode IN (29,68) THEN OwnerCharacterIDBefore
		   WHEN ItemLogCode IN (12,66) THEN ItemLogFKey1 -- ??? ?? ??? ItemID (?? ?? ??), ?? ??? ?? ??? ItemSerial
		   ELSE NULL
	  END
	, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE ItemLogCode IN (1, 3, 4, 5, 6, 7, 9, 10, 12, 13, 14, 15, 16, 17, 18, 20, 21, 22, 23, 24, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 62, 63, 65, 66, 67, 68, 69, 73, 74, 75, 76, 77, 78)
	AND (OwnerCharacterID IS NOT NULL OR OwnerCharacterIDBefore IS NOT NULL);'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'ItemLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT OwnerCharacterIDBefore, LogDate, NULL
	, CASE ItemLogCode
		WHEN 28 THEN 63
		WHEN 29 THEN 65
		WHEN 58 THEN 139
		WHEN 59 THEN 140
		WHEN 60 THEN 141
	END
	, ItemCountBefore, ItemCount, NULL, IP, ItemID, ItemSerial
	, CASE WHEN ItemLogCode = 29 THEN OwnerCharacterID ELSE ItemLogFKey1 END
	, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE ItemLogCode IN (28, 29, 58, 59, 60);'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'JobChangeLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL, 74, NULL, NULL, NULL, NULL, JobCode, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK);'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'MailLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CASE MailTypeCode WHEN 1 THEN SenderCharacterID ELSE ReceiverCharacterID END, SendDate, NULL
	, CASE MailTypeCode WHEN 1 THEN 75 WHEN 2 THEN 76 WHEN 3 THEN 77 END
	, AttachCoin, AttachItemFlag, NULL, NULL, MailID, CASE MailTypeCode WHEN 1 THEN ReceiverCharacterID ELSE NULL END, NULL, Subject
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE MailTypeCode BETWEEN 1 AND 3;'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'MapStayLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, ExitDate, EntranceDate, 78, NULL, NULL, MapID, NULL, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK);'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'MissionAchieveLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, AchieveDate, NULL, 79, NULL, NULL, NULL, NULL, MissionID, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK);'
	EXEC (@nvcStmt);

	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, Amount3, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT A.CharacterID, A.PVPStartDate, A.PVPEndDate, 80, A.vsWarriorKOWin + A.vsArcherKOWin + A.vsSorceressKOWin + A.vsClericKOWin + A.vsAcademicKOWin + A.vsKaliKOWin, A.vsWarriorKOLose + A.vsArcherKOLose + A.vsSorceressKOLose + A.vsClericKOLose + A.vsAcademicKOLose + A.vsKaliKOLose
	,A.PVPResultCode, B.MapID, NULL, A.PVPStartID, PVPModeCode, NULL, NULL
FROM dbo.PVPEndLogs_' + @nvcLogTableSuffix + N' A WITH (NOLOCK)
	INNER JOIN dbo.PVPLogs_' + @nvcLogTableSuffix + N' B WITH (NOLOCK) ON A.PVPStartID = B.PVPStartID AND A.WorldID = B.WorldID;'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'QuestCompleteLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, CompleteDate, NULL, 81, NULL, NULL, NULL, NULL, QuestID, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK);'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'SkillLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL
	, CASE SkillChangeCode
		WHEN 2 THEN 82
		WHEN 3 THEN 83
		WHEN 6 THEN 84
		WHEN 8 THEN 85
		WHEN 12 THEN 86
		WHEN 13 THEN 87
		WHEN 15 THEN 88
	END, SkillLevelBefore, SkillLevelAfter, MapID, NULL, SkillID, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE SkillChangeCode IN (2, 3, 6, 8, 12, 13, 15);'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'SkillPointLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogDate, NULL
	, CASE SkillPointChangeCode
		WHEN 1 THEN 89
		WHEN 2 THEN 90
		WHEN 3 THEN 91
		WHEN 4 THEN 92
		WHEN 6 THEN 93
		WHEN 7 THEN 94
		WHEN 8 THEN 95
		WHEN 9 THEN 114
		WHEN 10 THEN 115
	END, SkillPointBefore, SkillPointAfter, NULL, NULL, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE SkillPointChangeCode IN (1, 2, 3, 4, 6, 7, 8, 9, 10);'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'StageEndLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, Amount3, MapID, IP, LogFKey1)
SELECT CharacterID, StageStartDate, StageEndDate, 96, DifficultyCode, ClearTime, ClearRound, MapID, NULL, StageStartID
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK);'
	EXEC (@nvcStmt);
	SET @nvcLogTableS = N'TradeRegisterLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, RegisterDate, NULL, 97, ItemLevel, Price, NULL, NULL, TradeID, ItemID, ItemSerial, PayMethodCode
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK);'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'DarkLairLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, Amount3, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID1, CAST(CONVERT(char(19), LogDate, 121) as datetime2(0)), NULL, 143, PartySize, PlayTime, PlayRound, MapID, NULL, JobCode1, CharacterLevel1, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
UNION ALL
SELECT CharacterID2, CAST(CONVERT(char(19), LogDate, 121) as datetime2(0)), NULL, 143, PartySize, PlayTime, PlayRound, MapID, NULL, JobCode2, CharacterLevel2, NULL, NULL 
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE CharacterID2 IS NOT NULL
UNION ALL
SELECT CharacterID3, CAST(CONVERT(char(19), LogDate, 121) as datetime2(0)), NULL, 143, PartySize, PlayTime, PlayRound, MapID, NULL, JobCode3, CharacterLevel3, NULL, NULL 
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE CharacterID3 IS NOT NULL
UNION ALL
SELECT CharacterID4, CAST(CONVERT(char(19), LogDate, 121) as datetime2(0)), NULL, 143, PartySize, PlayTime, PlayRound, MapID, NULL, JobCode4, CharacterLevel4, NULL, NULL 
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE CharacterID4 IS NOT NULL'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'TradeSuccessLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT BuyerCharacterID, TradeDate, NULL, 98, ItemLevel, Price, NULL, NULL, TradeID, ItemID, ItemSerial, PayMethodCode
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK);'
	EXEC (@nvcStmt);
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, Amount3, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT V.CharacterID, L.TradeDate, NULL, 99, L.ItemLevel, L.Price, L.RandomSeed, NULL, NULL, L.TradeID, L.ItemID, L.ItemSerial, L.PayMethodCode
FROM dbo.TradeSuccessLogs_' + @nvcLogTableSuffix + N' L WITH (NOLOCK)
	INNER JOIN (' + @nvcStmtOUT + N') V ON L.TradeID = V.TradeID AND L.WorldID = V.WorldID'
	EXEC (@nvcStmt);

	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, OrderDate, NULL
	, CASE
		WHEN GiftFlag = 0 AND PayMethodCode = 2 THEN 100
		WHEN GiftFlag = 0 AND PayMethodCode = 3 THEN 101
		WHEN GiftFlag = 1 AND PayMethodCode = 2 THEN 102
		WHEN PayMethodCode = 4 THEN 103
		WHEN PayMethodCode = 5 THEN 108
		WHEN PayMethodCode = 6 THEN 109
		WHEN PayMethodCode = 7 THEN 110
		WHEN PayMethodCode = 8 THEN 116
		WHEN PayMethodCode = 9	THEN 117
	END, PD.ListPrice, NULL, NULL, PO.IP, PO.PurchaseOrderID, PD.ProductID, CASE WHEN GiftFlag = 1 AND PayMethodCode = 2 THEN ReceiverCharacterID ELSE NULL END, NULL
FROM dbo.PurchaseOrders PO WITH (NOLOCK)
	INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
WHERE PO.OrderStatusCode = 2 AND PO.OrderDate >= ''' + CONVERT(nvarchar(10), @dtmBasisDate, 121) + N''' AND PO.OrderDate < ''' + CONVERT(nvarchar(10), @dtmBasisDate + 1, 121) + N'''
	AND PO.CharacterID IS NOT NULL
	AND PO.PayMethodCode BETWEEN 2 AND 9;'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'PCRoomCharacterLoginLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LoginDate, NULL, 178, NULL, NULL, NULL, LoginIP, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)'
	EXEC (@nvcStmt);

	SET @nvcLogTableS = N'CharacterLogoutLogs_' + @nvcLogTableSuffix;
	SET @nvcStmt = N'
INSERT dbo.' + @nvcLogTableT + N' (CharacterID, LogDate, LogFDate, CharacterActionLogCode, Amount1, Amount2, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogMessage1)
SELECT CharacterID, LogoutDate, LoginDate, 179, NULL, NULL, NULL, LoginIP, NULL, NULL, NULL, NULL
FROM dbo.' + @nvcLogTableS + N' WITH (NOLOCK)
WHERE PCRoomFlag = 1'
	EXEC (@nvcStmt);

	SET @nvcStmt = N'CREATE CLUSTERED INDEX IX_NC_' + @nvcLogTableT + N'_CharacterID_LogDate ON dbo.' + @nvcLogTableT + N' (CharacterID, LogDate)'
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
