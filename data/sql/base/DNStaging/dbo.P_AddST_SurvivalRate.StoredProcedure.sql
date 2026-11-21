USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_SurvivalRate]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-04-20

description :
	?????? ?????.
		1. ?? ?? ???
			1. ?? ?? ????? ?? ?? ??? ?? ??? ?? ??
			2. ?? ?? ????? ??? ?? 2?? ????? ?? ?? ??? ?? ??? ?? ?? (??? ?? 2? ? ??? ?? ?? ??)
		2. ?? ?? ???
			1. ?? ?? ????? ?? ?? ??? ?? ??? ?? ?? (?? 1?? ????)
			2. ?? ?? ????? ??? ?? ????? ?? ?? ??? ?? ??? ?? ?? (?? 1?? ????)
			3. ?? ?? ????? ??, ????? ?? ?? ??? ?? ??? ?? ?? (?? 1?? ????)
			4. ?? ?? ????? ?? ??? ???, ?? ????? ?? ?? ??? ?? ??? ?? ?? (?? 1?? ????)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1 = ?? ?? (??? DailyETLJob ?? Script ? ?? ?? ??? SP ? ??/??/????)
	2 = Tuning. OPTION (MAXDOP 1, HASH JOIN) ??     by 20110601.wgkim
	4 = ?? ??? ???? ????? ?? by ??? at 2013-03-19
**/

CREATE PROCEDURE [dbo].[P_AddST_SurvivalRate]
	  @dtmBasisDate datetime --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int, @dtBasisDate date;

SET @dtBasisDate = CONVERT(DATE, @dtmBasisDate);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF DATEPART(WEEKDAY, @dtmBasisDate) = 7
	BEGIN
		-- 1. ?? ?? ???
		-- 1. ?? ?? ????? ?? ?? ??? ?? ??? ?? ??
		INSERT dbo.ST_SurvivalRate (MainCategory, SubCategory, BasisDate, JobCode, CharacterLevel, Cnt)
		SELECT 1, 1, @dtBasisDate, T.JobCode, T.CharacterLevel, COUNT(*)
		  FROM (SELECT ROW_NUMBER() OVER(PARTITION BY DUVA.AccountID ORDER BY DCS.CharacterLevel DESC, DC.CreateDate) AS SeqNo
					 , DUVA.AccountID
					 , DCS.JobCode
					 , DCS.CharacterLevel
				  FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
					   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (DUVA.AccountID = DC.AccountID)
					   INNER JOIN dbo.DailyCharacterStatus DCS WITH (NOLOCK) ON (DCS.BasisDate = @dtBasisDate AND DC.CharacterID = DCS.CharacterID)
				 WHERE DUVA.BasisDate >= DATEADD(DAY, -6, @dtBasisDate)
				   AND DUVA.BasisDate <= @dtBasisDate
                   AND DC.AccountLevelCode = 0) T
		 WHERE T.SeqNo = 1
		 GROUP BY T.JobCode, T.CharacterLevel;
		-- 1. ?? ?? ???
		-- 2. ?? ?? ????? ??? ?? 2?? ????? ?? ?? ??? ?? ??? ?? ?? (??? ?? 2? ? ??? ?? ?? ??)
		INSERT dbo.ST_SurvivalRate (MainCategory, SubCategory, BasisDate, JobCode, CharacterLevel, Cnt)
		SELECT 1, 2, DATEADD(DAY, -14, @dtBasisDate), T.JobCode, T.CharacterLevel, COUNT(*)
		  FROM (SELECT ROW_NUMBER() OVER(PARTITION BY DUVA.AccountID ORDER BY DCS.CharacterLevel DESC, DC.CreateDate) AS SeqNo
					 , DUVA.AccountID
					 , DCS.JobCode
					 , DCS.CharacterLevel
				  FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
					   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (DUVA.AccountID = DC.AccountID)
					   INNER JOIN dbo.DailyCharacterStatus DCS WITH (NOLOCK) ON (DCS.BasisDate = @dtBasisDate AND DC.CharacterID = DCS.CharacterID)
				 WHERE DUVA.BasisDate >= DATEADD(DAY, -20, @dtBasisDate)
				   AND DUVA.BasisDate <= DATEADD(DAY, -14, @dtBasisDate)
				   AND DC.AccountLevelCode = 0
				   AND NOT EXISTS (SELECT * FROM dbo.DailyUVAccounts DUVA2 WITH (NOLOCK) WHERE DUVA2.BasisDate >= DATEADD(DAY, -13, @dtBasisDate) AND DUVA2.BasisDate <= @dtBasisDate AND DUVA.AccountID = DUVA2.AccountID)
				   ) T
		 WHERE T.SeqNo = 1
		 GROUP BY T.JobCode, T.CharacterLevel;
		 
	END
	IF @dtmBasisDate = DATEADD(DD, -1, DATEADD(MM, DATEDIFF(M, 0, @dtmBasisDate) + 1, 0))
	BEGIN

		-- 2. ?? ?? ???
		-- 1. ?? ?? ????? ?? ?? ??? ?? ??? ?? ?? (?? 1?? ????)
		INSERT dbo.ST_SurvivalRate (MainCategory, SubCategory, BasisDate, JobCode, CharacterLevel, Cnt)
		SELECT 2, 1, DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0), T.JobCode, T.CharacterLevel, COUNT(*)
		  FROM (SELECT ROW_NUMBER() OVER(PARTITION BY DUVA.AccountID ORDER BY DCS.CharacterLevel DESC, DC.CreateDate) AS SeqNo
					 , DUVA.AccountID
					 , DCS.JobCode
					 , DCS.CharacterLevel
				  FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
					   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (DUVA.AccountID = DC.AccountID)
					   INNER JOIN dbo.DailyCharacterStatus DCS WITH (NOLOCK) ON (DCS.BasisDate = @dtBasisDate AND DC.CharacterID = DCS.CharacterID)
				 WHERE DUVA.BasisDate >= DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0)
				   AND DUVA.BasisDate <= @dtmBasisDate
				   AND DC.AccountLevelCode = 0) T
		 WHERE T.SeqNo = 1
		 GROUP BY T.JobCode, T.CharacterLevel
		OPTION (MAXDOP 1, HASH JOIN);
		-- 2. ?? ?? ???
		-- 2. ?? ?? ????? ??? ?? ????? ?? ?? ??? ?? ??? ?? ?? (?? 1?? ????)
		INSERT dbo.ST_SurvivalRate (MainCategory, SubCategory, BasisDate, JobCode, CharacterLevel, Cnt)
		SELECT 2, 2, DATEADD(MM, DATEDIFF(MM, 0, DATEADD(MONTH, -1, @dtmBasisDate)), 0), T.JobCode, T.CharacterLevel, COUNT(*)
		  FROM (SELECT ROW_NUMBER() OVER(PARTITION BY DUVA.AccountID ORDER BY DCS.CharacterLevel DESC, DC.CreateDate) AS SeqNo
					 , DUVA.AccountID
					 , DCS.JobCode
					 , DCS.CharacterLevel
				  FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
					   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (DUVA.AccountID = DC.AccountID)
					   INNER JOIN dbo.DailyCharacterStatus DCS WITH (NOLOCK) ON (DCS.BasisDate = @dtBasisDate AND DC.CharacterID = DCS.CharacterID)
				 WHERE DUVA.BasisDate >= DATEADD(MM, DATEDIFF(MM, 0, DATEADD(MONTH, -1, @dtmBasisDate)), 0)
				   AND DUVA.BasisDate < DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0)
				   AND DC.AccountLevelCode = 0
				   AND NOT EXISTS (SELECT *
									 FROM dbo.DailyUVAccounts DUVA2 WITH (NOLOCK)
								    WHERE DUVA2.BasisDate >= DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0)
									  AND DUVA2.BasisDate <= @dtmBasisDate
									  AND DUVA.AccountID = DUVA2.AccountID)) T
		 WHERE T.SeqNo = 1
		 GROUP BY T.JobCode, T.CharacterLevel
		OPTION (MAXDOP 1, HASH JOIN);
		-- 2. ?? ?? ???
		-- 3. ?? ?? ????? ??, ????? ?? ?? ??? ?? ??? ?? ?? (?? 1?? ????)
		INSERT dbo.ST_SurvivalRate (MainCategory, SubCategory, BasisDate, JobCode, CharacterLevel, Cnt)
		SELECT 2, 3, DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0), T.JobCode, T.CharacterLevel, COUNT(*)
		  FROM (SELECT ROW_NUMBER() OVER(PARTITION BY DUVA.AccountID ORDER BY DCS.CharacterLevel DESC, DC.CreateDate) AS SeqNo
					 , DUVA.AccountID
					 , DCS.JobCode
					 , DCS.CharacterLevel
				  FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
					   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (DUVA.AccountID = DC.AccountID)
					   INNER JOIN dbo.DailyCharacterStatus DCS WITH (NOLOCK) ON (DCS.BasisDate = @dtBasisDate AND DC.CharacterID = DCS.CharacterID)
				 WHERE DUVA.BasisDate >= DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0)
				   AND DUVA.BasisDate <= @dtmBasisDate
				   AND DC.AccountLevelCode = 0
				   AND EXISTS (SELECT *
								 FROM dbo.PurchaseOrders PO WITH (NOLOCK)
							    WHERE OrderDate >= DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0)
								  AND OrderDate < DATEADD(DD, 1, @dtmBasisDate)
								  AND DC.CharacterID = PO.CharacterID
								  AND PO.PayMethodCode = 2
								  AND PO.OrderStatusCode = 2)) T
		 WHERE T.SeqNo = 1
		 GROUP BY T.JobCode, T.CharacterLevel
		OPTION (MAXDOP 1, HASH JOIN);
		-- 2. ?? ?? ???
		-- 4. ?? ?? ????? ?? ??? ???, ?? ????? ?? ?? ??? ?? ??? ?? ?? (?? 1?? ????)
		INSERT dbo.ST_SurvivalRate (MainCategory, SubCategory, BasisDate, JobCode, CharacterLevel, Cnt)
		SELECT 2, 4, DATEADD(MM, DATEDIFF(MM, 0, DATEADD(MONTH, -1, @dtmBasisDate)), 0), T.JobCode, T.CharacterLevel, COUNT(*)
		  FROM (SELECT ROW_NUMBER() OVER(PARTITION BY DUVA.AccountID ORDER BY DCS.CharacterLevel DESC, DC.CreateDate) AS SeqNo
					 , DUVA.AccountID
					 , DCS.JobCode
					 , DCS.CharacterLevel
				  FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
					   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (DUVA.AccountID = DC.AccountID)
					   INNER JOIN dbo.DailyCharacterStatus DCS WITH (NOLOCK) ON (DCS.BasisDate = @dtBasisDate AND DC.CharacterID = DCS.CharacterID)
				 WHERE DUVA.BasisDate >= DATEADD(MM, DATEDIFF(MM, 0, DATEADD(MONTH, -1, @dtmBasisDate)), 0)
				   AND DUVA.BasisDate < DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0)
				   AND DC.AccountLevelCode = 0
				   AND EXISTS (SELECT *
								 FROM dbo.PurchaseOrders PO WITH (NOLOCK)
							    WHERE OrderDate >= DATEADD(MM, DATEDIFF(MM, 0, DATEADD(MONTH, -1, @dtmBasisDate)), 0)
								  AND OrderDate < DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0)
								  AND DC.CharacterID = PO.CharacterID
								  AND PO.PayMethodCode = 2
								  AND PO.OrderStatusCode = 2)
				   AND NOT EXISTS (SELECT *
									 FROM dbo.DailyUVAccounts DUVA2 WITH (NOLOCK)
								    WHERE DUVA2.BasisDate >= DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0)
									  AND DUVA2.BasisDate <= @dtmBasisDate
									  AND DUVA.AccountID = DUVA2.AccountID)) T
		 WHERE T.SeqNo = 1
		 GROUP BY T.JobCode, T.CharacterLevel
		OPTION (MAXDOP 1, HASH JOIN);

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
