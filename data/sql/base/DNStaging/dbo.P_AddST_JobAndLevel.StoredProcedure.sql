USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_JobAndLevel]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-23
description : ???? ??, ?? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = ST_JobAndLevelType6 ???? ? ?? ?? at 2010-08-09
7~9 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
12 = ?? ??? ???? ????? ?? by ??? at 2013-03-19
**/

CREATE PROCEDURE [dbo].[P_AddST_JobAndLevel]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CharacterLogoutLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)
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
	SET @dtmBasisDate = CAST(CAST(@dtmBasisDate AS date) AS datetime);
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;

	INSERT dbo.ST_JobAndLevelType1 (BasisDate, WorldID, JobCode, CharacterLevel, CharacterCount)
	SELECT @dtmBasisDate, WorldID, JobCode, CharacterLevel, COUNT(*)
	FROM dbo.DimCharacters
	WHERE DeleteFlag = 0 AND AccountLevelCode = 0
	GROUP BY WorldID, JobCode, CharacterLevel;

	WITH A AS (
		SELECT ROW_NUMBER() OVER(PARTITION BY AccountID ORDER BY CharacterExp DESC, CreateDate DESC) AS Seq, CharacterID
		FROM dbo.DimCharacters
		WHERE DeleteFlag = 0 AND AccountLevelCode = 0
	), B AS (
		SELECT CharacterID FROM A WHERE Seq = 1
	)
	INSERT dbo.ST_JobAndLevelType2 (BasisDate, WorldID, JobCode, CharacterLevel, CharacterCount)
	SELECT @dtmBasisDate, CH.WorldID, CH.JobCode, CH.CharacterLevel, COUNT(*)
	FROM dbo.DimCharacters CH
		INNER JOIN B ON CH.CharacterID = B.CharacterID
	WHERE CH.DeleteFlag = 0 AND CH.AccountLevelCode = 0
	GROUP BY CH.WorldID, CH.JobCode, CH.CharacterLevel;

	SET @nvcStmt = N'
	WITH C1 AS (
		SELECT DISTINCT CharacterID
		FROM (' + @nvcStmtOUT + N') V
		WHERE LogoutDate >= @dtmBasisDate AND LogoutDate < (@dtmBasisDate + 1)
	), C2 AS (
		SELECT ROW_NUMBER() OVER(PARTITION BY D.AccountID ORDER BY DC.CharacterExp DESC, CreateDate DESC) AS Seq, D.WorldID, DC.CharacterLevel, DC.JobCode
		FROM C1
			INNER JOIN dbo.DailyCharacterStatus DC ON C1.CharacterID = DC.CharacterID AND DC.BasisDate = CAST(@dtmBasisDate AS date)
			INNER JOIN dbo.DimCharacters D ON C1.CharacterID = D.CharacterID
		WHERE D.DeleteFlag = 0 AND D.AccountLevelCode = 0
	)
	INSERT dbo.ST_JobAndLevelType3 (BasisDate, WorldID, JobCode, CharacterLevel, CharacterCount)
	SELECT @dtmBasisDate, WorldID, JobCode, CharacterLevel, COUNT(*)
	FROM C2
	WHERE Seq = 1
	GROUP BY WorldID, JobCode, CharacterLevel;
	';
	EXEC sp_executesql @nvcStmt, N'@dtmBasisDate datetime', @dtmBasisDate = @dtmBasisDate;

	INSERT dbo.ST_JobAndLevelType4 (BasisDate, WorldID, JobCode, CharacterLevel, CharacterCount)
	SELECT CAST(@dtmBasisDate AS date), DC.WorldID, C.JobCode, C.CharacterLevel, COUNT(*)
	FROM dbo.DimCharacters DC
		INNER JOIN dbo.DimAccounts DA ON DC.AccountID = DA.AccountID
		INNER JOIN dbo.DailyCharacterStatus C ON DC.CharacterID = C.CharacterID
	WHERE DC.DeleteFlag = 0 AND DA.RegisterDate >= @dtmBasisDate AND DA.RegisterDate < (@dtmBasisDate + 1) AND DC.AccountLevelCode = 0
		AND C.BasisDate = @dtmBasisDate
	GROUP BY CAST(DA.RegisterDate AS date), DC.WorldID, C.JobCode, C.CharacterLevel;

	WITH A AS (
		SELECT ROW_NUMBER() OVER(PARTITION BY A.AccountID ORDER BY B.CharacterLevel DESC, A.CreateDate DESC) AS Seq, B.CharacterID
		FROM dbo.DimCharacters A
			INNER JOIN dbo.DailyCharacterStatus B ON A.CharacterID = B.CharacterID
		WHERE A.DeleteFlag = 0 AND A.AccountLevelCode = 0 AND B.BasisDate = @dtmBasisDate
	), B AS (
		SELECT CharacterID FROM A WHERE Seq = 1
	)
	INSERT dbo.ST_JobAndLevelType5 (BasisDate, WorldID, JobCode, CharacterLevel, CharacterCount)
	SELECT CAST(@dtmBasisDate AS date), DC.WorldID, C.JobCode, C.CharacterLevel, COUNT(*)
	FROM dbo.DimCharacters DC
		INNER JOIN B ON DC.CharacterID = B.CharacterID
		INNER JOIN dbo.DimAccounts DA ON DC.AccountID = DA.AccountID
		INNER JOIN dbo.DailyCharacterStatus C ON B.CharacterID = C.CharacterID
	WHERE DC.DeleteFlag = 0 AND DC.AccountLevelCode = 0 AND DA.RegisterDate >= CAST(@dtmBasisDate AS datetime2(0)) AND DA.RegisterDate < CAST(DATEADD(DAY, 1, @dtmBasisDate) AS datetime2(0)) AND C.BasisDate = @dtmBasisDate
	GROUP BY CAST(DA.RegisterDate AS date), DC.WorldID, C.JobCode, C.CharacterLevel;

	SET @nvcStmt = N'
	WITH C1 AS (
		SELECT DISTINCT CharacterID
		FROM (' + @nvcStmtOUT + N') V
		WHERE LogoutDate >= @dtmBasisDate AND LogoutDate < (@dtmBasisDate + 1)
	)
	INSERT dbo.ST_JobAndLevelType6 (BasisDate, WorldID, JobCode, CharacterLevel, CharacterCount)
	SELECT @dtmBasisDate, D.WorldID, DC.JobCode, DC.CharacterLevel, COUNT(*)
	FROM C1
		INNER JOIN dbo.DailyCharacterStatus DC ON C1.CharacterID = DC.CharacterID AND DC.BasisDate = CAST(@dtmBasisDate AS date)
		INNER JOIN dbo.DimCharacters D ON C1.CharacterID = D.CharacterID
	WHERE D.DeleteFlag = 0 AND D.AccountLevelCode = 0
	GROUP BY D.WorldID, DC.JobCode, DC.CharacterLevel;
	';
	EXEC sp_executesql @nvcStmt, N'@dtmBasisDate datetime', @dtmBasisDate = @dtmBasisDate;
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
