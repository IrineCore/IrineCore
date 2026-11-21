USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddWeeklyRank_PVPKillDeath]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-02-22
description : ?? PVP ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = ?? ??? ??? ?? ?? by ??? at 2012-03-15
6 = ?? ?? by ??? at 2012-05-21
**/

CREATE PROCEDURE [dbo].[P_AddWeeklyRank_PVPKillDeath]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int;
DECLARE @dt2StartDate datetime2(0) = CAST(CAST(DATEADD(DAY, - DATEPART(WEEKDAY, GETDATE() - 1), GETDATE()) AS date) AS datetime2(0))
      , @dt2EndDate datetime2(0) = GETDATE()
	  , @datBasisDate date = GETDATE() - 1;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName1 nvarchar(50) = N'PVPLogs'
	, @nvcTblName2 nvarchar(50) = N'PVPEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112)		
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT1 nvarchar(max) = N''
	, @nvcStmtOUT2 nvarchar(max) = N'';


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName1
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT1 OUTPUT;	
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName2
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT2 OUTPUT;	

	TRUNCATE TABLE dbo.WeeklyRank_PVPKillDeath;
	
	SET @nvcStmt = N'
	SELECT B.CharacterID
		 , SUM(vsWarriorKOWin + vsArcherKOWin + vsSorceressKOWin + vsClericKOWin + vsAcademicKOWin + vsKaliKOWin) AS KillCount
		 , SUM(vsWarriorKOLose + vsArcherKOLose + vsSorceressKOLose + vsClericKOLose + vsAcademicKOLose + vsKaliKOLose) AS DeathCount
		 , SUM(vsWarriorKOWin) AS vsWarriorKOWin
		 , SUM(vsWarriorKOLose) AS vsWarriorKOLose
		 , SUM(vsArcherKOWin) AS vsArcherKOWin
		 , SUM(vsArcherKOLose) AS vsArcherKOLose
		 , SUM(vsSorceressKOWin) AS vsSorceressKOWin
		 , SUM(vsSorceressKOLose) AS vsSorceressKOLose
		 , SUM(vsClericKOWin) AS vsClericKOWin
		 , SUM(vsClericKOLose) AS vsClericKOLose
		 , SUM(vsAcademicKOWin) AS vsAcademicKOWin
		 , SUM(vsAcademicKOLose) AS vsAcademicKOLose
		 , SUM(vsKaliKOWin) AS vsKaliKOWin
		 , SUM(vsKaliKOLose) AS vsKaliKOLose
		 , SUM(vsAssasinKOWin) AS vsAssasinKOWin
		 , SUM(vsAssasinKOLose) AS vsAssasinKOLose
	INTO #t1
	FROM (' + @nvcStmtOUT1 + N') A
		INNER JOIN  (' + @nvcStmtOUT2 + N') B ON A.PVPStartID = B.PVPStartID AND A.WorldID = B.WorldID
	WHERE B.PVPStartDate >= @dt2StartDate AND B.PVPStartDate < @dt2EndDate
      AND A.PVPStartDate >= @dt2StartDate AND A.PVPStartDate < @dt2EndDate
      AND A.MaxPlayerCount >= 4
	GROUP BY B.CharacterID;

	ALTER TABLE #t1 ADD CONSTRAINT PK_t2 PRIMARY KEY (CharacterID);
	
	SELECT DC.CharacterClassCode, C0.CharacterID, DC.WorldID, DCS.CharacterLevel, DCS.JobCode, PS.PVPLevel, C0.KillCount, C0.DeathCount, C0.vsWarriorKOWin, C0.vsWarriorKOLose, C0.vsArcherKOWin, C0.vsArcherKOLose, C0.vsSorceressKOWin, C0.vsSorceressKOLose, C0.vsClericKOWin, C0.vsClericKOLose, C0.vsAcademicKOWin, C0.vsAcademicKOLose, C0.vsKaliKOWin, C0.vsKaliKOLose, C0.vsAssasinKOWin, C0.vsAssasinKOLose
	INTO #t2
	FROM #t1 C0
		INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON C0.CharacterID = DC.CharacterID
		INNER JOIN dbo.DailyCharacterStatus DCS WITH (NOLOCK) ON C0.CharacterID = DCS.CharacterID
		INNER JOIN dbo.SS_PVPScores PS WITH (NOLOCK) ON C0.CharacterID = PS.CharacterID
	WHERE DC.DeleteFlag = CAST(0 AS bit) 
	  AND DCS.BasisDate = @datBasisDate
	  AND NOT EXISTS (SELECT * FROM dbo.SS_RestraintedAccounts WITH(NOLOCK) WHERE AccountID = DC.AccountID);

	WITH C2 AS (
		SELECT CharacterID, RANK() OVER(ORDER BY KillCount DESC) AS TotalKillRank
		FROM #t2
	), C3 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY WorldID ORDER BY KillCount DESC) AS WorldKillRank
		FROM #t2
	), C4 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY WorldID, CharacterClassCode ORDER BY KillCount DESC) AS WorldCharacterClassKillRank
		FROM #t2
	), C5 AS (
		SELECT CharacterID, RANK() OVER(ORDER BY DeathCount DESC) AS TotalDeathRank
		FROM #t2
	), C6 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY WorldID ORDER BY DeathCount DESC) AS WorldDeathRank
		FROM #t2
	), C7 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY WorldID, CharacterClassCode ORDER BY DeathCount DESC) AS WorldCharacterClassDeathRank
		FROM #t2
	), C8 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY CharacterClassCode ORDER BY KillCount DESC) AS CharacterClassKillRank
		FROM #t2
	), C9 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY CharacterClassCode ORDER BY DeathCount DESC) AS CharacterClassDeathRank
		FROM #t2
	)
	SELECT C1.CharacterID, C1.WorldID, C1.CharacterLevel, C1.JobCode, C1.PVPLevel, C1.KillCount, C1.DeathCount, C1.vsWarriorKOWin, C1.vsWarriorKOLose, C1.vsArcherKOWin, C1.vsArcherKOLose, C1.vsSorceressKOWin, C1.vsSorceressKOLose, C1.vsClericKOWin, C1.vsClericKOLose, C2.TotalKillRank, C3.WorldKillRank, C4.WorldCharacterClassKillRank, C5.TotalDeathRank, C6.WorldDeathRank, C7.WorldCharacterClassDeathRank, C8.CharacterClassKillRank, C9.CharacterClassDeathRank, C1.vsAcademicKOWin, C1.vsAcademicKOLose, C1.vsKaliKOWin, C1.vsKaliKOLose, C1.vsAssasinKOWin, C1.vsAssasinKOLose
	INTO #WeeklyRank_PVPKillDeath
	FROM #t2 C1
		INNER JOIN C2 ON C1.CharacterID = C2.CharacterID
		INNER JOIN C3 ON C1.CharacterID = C3.CharacterID
		INNER JOIN C4 ON C1.CharacterID = C4.CharacterID
		INNER JOIN C5 ON C1.CharacterID = C5.CharacterID
		INNER JOIN C6 ON C1.CharacterID = C6.CharacterID
		INNER JOIN C7 ON C1.CharacterID = C7.CharacterID
		INNER JOIN C8 ON C1.CharacterID = C8.CharacterID
		INNER JOIN C9 ON C1.CharacterID = C9.CharacterID
	ORDER BY CharacterID;
	
	INSERT dbo.WeeklyRank_PVPKillDeath (CharacterID, WorldID, CharacterLevel, JobCode, PVPLevel, KillCount, DeathCount, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, TotalKillRank, WorldKillRank, WorldCharacterClassKillRank, TotalDeathRank, WorldDeathRank, WorldCharacterClassDeathRank, CharacterClassKillRank, CharacterClassDeathRank, vsAcademicKOWin, vsAcademicKOLose, vsKaliKOWin, vsKaliKOLose, vsAssasinKOWin, vsAssasinKOLose)
	SELECT CharacterID, WorldID, CharacterLevel, JobCode, PVPLevel, KillCount, DeathCount, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, TotalKillRank, WorldKillRank, WorldCharacterClassKillRank, TotalDeathRank, WorldDeathRank, WorldCharacterClassDeathRank, CharacterClassKillRank, CharacterClassDeathRank, vsAcademicKOWin, vsAcademicKOLose, vsKaliKOWin, vsKaliKOLose, vsAssasinKOWin, vsAssasinKOLose
	FROM #WeeklyRank_PVPKillDeath

	IF DATEPART(WEEKDAY, GETDATE()) = 1
		WITH C0 AS (
			SELECT ROW_NUMBER() OVER(PARTITION BY WorldID ORDER BY WorldKillRank) AS Seq, CharacterID, WorldID, CharacterLevel, JobCode, PVPLevel, KillCount, DeathCount, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, TotalKillRank, WorldKillRank, WorldCharacterClassKillRank, TotalDeathRank, WorldDeathRank, WorldCharacterClassDeathRank, CharacterClassKillRank, CharacterClassDeathRank, vsAcademicKOWin, vsAcademicKOLose, vsKaliKOWin, vsKaliKOLose, vsAssasinKOWin, vsAssasinKOLose
			FROM dbo.WeeklyRank_PVPKillDeath
		), C1 AS (
			SELECT ROW_NUMBER() OVER(PARTITION BY WorldID ORDER BY WorldDeathRank) AS Seq, CharacterID, WorldID, CharacterLevel, JobCode, PVPLevel, KillCount, DeathCount, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, TotalKillRank, WorldKillRank, WorldCharacterClassKillRank, TotalDeathRank, WorldDeathRank, WorldCharacterClassDeathRank, CharacterClassKillRank, CharacterClassDeathRank, vsAcademicKOWin, vsAcademicKOLose, vsKaliKOWin, vsKaliKOLose, vsAssasinKOWin, vsAssasinKOLose
			FROM dbo.WeeklyRank_PVPKillDeath
		), C2 AS (
			SELECT CharacterID, WorldID, CharacterLevel, JobCode, PVPLevel, KillCount, DeathCount, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, TotalKillRank, WorldKillRank, WorldCharacterClassKillRank, TotalDeathRank, WorldDeathRank, WorldCharacterClassDeathRank, CharacterClassKillRank, CharacterClassDeathRank, vsAcademicKOWin, vsAcademicKOLose, vsKaliKOWin, vsKaliKOLose, vsAssasinKOWin, vsAssasinKOLose
			FROM C0
			WHERE Seq <= 1000
			UNION
			SELECT CharacterID, WorldID, CharacterLevel, JobCode, PVPLevel, KillCount, DeathCount, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, TotalKillRank, WorldKillRank, WorldCharacterClassKillRank, TotalDeathRank, WorldDeathRank, WorldCharacterClassDeathRank, CharacterClassKillRank, CharacterClassDeathRank, vsAcademicKOWin, vsAcademicKOLose, vsKaliKOWin, vsKaliKOLose, vsAssasinKOWin, vsAssasinKOLose
			FROM C1
			WHERE Seq <= 1000
		)
		INSERT dbo.PastWeeklyRank_PVPKillDeath (BasisDate, CharacterID, WorldID, CharacterLevel, JobCode, PVPLevel, KillCount, DeathCount, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, TotalKillRank, WorldKillRank, WorldCharacterClassKillRank, TotalDeathRank, WorldDeathRank, WorldCharacterClassDeathRank, CharacterClassKillRank, CharacterClassDeathRank, vsAcademicKOWin, vsAcademicKOLose, vsKaliKOWin, vsKaliKOLose, vsAssasinKOWin, vsAssasinKOLose)
		SELECT CAST(@dt2StartDate AS date), CharacterID, WorldID, CharacterLevel, JobCode, PVPLevel, KillCount, DeathCount, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, TotalKillRank, WorldKillRank, WorldCharacterClassKillRank, TotalDeathRank, WorldDeathRank, WorldCharacterClassDeathRank, CharacterClassKillRank, CharacterClassDeathRank, vsAcademicKOWin, vsAcademicKOLose, vsKaliKOWin, vsKaliKOLose, vsAssasinKOWin, vsAssasinKOLose
		FROM C2
		ORDER BY CharacterID;

	DROP TABLE #t1;
	DROP TABLE #t2;'
	
	EXEC sp_executesql @nvcStmt, N'
	  @dt2StartDate datetime2(0), @dt2EndDate datetime2(0), @datBasisDate date'
	, @dt2StartDate = @dt2StartDate, @dt2EndDate = @dt2EndDate, @datBasisDate= @datBasisDate
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
