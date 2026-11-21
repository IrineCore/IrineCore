USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListActiveGameMoneyByCharacterLevel]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-09
description : ?? ?? ?? ?? ??? ??? ???? ?????.
result set : BasisDate, WorldName, TotalGameMoney, TotalCharacterCount, Band1GameMoney, Band1CharacterCount, Band2GameMoney, Band2CharacterCount, Band3GameMoney, Band3CharacterCount, Band4GameMoney, Band4CharacterCount, Band5GameMoney, Band5CharacterCount, Band6GameMoney, Band6CharacterCount, Band7GameMoney, Band7CharacterCount, Band8GameMoney, Band8CharacterCount, Band9GameMoney, Band9CharacterCount, Band10GameMoney, Band10CharacterCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ??? ???? ?? ?? ?? by ??? at 2012-03-06
**/

CREATE PROCEDURE [dbo].[P_GetListActiveGameMoneyByCharacterLevel]
	  @datStartDate date --// ?? ?? ??
	, @datEndDate date --// ?? ? ??
	, @vchLevelDefine varchar(max) --// ??? ?? ?? ?? / ?? 10?? ??? ??? ? ????. ?) '1,23|24,31|32,39|40,40' => 1 ~ 23 / 24 ~ 31 / 32 ~ 39 / 40 ~ 40
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblLevelDefine table (Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, StartCharacterLevel tinyint NOT NULL, EndCharacterLevel tinyint NOT NULL);
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblLevelDefine (StartCharacterLevel, EndCharacterLevel)
	SELECT CAST(LEFT(string, CHARINDEX(',', string) - 1) AS tinyint), CAST(SUBSTRING(string, CHARINDEX(',', string) + 1, LEN(string) - CHARINDEX(',', string)) AS tinyint)
	FROM dbo.IF_Split(@vchLevelDefine, '|')
	WHERE string > '';

	WITH C1 AS (
		SELECT BasisDate, WorldID, 0 AS Seq, GameMoneyAmount, CharacterCount
		FROM dbo.ST_ActiveGameMoney WITH (NOLOCK)
		WHERE ActiveGameMoneyCode = 1 AND BasisDate BETWEEN @datStartDate AND @datEndDate
		UNION ALL
		SELECT ST1.BasisDate, ST1.WorldID, T.Seq, SUM(ST1.GameMoneyAmount) AS GameMoneyAmount, SUM(ST1.CharacterCount) AS CharacterCount
		FROM dbo.ST_ActiveGameMoneyByCharacterLevel ST1 WITH (NOLOCK)
			INNER JOIN @tblLevelDefine T ON ST1.CharacterLevel BETWEEN T.StartCharacterLevel AND T.EndCharacterLevel
		WHERE ST1.BasisDate BETWEEN @datStartDate AND @datEndDate
		GROUP BY ST1.BasisDate, ST1.WorldID, T.Seq
	)
	SELECT C1.BasisDate, W.WorldName
		, ISNULL(MAX(CASE C1.Seq WHEN 0 THEN GameMoneyAmount END), 0) AS TotalGameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 0 THEN CharacterCount END), 0) AS TotalCharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 1 THEN GameMoneyAmount END), 0) AS Band1GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 1 THEN CharacterCount END), 0) AS Band1CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 2 THEN GameMoneyAmount END), 0) AS Band2GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 2 THEN CharacterCount END), 0) AS Band2CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 3 THEN GameMoneyAmount END), 0) AS Band3GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 3 THEN CharacterCount END), 0) AS Band3CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 4 THEN GameMoneyAmount END), 0) AS Band4GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 4 THEN CharacterCount END), 0) AS Band4CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 5 THEN GameMoneyAmount END), 0) AS Band5GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 5 THEN CharacterCount END), 0) AS Band5CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 6 THEN GameMoneyAmount END), 0) AS Band6GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 6 THEN CharacterCount END), 0) AS Band6CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 7 THEN GameMoneyAmount END), 0) AS Band7GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 7 THEN CharacterCount END), 0) AS Band7CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 8 THEN GameMoneyAmount END), 0) AS Band8GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 8 THEN CharacterCount END), 0) AS Band8CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 9 THEN GameMoneyAmount END), 0) AS Band9GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 9 THEN CharacterCount END), 0) AS Band9CharacterCount
		, ISNULL(MAX(CASE C1.Seq WHEN 10 THEN GameMoneyAmount END), 0) AS Band10GameMoney
		, ISNULL(MAX(CASE C1.Seq WHEN 10 THEN CharacterCount END), 0) AS Band10CharacterCount
	FROM C1
		INNER JOIN dbo.DimWorlds W WITH (NOLOCK) ON C1.WorldID = W.WorldID
	GROUP BY C1.BasisDate, C1.WorldID, W.WorldName
	ORDER BY C1.BasisDate, C1.WorldID;
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
