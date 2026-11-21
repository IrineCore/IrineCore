USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListActionLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 38
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-01
description : ?? ??? ?????.
result set : LogDate/datetime2, CharacterActionLogCode/tinyint, ValueAfter/bigint, ValueDelta/bigint, GameMoneyAfter/decimal, GameMoneyDelta/decimal, AttachGameMoney/decimal, AttachItemFlag/bit, PVPWin/bigint, PVPLose/bigint, DifficultyCode/bigint, ClearTime/varchar, Price/decimal, ItemLevel/bigint, ItemPotential/bigint, SealCount/bigint, SoulBoundFlag/bit, MapName/nvarchar, IP/varchar, LogFKey1/bigint, LogFKey2/bigint, LogFKey3/bigint, LogFDate/datetime2, LogMessage1/nvarchar, ItemName/nvarchar, LevelLimit/tinyint, ItemRankCode/tinyint, CharacterName/nvarchar, ProductName/nvarchar, QuestName/nvarchar, SkillName/nvarchar, JobName/nvarchar, CharacterID/bigint, GuildName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ?? ?? ?? at 2010-07-22
3 = CharacterActionLogCode ?? (122, 123) at 2010-10-06
4 = CharacterActionLogCode ?? (124 ~ 128) at 2010-10-13
5 = ?? ?? at 2010-11-01
6 = CharacterActionLogCode ?? (129 ~ 131) at 2010-12-23
7 = ?? ??? CharacterID ?? at 2011-01-26
8 = CharacterActionLogCode 107??, ItemID ????? ?? at 2011-03-02
9 = ItemID ???? ??? ?? at 2011-03-03
10 = CharacterActionLogCode ?? (132 ~ 134) at 2011-03-03
12 = ItemID ???? ??? ?? at 2011-09-01 by ???
13 = ClearRound ??	 at 2011-10-11 by ohtack
14 = StageStartID ??	 at 2011-10-13 by ohtack
15 = StageStartID ?? 	 at 2011-10-13 by ohtack
16~19 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
20 = CharacterActionLogCode (143) ?? by ??? at 2012-02-08
21~24 = PVP?????? Result Set? ?? ?? by ??? at 2012-02-29
25 = CharacterActionLogCode (144, 145) ?? by ??? at 2012-03-27
26 = CharacterActionLogCode (146, 147) ?? by ??? at 2012-03-27
27 = ??? ?? ???? CharacterActionLogCode (148, 149) ?? by ??? at 2012-04-19
28 = ????? ?? ???? CharacterActionLogCode 150 ?? by ??? at 2012-04-19
29 = ?? ?? ???? CharacterActionLogCode 151 ?? by ??? at 2012-04-19
31 = ?????? CharacterActionLog(156=C2C??, 157=C2C??, 158=C2C????) by ??? at 2012-12-06
32 = ????? ??(159) by ??? at 2012-12-20
33 = ????(160=????????, 161=???? ?? ???) by ??? at 2012-12-26
34 = 162=??????, 163=??(??) by ??? at 2013-01-21
35 = ?? ?? ?? 167=?? ???(??), 168=?? ?? ??(??), 169=?? ??? ??(??) by ??? at 2013-01-28
37 = ??? ?? by ??? at 2013-04-12
**/

CREATE PROCEDURE [dbo].[P_GetListActionLog]
	  @inbCharacterID bigint --// ??? ID
	, @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ? ??
	, @vchCharacterActionLogCodes varchar(1000) = NULL --// ??? CharacterActionLogCode ??? (??? ,). NULL? ???? ?? ???? ?????.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @nvcStmt nvarchar(max)
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CharacterActionLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112)	
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

	IF @vchCharacterActionLogCodes > ''
	BEGIN
		SET @nvcStmt = N'
DECLARE @intWorldID int
SELECT @intWorldID = WorldID FROM dbo.DimCharacters WITH(NOLOCK) WHERE CharacterID = @inbCharacterID

DECLARE @tblCharacterActionLogCodes table (
	CharacterActionLogCode tinyint NOT NULL PRIMARY KEY
);

DECLARE @tblTemp table (
	LogDate datetime2(0),
	CharacterActionLogCode tinyint,
	Amount1 bigint,
	Amount2 bigint,
	Amount3 bigint,
	MapID int,
	IP int,
	LogFKey1 bigint,
	LogFKey2 bigint,
	LogFKey3 bigint,
	LogFDate datetime2(0),
	LogMessage1 nvarchar(100),
	ItemID int,
	ProductID int,
	QuestID int,
	SkillID int,
	JobCode tinyint,
	CharacterID bigint,
	GuildID int
);
INSERT @tblCharacterActionLogCodes (CharacterActionLogCode)
SELECT CAST(string AS tinyint) FROM dbo.IF_Split(@vchCharacterActionLogCodes, '','') WHERE string > '''';

INSERT @tblTemp
SELECT TOP (1000) L.LogDate, L.CharacterActionLogCode, L.Amount1, L.Amount2, L.Amount3, L.MapID, L.IP, L.LogFKey1, L.LogFKey2, L.LogFKey3, L.LogFDate, L.LogMessage1
	, CASE WHEN L.CharacterActionLogCode BETWEEN 42 AND 73 OR L.CharacterActionLogCode BETWEEN 104 AND 107 OR L.CharacterActionLogCode IN (111, 118, 120, 122, 123, 128, 129, 130, 132, 133, 134, 135, 144, 145, 148, 149, 150, 151, 154, 155, 159, 162, 163) THEN CAST(L.LogFKey1 AS int) WHEN L.CharacterActionLogCode BETWEEN 97 AND 99 THEN CAST(L.LogFKey2 AS int) ELSE NULL END AS ItemID
	, CASE WHEN L.CharacterActionLogCode BETWEEN 100 AND 103 THEN CAST(L.LogFKey2 AS int) ELSE NULL END AS ProductID
	, CASE WHEN L.CharacterActionLogCode IN (2, 11, 81) THEN CAST(L.LogFKey1 AS int) WHEN L.CharacterActionLogCode = 43 THEN CAST(L.LogFKey3 AS int) ELSE NULL END AS QuestID
	, CASE WHEN L.CharacterActionLogCode = 26 OR L.CharacterActionLogCode BETWEEN 82 AND 88 THEN CAST(L.LogFKey1 AS int) ELSE NULL END AS SkillID
	, CASE WHEN L.CharacterActionLogCode IN (74, 143) THEN CAST(L.LogFKey1 AS tinyint) ELSE NULL END AS JobCode
	, CASE WHEN L.CharacterActionLogCode IN (21, 22) THEN L.LogFKey1 WHEN L.CharacterActionLogCode = 75 THEN L.LogFKey2 WHEN L.CharacterActionLogCode IN (64, 65, 102, 128) THEN L.LogFKey3 ELSE NULL END AS CharacterID
	, CASE WHEN L.CharacterActionLogCode IN (124, 125) THEN L.LogFKey1 WHEN L.CharacterActionLogCode IN (122, 123) THEN L.LogFKey3 END AS GuildID
FROM (' + @nvcStmtOUT + N') L
	INNER JOIN @tblCharacterActionLogCodes T ON L.CharacterActionLogCode = T.CharacterActionLogCode
WHERE L.CharacterID = @inbCharacterID AND L.LogDate BETWEEN ''' + CONVERT(nvarchar(19), @dt2StartDate, 121) + N''' AND ''' + CONVERT(nvarchar(19), @dt2EndDate, 121) + N''';
';

		SET @nvcStmt += N'
SELECT L1.LogDate, L1.CharacterActionLogCode
	, CASE WHEN L1.CharacterActionLogCode IN (1, 37, 38, 2, 3, 4, 5, 6, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 62, 64, 67, 69, 71, 72, 107, 118, 55, 56, 57, 58, 59, 60, 61, 63, 65, 66, 68, 70, 73, 111, 120, 7, 8, 9, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 114, 115, 105, 122, 123, 124, 125, 126, 127, 128, 129, 130, 162, 163, 175, 174) THEN L1.Amount2 END AS ValueAfter
	, CASE WHEN L1.CharacterActionLogCode IN (1, 37, 38, 2, 3, 4, 5, 6, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 62, 64, 67, 69, 71, 72, 107, 118, 55, 56, 57, 58, 59, 60, 61, 63, 65, 66, 68, 70, 73, 111, 120, 7, 8, 9, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 114, 115, 105, 122, 123, 124, 125, 126, 127, 128, 129, 130, 162, 163, 175, 174) THEN L1.Amount2 - L1.Amount1 END AS ValueDelta
	, CASE WHEN L1.CharacterActionLogCode IN (11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 112, 113, 119, 121, 124, 125, 126, 127, 144, 145, 146, 147, 152, 153, 156, 157, 158, 160, 161, 167, 168, 169, 176) THEN CAST(L1.Amount2 / 10000.0 AS decimal(19, 4)) END AS GameMoneyAfter
	, CASE WHEN L1.CharacterActionLogCode IN (11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 112, 113, 119, 121, 124, 125, 126, 127, 144, 145, 146, 147, 152, 153, 156, 157, 158, 160, 161, 167, 168, 169, 176) THEN CAST((L1.Amount2 - L1.Amount1) / 10000.0 AS decimal(19, 4)) END AS GameMoneyDelta
	, CASE WHEN L1.CharacterActionLogCode IN (75, 76, 77) THEN CAST(L1.Amount1 / 10000.0 AS decimal(19, 4)) END AS AttachGameMoney
	, CASE WHEN L1.CharacterActionLogCode IN (75, 76, 77) THEN CAST(L1.Amount2 AS bit) END AS AttachItemFlag
	, CASE WHEN L1.CharacterActionLogCode IN (80) THEN L1.Amount1 END AS PVPWin
	, CASE WHEN L1.CharacterActionLogCode IN (80) THEN L1.Amount2 END AS PVPLose
	, CASE WHEN L1.CharacterActionLogCode IN (80) THEN L1.Amount3 END AS PVPResultCode
	, CASE WHEN L1.CharacterActionLogCode IN (80) THEN L1.LogFKey2 END AS PVPModeCode
	, CASE WHEN L1.CharacterActionLogCode IN (96) THEN L1.Amount1 END AS DifficultyCode
	, CASE WHEN L1.CharacterActionLogCode IN (96, 143) THEN CAST(L1.Amount2 / 3600 AS varchar(10)) + '':'' + RIGHT(''0'' + CAST(L1.Amount2 % 3600 / 60 AS varchar(10)), 2) + '':'' + RIGHT(''0'' + CAST(L1.Amount2 % 60 AS varchar(10)), 2) END AS ClearTime
	, CASE WHEN L1.CharacterActionLogCode IN (96, 143) THEN L1.Amount3 END AS ClearRound
	, CASE WHEN L1.CharacterActionLogCode IN (97, 98, 99) THEN CAST(L1.Amount2 / 10000.0 AS decimal(19, 4)) WHEN L1.CharacterActionLogCode IN (100, 101, 102, 108, 109, 110, 116, 117) THEN L1.Amount1 END AS Price
	, CASE WHEN L1.CharacterActionLogCode IN (97, 98, 99, 104, 151) THEN L1.Amount1 END AS ItemLevel
	, CASE WHEN L1.CharacterActionLogCode IN (104, 151) THEN L1.Amount2 WHEN L1.CharacterActionLogCode IN (148, 149) THEN L1.Amount1 END AS ItemPotential
	, CASE WHEN L1.CharacterActionLogCode IN (106) THEN L1.Amount1 END AS SealCount
	, CASE WHEN L1.CharacterActionLogCode IN (106) THEN CAST(L1.Amount2 AS bit) END AS SoulBoundFlag
	, CASE WHEN L1.CharacterActionLogCode IN (148, 149) THEN CAST(L1.Amount2 AS tinyint) END AS ItemPotentialMoveCount
	, CASE WHEN L1.CharacterActionLogCode IN (150) THEN L1.LogFDate END AS ItemExpireDate
	, M.MapName, dbo.FN_IPNumber2IP(L1.IP) AS IP, L1.LogFKey1, L1.LogFKey2, L1.LogFKey3, L1.LogFDate, L1.LogMessage1
	, I.ItemName, I.LevelLimit, I.ItemRankCode
	, CH.CharacterName, P.ProductName, Q.QuestName, SK.SkillName, J.JobName, L1.CharacterID, G.GuildName
FROM @tblTemp L1
	LEFT OUTER JOIN dbo.DimMaps M WITH (NOLOCK) ON L1.MapID = M.MapID
	LEFT OUTER JOIN dbo.DimItems I WITH (NOLOCK) ON L1.ItemID = I.ItemID
	LEFT OUTER JOIN dbo.DimProducts P WITH (NOLOCK) ON L1.ProductID = P.ProductID
	LEFT OUTER JOIN dbo.DimQuests Q WITH (NOLOCK) ON L1.QuestID = Q.QuestID
	LEFT OUTER JOIN dbo.DimSkills SK WITH (NOLOCK) ON L1.SkillID = SK.SkillID
	LEFT OUTER JOIN dbo.DimJobs J WITH (NOLOCK) ON L1.JobCode = J.JobCode
	LEFT OUTER JOIN dbo.DimCharacters CH WITH (NOLOCK, FORCESEEK) ON L1.CharacterID = CH.CharacterID
	LEFT OUTER JOIN dbo.SS_Guilds G WITH (NOLOCK) ON L1.GuildID = G.GuildID AND G.WorldID = @intWorldID
ORDER BY L1.LogDate;';
	END

	ELSE
	BEGIN
		SET @nvcStmt = N'
DECLARE @intWorldID int
SELECT @intWorldID = WorldID FROM dbo.DimCharacters WITH(NOLOCK) WHERE CharacterID = @inbCharacterID;

DECLARE @tblTemp table (
	LogDate datetime2(0),
	CharacterActionLogCode tinyint,
	Amount1 bigint,
	Amount2 bigint,
	Amount3 bigint,
	MapID int,
	IP int,
	LogFKey1 bigint,
	LogFKey2 bigint,
	LogFKey3 bigint,
	LogFDate datetime2(0),
	LogMessage1 nvarchar(100),
	ItemID int,
	ProductID int,
	QuestID int,
	SkillID int,
	JobCode tinyint,
	CharacterID bigint,
	GuildID int
);

INSERT @tblTemp
SELECT TOP (1000) LogDate, CharacterActionLogCode, Amount1, Amount2, Amount3, MapID, IP, LogFKey1, LogFKey2, LogFKey3, LogFDate, LogMessage1
	, CASE WHEN CharacterActionLogCode BETWEEN 42 AND 73 OR CharacterActionLogCode BETWEEN 104 AND 106 OR L.CharacterActionLogCode IN (144, 145, 148, 149, 150, 151, 154, 155, 159) THEN CAST(LogFKey1 AS int) WHEN CharacterActionLogCode BETWEEN 97 AND 99 THEN CAST(LogFKey2 AS int) ELSE NULL END AS ItemID
	, CASE WHEN CharacterActionLogCode BETWEEN 100 AND 103 THEN CAST(LogFKey2 AS int) ELSE NULL END AS ProductID
	, CASE WHEN CharacterActionLogCode IN (2, 11) THEN CAST(LogFKey1 AS int) WHEN CharacterActionLogCode = 43 THEN CAST(LogFKey3 AS int) ELSE NULL END AS QuestID
	, CASE WHEN CharacterActionLogCode = 26 OR CharacterActionLogCode BETWEEN 82 AND 88 THEN CAST(LogFKey1 AS int) ELSE NULL END AS SkillID
	, CASE WHEN L.CharacterActionLogCode IN (74, 143) THEN CAST(L.LogFKey1 AS tinyint) ELSE NULL END AS JobCode
	, CASE WHEN CharacterActionLogCode IN (21, 22) THEN LogFKey1 WHEN CharacterActionLogCode = 75 THEN LogFKey2 WhEN CharacterActionLogCode IN (64, 65, 102) THEN LogFKey3 ELSE NULL END AS CharacterID
	, CASE WHEN L.CharacterActionLogCode IN (124, 125) THEN L.LogFKey1 WHEN L.CharacterActionLogCode IN (122, 123) THEN L.LogFKey3 END AS GuildID
FROM (' + @nvcStmtOUT + N') L
WHERE CharacterID = @inbCharacterID AND LogDate BETWEEN ''' + CONVERT(nvarchar(19), @dt2StartDate, 121) + N''' AND ''' + CONVERT(nvarchar(19), @dt2EndDate, 121) + N'''
';
		SET @nvcStmt += N'
SELECT L1.LogDate, L1.CharacterActionLogCode
	, CASE WHEN L1.CharacterActionLogCode IN (1, 37, 38, 2, 3, 4, 5, 6, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 62, 64, 67, 69, 71, 72, 107, 118, 55, 56, 57, 58, 59, 60, 61, 63, 65, 66, 68, 70, 73, 111, 120, 7, 8, 9, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 114, 115, 105, 122, 123, 124, 125, 126, 127, 128, 129, 130, 132, 133, 134, 135, 162, 163, 174, 175) THEN L1.Amount2 END AS ValueAfter
	, CASE WHEN L1.CharacterActionLogCode IN (1, 37, 38, 2, 3, 4, 5, 6, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 62, 64, 67, 69, 71, 72, 107, 118, 55, 56, 57, 58, 59, 60, 61, 63, 65, 66, 68, 70, 73, 111, 120, 7, 8, 9, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 114, 115, 105, 122, 123, 124, 125, 126, 127, 128, 129, 130, 132, 133, 134, 135, 162, 163, 174, 175) THEN L1.Amount2 - L1.Amount1 END AS ValueDelta
	, CASE WHEN L1.CharacterActionLogCode IN (11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 112, 113, 119, 121, 124, 125, 126, 127, 144, 145, 146, 147, 152, 153, 156, 157, 158, 160, 161, 167, 168, 169, 176) THEN CAST(L1.Amount2 / 10000.0 AS decimal(19, 4)) END AS GameMoneyAfter
	, CASE WHEN L1.CharacterActionLogCode IN (11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 112, 113, 119, 121, 124, 125, 126, 127, 144, 145, 146, 147, 152, 153, 156, 157, 158, 160, 161, 167, 168, 169, 176) THEN CAST((L1.Amount2 - L1.Amount1) / 10000.0 AS decimal(19, 4)) END AS GameMoneyDelta
	, CASE WHEN L1.CharacterActionLogCode IN (75, 76, 77) THEN CAST(L1.Amount1 / 10000.0 AS decimal(19, 4)) END AS AttachGameMoney
	, CASE WHEN L1.CharacterActionLogCode IN (75, 76, 77) THEN CAST(L1.Amount2 AS bit) END AS AttachItemFlag
	, CASE WHEN L1.CharacterActionLogCode IN (80) THEN L1.Amount1 END AS PVPWin
	, CASE WHEN L1.CharacterActionLogCode IN (80) THEN L1.Amount2 END AS PVPLose
	, CASE WHEN L1.CharacterActionLogCode IN (80) THEN L1.Amount3 END AS PVPResultCode
	, CASE WHEN L1.CharacterActionLogCode IN (80) THEN L1.LogFKey2 END AS PVPModeCode
	, CASE WHEN L1.CharacterActionLogCode IN (96) THEN L1.Amount1 END AS DifficultyCode
	, CASE WHEN L1.CharacterActionLogCode IN (96, 143) THEN CAST(L1.Amount2 / 3600 AS varchar(10)) + '':'' + RIGHT(''0'' + CAST(L1.Amount2 % 3600 / 60 AS varchar(10)), 2) + '':'' + RIGHT(''0'' + CAST(L1.Amount2 % 60 AS varchar(10)), 2) END AS ClearTime
	, CASE WHEN L1.CharacterActionLogCode IN (96, 143) THEN L1.Amount3 END AS ClearRound
	, CASE WHEN L1.CharacterActionLogCode IN (97, 98, 99) THEN CAST(L1.Amount2 / 10000.0 AS decimal(19, 4)) WHEN L1.CharacterActionLogCode IN (100, 101, 102, 108, 109, 110, 116, 117) THEN CAST(L1.Amount1 / 10000.0 AS decimal(19, 4)) END AS Price
	, CASE WHEN L1.CharacterActionLogCode IN (97, 98, 99, 104, 151) THEN L1.Amount1 END AS ItemLevel
	, CASE WHEN L1.CharacterActionLogCode IN (104, 151) THEN L1.Amount2 WHEN L1.CharacterActionLogCode IN (148, 149) THEN L1.Amount1 END AS ItemPotential
	, CASE WHEN L1.CharacterActionLogCode IN (106) THEN L1.Amount1 END AS SealCount
	, CASE WHEN L1.CharacterActionLogCode IN (106) THEN CAST(L1.Amount2 AS bit) END AS SoulBoundFlag
	, CASE WHEN L1.CharacterActionLogCode IN (148, 149) THEN CAST(L1.Amount2 AS tinyint) END AS ItemPotentialMoveCount
	, CASE WHEN L1.CharacterActionLogCode IN (150) THEN L1.LogFDate END AS ItemExpireDate
	, M.MapName, dbo.FN_IPNumber2IP(L1.IP) AS IP, L1.LogFKey1, L1.LogFKey2, L1.LogFKey3, L1.LogFDate, L1.LogMessage1
	, I.ItemName, I.LevelLimit, I.ItemRankCode
	, CH.CharacterName, P.ProductName, Q.QuestName, SK.SkillName, J.JobName, L1.CharacterID, G.GuildName
FROM @tblTemp L1
	LEFT OUTER JOIN dbo.DimMaps M WITH (NOLOCK) ON L1.MapID = M.MapID
	LEFT OUTER JOIN dbo.DimItems I WITH (NOLOCK) ON L1.ItemID = I.ItemID
	LEFT OUTER JOIN dbo.DimProducts P WITH (NOLOCK) ON L1.ProductID = P.ProductID
	LEFT OUTER JOIN dbo.DimQuests Q WITH (NOLOCK) ON L1.QuestID = Q.QuestID
	LEFT OUTER JOIN dbo.DimSkills SK WITH (NOLOCK) ON L1.SkillID = SK.SkillID
	LEFT OUTER JOIN dbo.DimJobs J WITH (NOLOCK) ON L1.JobCode = J.JobCode
	LEFT OUTER JOIN dbo.DimCharacters CH WITH (NOLOCK, FORCESEEK) ON L1.CharacterID = CH.CharacterID
	LEFT OUTER JOIN dbo.SS_Guilds G WITH (NOLOCK) ON L1.GuildID = G.GuildID AND G.WorldID = @intWorldID
ORDER BY L1.LogDate;';
	END

	EXEC sp_executesql @nvcStmt, N'@inbCharacterID bigint, @vchCharacterActionLogCodes varchar(1000)', @inbCharacterID = @inbCharacterID, @vchCharacterActionLogCodes = @vchCharacterActionLogCodes;
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
