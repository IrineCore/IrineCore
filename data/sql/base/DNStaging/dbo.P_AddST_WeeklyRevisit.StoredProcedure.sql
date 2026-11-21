USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_WeeklyRevisit]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-06
description : ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ??? 12??? 21?? ?? at 2010-06-23
3 = ???? ?? - 20110302.wgkim
4 = ???? null ?? ??, ????? 0? ???? ???? ?? ???? ?? - 20110302.wgkim
5 = NationalityCode ? null ? ??? ?? ?? - 20110304.wgkim
6 = ???? ?? ?? ?? - 20110304.wgkim
7 = ?????? ?? ??? ??? ?? NULL?? ?? by ??? at 2012-12-03
8 = 7??? ?? by ??? at 2012-12-03
**/

CREATE PROCEDURE [dbo].[P_AddST_WeeklyRevisit]
	  @dtmBasisDate datetime --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

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

	IF DATEPART(WEEKDAY, @dtmBasisDate) < 7
		RETURN 0;

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

	    /**_# [ST_WeeklyRevisit] ???? INSERT???.*/
	    WITH UV AS (
		    SELECT WUVA.AccountID
		    FROM dbo.WeeklyUVAccounts WUVA WITH (NOLOCK)
                 INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (WUVA.AccountID = DA.AccountID AND (@inyNationalityCode IS NULL OR DA.NationalityCode = @inyNationalityCode))
		    WHERE WUVA.BasisDate = CAST(@dtmBasisDate - 6 AS date)
	    ), C0 AS (
		    SELECT DATEADD(DAY, - DATEPART(WEEKDAY, BasisDate) + 1, BasisDate) AS BasisDate, SUM(RUCount) AS RUCount
		    FROM dbo.ST_DailyDashboard WITH (NOLOCK)
		    WHERE BasisDate BETWEEN CAST(DATEADD(WEEK, -21, @dtmBasisDate - 6) AS date) AND CAST(@dtmBasisDate AS date) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
		    GROUP BY DATEADD(DAY, - DATEPART(WEEKDAY, BasisDate) + 1, BasisDate)
	    ), C1 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -1, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -1, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C2 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -2, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -2, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C3 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -3, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -3, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C4 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -4, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -4, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C5 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -5, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -5, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C6 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -6, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -6, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C7 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -7, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -7, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C8 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -8, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -8, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C9 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -9, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -9, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C10 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -10, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -10, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C11 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -11, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -11, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C12 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -12, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -12, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C13 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -13, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -13, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C14 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -14, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -14, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C15 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -15, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -15, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C16 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -16, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -16, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C17 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -17, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -17, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C18 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -18, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -18, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C19 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -19, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -19, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C20 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -20, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -20, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    ), C21 AS (
		    SELECT AccountID
		    FROM dbo.DimAccounts WITH (NOLOCK)
		    WHERE RegisterDate >= DATEADD(WEEK, -21, @dtmBasisDate - 6) AND RegisterDate < DATEADD(WEEK, -21, @dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)
	    )
	    INSERT dbo.ST_WeeklyRevisit (AccountRegisterDate, BasisDate, RevisitCount, RevisitRatio, NationalityCode)
	    SELECT DATEADD(WEEK, -1, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -1, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C1 ON UV.AccountID = C1.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -2, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -2, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C2 ON UV.AccountID = C2.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -3, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -3, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C3 ON UV.AccountID = C3.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -4, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -4, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C4 ON UV.AccountID = C4.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -5, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -5, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C5 ON UV.AccountID = C5.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -6, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -6, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C6 ON UV.AccountID = C6.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -7, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -7, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C7 ON UV.AccountID = C7.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -8, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -8, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C8 ON UV.AccountID = C8.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -9, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -9, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C9 ON UV.AccountID = C9.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -10, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -10, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C10 ON UV.AccountID = C10.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -11, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -11, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C11 ON UV.AccountID = C11.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -12, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -12, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C12 ON UV.AccountID = C12.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -13, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -13, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C13 ON UV.AccountID = C13.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -14, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -14, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C14 ON UV.AccountID = C14.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -15, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -15, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C15 ON UV.AccountID = C15.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -16, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -16, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C16 ON UV.AccountID = C16.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -17, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -17, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C17 ON UV.AccountID = C17.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -18, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -18, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C18 ON UV.AccountID = C18.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -19, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -19, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C19 ON UV.AccountID = C19.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -20, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -20, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C20 ON UV.AccountID = C20.AccountID
	    HAVING COUNT(*) > 0
	    UNION ALL
	    SELECT DATEADD(WEEK, -21, @dtmBasisDate - 6), @dtmBasisDate - 6, COUNT(*), CAST(ROUND(COUNT(*) / (SELECT CAST(RUCount AS DECIMAL(12, 2)) FROM C0 WHERE BasisDate = DATEADD(WEEK, -21, @dtmBasisDate - 6)), 6) AS DECIMAL(9, 6)), @inyNationalityCode
	    FROM UV
		    INNER JOIN C21 ON UV.AccountID = C21.AccountID
	    HAVING COUNT(*) > 0

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
