USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyLevelSurvival]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2012-10-15
description : ??? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ?? ?? by ??? at 2012-10-15
2 = 6?? ??? ?? by ??? at 2012-10-18
4 = CreateDate ?? ?? by ??? at 2012-11-29
5 = NationalityCode ?? ?? by ??? at 2012-12-27
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyLevelSurvival]
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
      
	WHILE (@inySeqNo <= @inyMaxSeqNo)
	BEGIN
		SELECT @inyNationalityCode = NationalityCode
        FROM @Temp_NationalityCode
        WHERE SeqNo = @inySeqNo;
         	
		/**_# ?? ??? ?? ??? ?????.*/
		IF OBJECT_ID('tempdb..#Acc') IS NOT NULL
			DROP TABLE #Acc;
		SELECT AccountID, MIN(CreateDate) AS CreateDate, MAX(LastLoginDate) as LastLoginDate, MAX(CharacterLevel) as CharacterLevel
		INTO #Acc
		FROM dbo.DimCharacters (NOLOCK)
		WHERE AccountLevelCode = 0
		GROUP BY AccountID
		HAVING MIN(CreateDate) >= DATEADD(MM, -6, CAST(CONVERT(VARCHAR(6), @dtmBasisDate, 112)+ '01' AS datetime))
		OPTION (MAXDOP 1);

		/**_# [ST_DailyLevelSurvival] ???? INSERT???.*/
		;WITH C1 AS (
			SELECT a.CharacterLevel
				, CAST(CONVERT(VARCHAR(6), a.CreateDate, 112)+ '01' AS date) AS CreateMonth
				, COUNT(DISTINCT a.AccountID) AS RU
				, SUM(CASE WHEN b.BasisDate >= DATEADD(DD, -1, @dtmBasisDate) THEN 1 ELSE 0 END) AS UVDay
				, SUM(CASE WHEN b.BasisDate >= DATEADD(DD, -6, @dtmBasisDate) THEN 1 ELSE 0 END) AS UVWeek
				, SUM(CASE WHEN b.BasisDate >= DATEADD(DD, -29, @dtmBasisDate) THEN 1 ELSE 0 END) AS UVMonth
			FROM #Acc a LEFT OUTER JOIN dbo.DailyUVAccounts b (NOLOCK) ON b.BasisDate > DATEADD(MM, -1, @dtmBasisDate) AND b.AccountID = a.AccountID
			GROUP BY a.CharacterLevel, CAST(CONVERT(VARCHAR(6), a.CreateDate, 112)+ '01' AS date)
		), C2 AS (
			SELECT a.CharacterLevel, CAST(CONVERT(VARCHAR(6), a.CreateDate, 112)+ '01' AS date) AS CreateMonth
				, COUNT(DISTINCT b.AccountID) AS RevisitCount
			FROM #Acc a INNER JOIN dbo.DailyUVAccounts b (NOLOCK) ON b.AccountID = a.AccountID
			WHERE b.BasisDate >= DATEADD(DD, -1, CAST(@dtmBasisDate AS date))
			GROUP BY a.CharacterLevel, CAST(CONVERT(VARCHAR(6), a.CreateDate, 112)+ '01' AS date)
			HAVING COUNT(*)>1
		)
		INSERT dbo.ST_DailyLevelSurvival (BasisDate, CharacterLevel, CreateMonth, RU, UVDay, UVWeek, UVMonth, RevisitCount, NationalityCode)
		SELECT @dtmBasisDate AS BasisDate, C1.CharacterLevel, C1.CreateMonth, C1.RU, C1.UVDay, C1.UVWeek, C1.UVMonth, C2.RevisitCount, @inyNationalityCode
		FROM C1 LEFT OUTER JOIN C2 ON C2.CharacterLevel = C1. CharacterLevel AND C2.CreateMonth = C1.CreateMonth OPTION (MAXDOP 1);
       
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
