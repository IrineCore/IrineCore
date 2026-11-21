USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_WeeklyUV]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-05-03
description : ?? UV? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ???? by ??? at 2013-05-03
2 = ?? ?? UV? ??? ?? by ??? at 2013-08-06
3~4 = ??? UV ????? ?? by ??? at 2013-08-07
**/

CREATE PROCEDURE [dbo].[P_AddST_WeeklyUV]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF OBJECT_ID('tempdb..#uva') IS NOT NULL DROP TABLE #uva;
	SELECT DISTINCT AccountID
	INTO #uva
	FROM dbo.DailyUVAccounts WITH (NOLOCK)
	WHERE BasisDate >= DATEADD(DD, -6, @datBasisDate) AND BasisDate < DATEADD(DD, 1, @datBasisDate)
	OPTION (MAXDOP 1);
	CREATE UNIQUE CLUSTERED INDEX ix_uva_AccountID ON #uva (AccountID);
	
	/**_# [ST_WeeklyUV] ???? INSERT???.*/
	;WITH acc AS (
		SELECT a.AccountID, MAX(CharacterLevel) AS CharacterLevel
		FROM #uva a INNER JOIN dbo.DimCharacters b WITH (NOLOCK) ON b.AccountID = a.AccountID AND b.DeleteFlag = 0 AND b.AccountLevelCode = 0
		GROUP BY a.AccountID
	)
	INSERT dbo.ST_WeeklyUV (BasisDate, UVCount, CharacterLevel)
	SELECT @datBasisDate AS BasisDate, COUNT(*) AS UVCount, CharacterLevel FROM acc GROUP BY CharacterLevel	OPTION (MAXDOP 1);
	IF OBJECT_ID('tempdb..#uva') IS NOT NULL DROP TABLE #uva;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
