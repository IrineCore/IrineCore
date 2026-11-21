USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListContentsIndicator]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 16
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-06-21

description :
	??? ?? ?? (??? ?? ?? - ?? ?? ??)
	- Stage : ??? ?? ??. ???? ?? ??. ???? ?? Map ID ? ??.
	- Map : ????, ???, ????, ????(PVP). ??? ?? ??. ? ?? ??. Map ID ? ??.
	- Mission : ?? ?? ??. Mission ID ? ??
result set : 
	version 2 : ??? ?? ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103331 = ???? ???? ????.
history :
	1 = ?? ??
	2 = input parameter ?? (@inyCountType)     by 20110805.wgkim
	3 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
	7 = 60?? ???? ?? ?? by ??? at 2012-06-12
	17 = 70?? ???? ?? ?? by ??? at 2013-06-14
**/

CREATE PROCEDURE [dbo].[P_GetListContentsIndicator]
	  @dtFromDate date --// ?? ?? (??)
	, @dtToDate date --// ?? ?? (??)
	, @intContentsID int --// ??? ID (MapID or Mission ID)
	, @inyContentsType tinyint = 1 --// ??? ?? (1=Stage, 2=Map, 3=Mission)
	, @inyCountType tinyint = 0 --// ??? ?? (0=??, 1=??? ???)
	, @inyDifficultyCode tinyint = NULL --// ???? ??? 
	, @intVersion int = 1 --// Result Set ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inyLevel tinyint;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName1 nvarchar(50) = N'StageEndLogs'
	, @nvcTblName2 nvarchar(50) = N'MapStayLogs'
	, @nvcTblName3 nvarchar(50) = N'MissionAchieveLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtFromDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtToDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**??? ??? ?? master.dbo.spt_values ???? ?? by ??? at 2012-02-07 **/
--DECLARE @Temp_Level TABLE (CharacterLevel int);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SET @dtToDate = DATEADD(day, 1, @dtToDate)

	-- Level
	/**??? ??? ?? master.dbo.spt_values ???? ?? by ??? at 2012-02-07 **/
	--SET @inyLevel = 1;

	--WHILE (@inyLevel <= 50)
	--BEGIN
	--	INSERT @Temp_Level (CharacterLevel) SELECT @inyLevel;
	--	SET @inyLevel = @inyLevel + 1;
	--END

	-- ??? ?? : Stage
	IF @inyContentsType = 1
	BEGIN
		/** ????? ??? ?? by ??? at 2012-02-07 **/
		EXEC dbo.P_MakeTableString
			  @nvcDBName = @nvcDBName
			, @nvcTblName = @nvcTblName1
			, @nvcStartDate = @nvcStartDate
			, @nvcEndDate = @nvcEndDate
			, @nvcHint = @nvcHint
			, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
			
		IF @intVersion = 1 OR @intVersion IS NULL
		BEGIN
			SET @nvcStmt = N'
			WITH T0 AS (
				SELECT DifficultyCode, CharacterLevel, COUNT(' + CASE @inyCountType WHEN 1 THEN N'DISTINCT CharacterID' ELSE N'*' END + N') AS CharacterCount
				  FROM (' + @nvcStmtOUT + N') V
				 WHERE StageStartDate >= CONVERT(datetime2(0), @dtFromDate)
				   AND StageStartDate < CONVERT(datetime2(0), @dtToDate)
				   AND MapID = @intContentsID
				 GROUP BY DifficultyCode, CharacterLevel
			), T1 AS (
				SELECT CharacterLevel, CharacterCount FROM T0 WHERE DifficultyCode = 1
			), T2 AS (
				SELECT CharacterLevel, CharacterCount FROM T0 WHERE DifficultyCode = 2
			), T3 AS (
				SELECT CharacterLevel, CharacterCount FROM T0 WHERE DifficultyCode = 3
			), T4 AS (
				SELECT CharacterLevel, CharacterCount FROM T0 WHERE DifficultyCode = 4
			), T5 AS (
				SELECT CharacterLevel, CharacterCount FROM T0 WHERE DifficultyCode = 5
			)
			SELECT T.CharacterLevel
				 , ISNULL(T1.CharacterCount, 0) AS CharacterCount1
				 , ISNULL(T2.CharacterCount, 0) AS CharacterCount2
				 , ISNULL(T3.CharacterCount, 0) AS CharacterCount3
				 , ISNULL(T4.CharacterCount, 0) AS CharacterCount4
				 , ISNULL(T5.CharacterCount, 0) AS CharacterCount5
			  FROM (SELECT number AS [CharacterLevel] FROM master.dbo.spt_values WITH (NOLOCK) WHERE TYPE = ''p'' AND number BETWEEN 1 AND 70) T
				   LEFT OUTER JOIN T1 ON (T.CharacterLevel = T1.CharacterLevel)
				   LEFT OUTER JOIN T2 ON (T.CharacterLevel = T2.CharacterLevel)
				   LEFT OUTER JOIN T3 ON (T.CharacterLevel = T3.CharacterLevel)
				   LEFT OUTER JOIN T4 ON (T.CharacterLevel = T4.CharacterLevel)
				   LEFT OUTER JOIN T5 ON (T.CharacterLevel = T5.CharacterLevel)
			 ORDER BY T.CharacterLevel;
			';

		EXEC sp_executesql @nvcStmt, N'
			  @dtFromDate date
			, @dtToDate date
			, @intContentsID int'
			, @dtFromDate = @dtFromDate
			, @dtToDate = @dtToDate
			, @intContentsID = @intContentsID;
		END
		ELSE
		BEGIN
			IF OBJECT_ID(N'tempdb..#tblTemp', N'U') IS NOT NULL
				DROP TABLE #tblTemp;

			CREATE TABLE #tblTemp (
				LogDate date NULL,
				CharacterLevel int NULL,
				CharacterCount int NULL
			);

			SET @nvcStmt = N'
				WITH T0 AS (	
					SELECT CAST(StageEndDate AS date) AS LogDate, CharacterLevel, COUNT(' + CASE @inyCountType WHEN 1 THEN N'DISTINCT CharacterID' ELSE N'*' END + N') AS CharacterCount
					  FROM (' + @nvcStmtOUT + N') V
					 WHERE StageStartDate >= CONVERT(datetime2(0), @dtFromDate)
					   AND StageStartDate < CONVERT(datetime2(0), @dtToDate)
					   AND MapID = @intContentsID
					   AND DifficultyCode = COALESCE(@inyDifficultyCode, DifficultyCode)
					   AND CAST(StageEndDate AS date) IS NOT NULL
					 GROUP BY CAST(StageEndDate AS date), CharacterLevel
				), T1 AS (
					SELECT T.CharacterLevel, T0.LogDate, T0.CharacterCount
					FROM (SELECT number AS [CharacterLevel] FROM master.dbo.spt_values WITH (NOLOCK) WHERE TYPE = ''p'' AND number BETWEEN 1 AND 70) T
						  LEFT OUTER JOIN T0 ON T.CharacterLevel = T0.CharacterLevel
				)
				INSERT #tblTemp (LogDate, CharacterLevel, CharacterCount)
				SELECT LogDate, CharacterLevel, CharacterCount
				FROM T1;
			';

			EXEC sp_executesql @nvcStmt, N'
				  @dtFromDate date
				, @dtToDate date
				, @intContentsID int
				, @inyDifficultyCode tinyint'
				, @dtFromDate = @dtFromDate
				, @dtToDate = @dtToDate
				, @intContentsID = @intContentsID
				, @inyDifficultyCode = @inyDifficultyCode;

			DECLARE @vchLogDate varchar(max);   
			SET @vchLogDate = ''; 

			SELECT @vchLogDate = @vchLogDate + '[' + LogDate + '],'  
			   FROM (SELECT DISTINCT CAST(LogDate AS varchar(10)) AS LogDate FROM #tblTemp WHERE LogDate IS NOT NULL) A 
			ORDER BY LogDate;

			IF LEN(@vchLogDate) < 1
			BEGIN
				SET @intReturnValue = 103331;
				DROP TABLE #tblTemp;
				GOTO ErrorHandler;
			END

			SET @vchLogDate = LEFT(@vchLogDate, LEN(@vchLogDate) - 1);   

			EXEC ('SELECT CharacterLevel, ' + @vchLogDate + ' FROM #tblTemp  
					PIVOT ( SUM(CharacterCount) FOR LogDate IN (' + @vchLogDate + ')  
					) AS PVT');

			DROP TABLE #tblTemp;

		END

	END
	-- ??? ?? : Map
	ELSE IF @inyContentsType = 2
	BEGIN
		/** ????? ??? ?? by ??? at 2012-02-07 **/
		EXEC dbo.P_MakeTableString
			  @nvcDBName = @nvcDBName
			, @nvcTblName = @nvcTblName2
			, @nvcStartDate = @nvcStartDate
			, @nvcEndDate = @nvcEndDate
			, @nvcHint = @nvcHint
			, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
		
		IF @intVersion = 1 OR @intVersion IS NULL
		BEGIN
			SET @nvcStmt = N'
			WITH T1 AS (
				SELECT CharacterLevel, COUNT(' + CASE @inyCountType WHEN 1 THEN N'DISTINCT CharacterID' ELSE N'*' END + N') AS CharacterCount
				  FROM (' + @nvcStmtOUT + N') V
				 WHERE ExitDate >= CONVERT(datetime2(0), @dtFromDate)
				   AND ExitDate < CONVERT(datetime2(0), @dtToDate)
				   AND MapID = @intContentsID
				 GROUP BY CharacterLevel
			)
			SELECT T.CharacterLevel, ISNULL(T1.CharacterCount, 0) AS CharacterCount
			  FROM (SELECT number AS [CharacterLevel] FROM master.dbo.spt_values WITH (NOLOCK) WHERE TYPE = ''p'' AND number BETWEEN 1 AND 70) T
				   LEFT OUTER JOIN T1 ON (T.CharacterLevel = T1.CharacterLevel)
			 ORDER BY T.CharacterLevel;
			';

			EXEC sp_executesql @nvcStmt, N'
			  @dtFromDate date
			, @dtToDate date
			, @intContentsID int'
			, @dtFromDate = @dtFromDate
			, @dtToDate = @dtToDate
			, @intContentsID = @intContentsID;
		END
		ELSE
		BEGIN			
			IF OBJECT_ID(N'tempdb..#tblTemp2', N'U') IS NOT NULL
				DROP TABLE #tblTemp2;

			CREATE TABLE #tblTemp2 (
				LogDate date NULL,
				CharacterLevel int NULL,
				CharacterCount int NULL
			);

			SET @nvcStmt = N'
				WITH T0 AS (	
					SELECT CAST( ExitDate AS date) AS LogDate, CharacterLevel, COUNT(' + CASE @inyCountType WHEN 1 THEN N'DISTINCT CharacterID' ELSE N'*' END + N') AS CharacterCount
					  FROM (' + @nvcStmtOUT + N') V
					 WHERE ExitDate >= CONVERT(datetime2(0), @dtFromDate)
					   AND ExitDate < CONVERT(datetime2(0), @dtToDate)
					   AND MapID = @intContentsID
					 GROUP BY CAST( ExitDate AS date), CharacterLevel
				), T1 AS (
					SELECT T.CharacterLevel, T0.LogDate, T0.CharacterCount
					FROM (SELECT number AS [CharacterLevel] FROM master.dbo.spt_values WITH (NOLOCK) WHERE TYPE = ''p'' AND number BETWEEN 1 AND 70) T
						  LEFT OUTER JOIN T0 ON T.CharacterLevel = T0.CharacterLevel
				)
				INSERT #tblTemp2 (LogDate, CharacterLevel, CharacterCount)
				SELECT LogDate, CharacterLevel, CharacterCount
				FROM T1;
			';

			EXEC sp_executesql @nvcStmt, N'
				  @dtFromDate date
				, @dtToDate date
				, @intContentsID int'
				, @dtFromDate = @dtFromDate
				, @dtToDate = @dtToDate
				, @intContentsID = @intContentsID;

			DECLARE @vchLogDate2 varchar(max);   
			SET @vchLogDate2 = ''; 

			SELECT @vchLogDate2 = @vchLogDate2 + '[' + LogDate + '],'  
			   FROM (SELECT DISTINCT CAST(LogDate AS varchar(10)) AS LogDate FROM #tblTemp2 WHERE LogDate IS NOT NULL) A 
			ORDER BY LogDate;

			IF LEN(@vchLogDate2) < 1
			BEGIN
				SET @intReturnValue = 103331;
				DROP TABLE #tblTemp2;
				GOTO ErrorHandler;
			END

			SET @vchLogDate2 = LEFT(@vchLogDate2, LEN(@vchLogDate2) - 1);   

			EXEC ('SELECT CharacterLevel, ' + @vchLogDate2 + ' FROM #tblTemp2  
					PIVOT ( SUM(CharacterCount) FOR LogDate IN (' + @vchLogDate2 + ')  
					) AS PVT');

			DROP TABLE #tblTemp2;			
		END

	END
	-- ??? ?? : Mission
	ELSE IF @inyContentsType = 3
	BEGIN
		/** ????? ??? ?? by ??? at 2012-02-07 **/
		EXEC dbo.P_MakeTableString
			  @nvcDBName = @nvcDBName
			, @nvcTblName = @nvcTblName3
			, @nvcStartDate = @nvcStartDate
			, @nvcEndDate = @nvcEndDate
			, @nvcHint = @nvcHint
			, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
			
		IF @intVersion = 1 OR @intVersion IS NULL
		BEGIN
			SET @nvcStmt = N'
			WITH T1 AS (
				SELECT DS.CharacterLevel, COUNT(' + CASE @inyCountType WHEN 1 THEN N'DISTINCT CharacterID' ELSE N'*' END + N') AS CharacterCount
				  FROM (' + @nvcStmtOUT + N') ML
					   INNER JOIN dbo.DailyCharacterStatus DS WITH (NOLOCK) ON (CONVERT(date, ML.AchieveDate) = DS.BasisDate AND ML.CharacterID = DS.CharacterID)
				 WHERE ML.AchieveDate >= CONVERT(datetime2(0), @dtFromDate)
				   AND ML.AchieveDate < CONVERT(datetime2(0), @dtToDate)
				   AND ML.MissionID = @intContentsID
				 GROUP BY DS.CharacterLevel
			)
			SELECT T.CharacterLevel, ISNULL(T1.CharacterCount, 0) AS CharacterCount
			  FROM (SELECT number AS [CharacterLevel] FROM master.dbo.spt_values WITH (NOLOCK) WHERE TYPE = ''p'' AND number BETWEEN 1 AND 70) T
				   LEFT OUTER JOIN T1 ON (T.CharacterLevel = T1.CharacterLevel)
			 ORDER BY T.CharacterLevel;
			';
			
			EXEC sp_executesql @nvcStmt, N'
			  @dtFromDate date
			, @dtToDate date
			, @intContentsID int'
			, @dtFromDate = @dtFromDate
			, @dtToDate = @dtToDate
			, @intContentsID = @intContentsID;
		END
		ELSE
		BEGIN
			IF OBJECT_ID(N'tempdb..#tblTemp3', N'U') IS NOT NULL
				DROP TABLE #tblTemp3;

			CREATE TABLE #tblTemp3 (
				LogDate date NULL,
				CharacterLevel int NULL,
				CharacterCount int NULL
			);


			SET @nvcStmt = N'
			WITH T0 AS (	
					SELECT CAST(ML.AchieveDate AS date) AS LogDate, DS.CharacterLevel, COUNT(' + CASE @inyCountType WHEN 1 THEN N'DISTINCT ML.CharacterID' ELSE N'*' END + N') AS CharacterCount
					  FROM (' + @nvcStmtOUT + N') ML
							INNER JOIN dbo.DailyCharacterStatus DS WITH (NOLOCK) ON (CONVERT(date, ML.AchieveDate) = DS.BasisDate AND ML.CharacterID = DS.CharacterID)
					 WHERE ML.AchieveDate >= CONVERT(datetime2(0), @dtFromDate)
					   AND ML.AchieveDate < CONVERT(datetime2(0), @dtToDate)
					   AND ML.MissionID = @intContentsID
					 GROUP BY CAST( ML.AchieveDate AS date), DS.CharacterLevel
			), T1 AS (
				SELECT T.CharacterLevel, T0.LogDate, T0.CharacterCount
				FROM (SELECT number AS [CharacterLevel] FROM master.dbo.spt_values WITH (NOLOCK) WHERE TYPE = ''p'' AND number BETWEEN 1 AND 70) T
					  LEFT OUTER JOIN T0 ON T.CharacterLevel = T0.CharacterLevel
			)
			INSERT #tblTemp3 (LogDate, CharacterLevel, CharacterCount)
			SELECT LogDate, CharacterLevel, CharacterCount
			FROM T1;
			';

			EXEC sp_executesql @nvcStmt, N'
			  @dtFromDate date
			, @dtToDate date
			, @intContentsID int'
			, @dtFromDate = @dtFromDate
			, @dtToDate = @dtToDate
			, @intContentsID = @intContentsID;

			DECLARE @vchLogDate3 varchar(max);   
			SET @vchLogDate3 = ''; 

			SELECT @vchLogDate3 = @vchLogDate3 + '[' + LogDate + '],'  
			   FROM (SELECT DISTINCT CAST(LogDate AS varchar(10)) AS LogDate FROM #tblTemp3 WHERE LogDate IS NOT NULL) A 
			ORDER BY LogDate;

			IF LEN(@vchLogDate3) < 1
			BEGIN
				SET @intReturnValue = 103331;
				DROP TABLE #tblTemp3;
				GOTO ErrorHandler;
			END

			SET @vchLogDate3 = LEFT(@vchLogDate3, LEN(@vchLogDate3) - 1);   

			EXEC ('SELECT CharacterLevel, ' + @vchLogDate3 + ' FROM #tblTemp3  
					PIVOT ( SUM(CharacterCount) FOR LogDate IN (' + @vchLogDate3 + ')  
					) AS PVT');

			DROP TABLE #tblTemp3;	
		END

	END

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
