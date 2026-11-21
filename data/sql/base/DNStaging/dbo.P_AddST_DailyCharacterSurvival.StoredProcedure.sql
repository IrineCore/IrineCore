USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyCharacterSurvival]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-06-28
description : ??? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ?? ?? by ??? at 2013-06-28
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyCharacterSurvival]
	  @dtmBasisDate datetime  --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @Temp_NationalityCode TABLE
( SeqNo             tinyint identity(1, 1)
, NationalityCode   tinyint NULL
);

DECLARE @intReturnValue int
      , @inyNationalityCode tinyint
      , @inySeqNo tinyint
      , @inyMaxSeqNo tinyint;

DECLARE @nvcStmt nvarchar(max)
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CharacterLoginLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DD, -1, @dtmBasisDate), 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

SET @inySeqNo = 1;
SET @inyMaxSeqNo = 1;
      
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

    /**_# ????? ?????? ????. */
    INSERT @Temp_NationalityCode (NationalityCode)
    SELECT DISTINCT NationalityCode
      FROM dbo.DimAccounts WITH (NOLOCK);

    IF @@ROWCOUNT > 1
    BEGIN
        /***_# ????? ?? ?? ???? NULL ? ????? ???? ??? ?????. */
        IF EXISTS (SELECT * FROM @Temp_NationalityCode WHERE NationalityCode IS NULL)
        BEGIN
            RAISERROR(N'Error!! Invalid data : NationalityCode is NULL', 11, 1);
        END
    END

    /***_# ????? 0 ? ???? ???? ??? ?????. */
    IF EXISTS (SELECT * FROM @Temp_NationalityCode WHERE NationalityCode = 0)
    BEGIN
        RAISERROR(N'Error!! Invalid data : NationalityCode is 0', 11, 1);
    END

    SELECT @inyMaxSeqNo = MAX(SeqNo)
      FROM @Temp_NationalityCode;

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
      
	WHILE (@inySeqNo <= @inyMaxSeqNo)
	BEGIN
		SELECT @inyNationalityCode = NationalityCode
        FROM @Temp_NationalityCode
        WHERE SeqNo = @inySeqNo;
         	
		SET @nvcStmt = N'
		/**_# ?? ??? ?? ??? ?????.*/
			IF OBJECT_ID(''tempdb..#Cha'') IS NOT NULL
				DROP TABLE #Cha;
			SELECT CharacterID, WorldID, CharacterLevel, JobCode
				, CAST(CONVERT(VARCHAR(6), CreateDate, 112)+ ''01'' AS date) AS CreateMonth
				, CASE WHEN LastLoginDate >= DATEADD(DD, -1, @dtmBasisDate) THEN 1 ELSE 0 END AS UniqueCharacterDay
				, CASE WHEN LastLoginDate >= DATEADD(DD, -6, @dtmBasisDate) THEN 1 ELSE 0 END AS UniqueCharacterWeek
				, CASE WHEN LastLoginDate >= DATEADD(DD, -29, @dtmBasisDate) THEN 1 ELSE 0 END AS UniqueCharacterMonth
				, ROW_NUMBER() OVER (PARTITION BY AccountID, WorldID ORDER BY CreateDate) AS rn
			INTO #Cha
			FROM dbo.DimCharacters (NOLOCK)
			WHERE CreateDate >= DATEADD(MM, -6, CAST(CONVERT(VARCHAR(6), @dtmBasisDate, 112)+ ''01'' AS datetime)) AND AccountLevelCode = 0
			OPTION (MAXDOP 1);

			/**_# [ST_DailyCharacterSurvival] ???? INSERT???.*/
			;WITH C1 AS (
				SELECT WorldID, CharacterLevel, JobCode, CreateMonth
					, COUNT(*) AS CharacterCreateCount
					, SUM(UniqueCharacterDay) AS UniqueCharacterDay
					, SUM(UniqueCharacterWeek) AS UniqueCharacterWeek
					, SUM(UniqueCharacterMonth) AS UniqueCharacterMonth
					, CASE WHEN rn = 1 THEN 1 ELSE 0 END AS FirstCreateCharacterFlag
				FROM #Cha
				GROUP BY WorldID, CharacterLevel, JobCode, CreateMonth, CASE WHEN rn = 1 THEN 1 ELSE 0 END
			), C2 AS (
				SELECT CharacterID
				FROM (' + @nvcStmtOUT + N') V
				GROUP BY CharacterID
				HAVING COUNT(DISTINCT CAST(LoginDate AS date)) > 1
			)
			, C3 AS (
				SELECT WorldID, CharacterLevel, JobCode, CreateMonth
					, COUNT(*) AS RevisitCount
				FROM #Cha a INNER JOIN C2 b ON b.CharacterID = a.CharacterID
				GROUP BY WorldID, CharacterLevel, JobCode, CreateMonth
			)
			INSERT dbo.ST_DailyCharacterSurvival (BasisDate, WorldID, CharacterLevel, JobCode, CreateMonth, CharacterCreateCount, UniqueCharacterDay, UniqueCharacterWeek, UniqueCharacterMonth, RevisitCount, NationalityCode, FirstCreateCharacterFlag)
			SELECT @dtmBasisDate AS BasisDate, C1.WorldID, C1.CharacterLevel, C1.JobCode, C1.CreateMonth, C1.CharacterCreateCount, C1.UniqueCharacterDay, C1.UniqueCharacterWeek, C1.UniqueCharacterMonth
				, C3.RevisitCount, @inyNationalityCode, C1.FirstCreateCharacterFlag
			FROM C1 LEFT OUTER JOIN C3 ON C3.WorldID = C1.WorldID AND C3.CharacterLevel = C1.CharacterLevel AND C3.JobCode = C1.JobCode AND C3.CreateMonth = C1.CreateMonth
			OPTION (MAXDOP 1);
		';
		EXEC dbo.sp_executesql @nvcStmt, N'@dtmBasisDate datetime, @inyNationalityCode tinyint', @dtmBasisDate = @dtmBasisDate, @inyNationalityCode = @inyNationalityCode;
       
		SELECT @inySeqNo = @inySeqNo + 1;
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
