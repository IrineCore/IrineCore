USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListLeavePreventionConnInfo]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 19
author : ???
e-mail : wgkim@eyedentitygames
created date : 2010-12-23
description : ???? ??? ?? ?? ??? ?????.
result set : 
	AccountID, PCRoomLoginDateCount, HomeLoginDateCount, LoginDateCount, AveragePlayTime, TotalSessionCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
8 = ??? alias ?/??? ?? ?? at 2011-01-05
9 = SET ANSI_WARNINGS OFF ?? at 2011-01-22
history
11~13 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
15~16 = @dtmBasisDate ?? ? GETDATE() ??     by ??? at 2012-05-21
**/

CREATE PROCEDURE [dbo].[P_GetListLeavePreventionConnInfo]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET ANSI_WARNINGS OFF;

DECLARE @intReturnValue int
	  , @chrLastDate	CHAR(8)
	  , @chrDate		CHAR(8)
	  , @intCount		INT
	  , @vchSQL			VARCHAR(MAX);

DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName1 nvarchar(50) = N'AccountLogoutLogs'
	, @nvcTblName2 nvarchar(50) = N'CharacterLoginLogs'
	, @nvcStartDate nvarchar(8) --= CONVERT(nvarchar(8), DATEADD(D, -7, GETDATE()), 112)
	, @nvcEndDate nvarchar(8) --= CONVERT(nvarchar(8), DATEADD(D, -1, GETDATE()), 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT1 nvarchar(max) = N''
	, @nvcStmtOUT2 nvarchar(max) = N''
	, @dtmBasisDate datetime;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT @dtmBasisDate = DATEADD(DAY, 1, MAX(BasisDate)) FROM dbo.ETLLogs WITH (NOLOCK);

	IF @@ROWCOUNT = 0 SET @dtmBasisDate = GETDATE();

	SET @nvcStartDate = CONVERT(nvarchar(8), DATEADD(D, -7, @dtmBasisDate), 112);
	SET @nvcEndDate = CONVERT(nvarchar(8), DATEADD(D, -1, @dtmBasisDate), 112);
	  
	SELECT @chrLastDate = CONVERT(CHAR(8), DATEADD(DAY, -7, @dtmBasisDate), 112)
		 , @chrDate		= ''
		 , @intCount	= 1
		 , @vchSQL		= '';

	/** ????? ??? ?? by ??? at 2012-02-07 **/
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
		
	SET @nvcStmt = N'
	SELECT aa.AccountID														AS AccountID            -- ??ID
		 , ISNULL(COUNT(DISTINCT CONVERT(CHAR(8), cc.LoginDate, 112)), 0)	AS PCRoomLoginDateCount -- ?? ??? PC? ?? ? ?
		 , ISNULL(COUNT(DISTINCT CONVERT(CHAR(8), dd.LoginDate, 112)), 0)	AS HomeLoginDateCount   -- ?? ??? ? ?? ? ?
		 , aa.LoginDateCount												AS LoginDateCount       -- ?? ??? ?? ? ?
		 , aa.AveragePlayTime												AS AveragePlayTime      -- ?? ??? ?? Play Time (?)
		 , aa.TotalSessionCount												AS TotalSessionCount    -- ?? ??? ?? Session Count
	  FROM (
			SELECT a.AccountID				AS AccountID
				 , COUNT(a.LoginDate)		AS LoginDateCount
				 , AVG(a.PlayTime)			AS AveragePlayTime
				 , SUM(a.SessionCount)		AS TotalSessionCount
			  FROM (
					SELECT AccountID									AS AccountID
						 , CONVERT(CHAR(8), LoginDate, 112)				AS LoginDate
						 , SUM(DATEDIFF(MI, LoginDate, LogoutDate))		AS PlayTime
						 , COUNT(SessionID)								AS SessionCount
					  FROM (' + @nvcStmtOUT1 + N') L
					 WHERE LogoutDate >= CONVERT(DATETIME, CONVERT(CHAR(8), DATEADD(D, -7, @dtmBasisDate)), 112)
					 GROUP BY AccountID, CONVERT(CHAR(8), LoginDate, 112)
				   ) a
			 GROUP BY a.AccountID
		   ) aa
		   INNER JOIN dbo.DimCharacters bb WITH (NOLOCK) ON (aa.AccountID = bb.AccountID)
		   LEFT OUTER JOIN (' + @nvcStmtOUT2 + N') cc ON cc.LoginDate >= CONVERT(datetime2, CONVERT(CHAR(8), DATEADD(D, -7, GETDATE()), 112))
													 AND bb.CharacterID = cc.CharacterID AND cc.PCRoomFlag = CONVERT(BIT, 1)
		   LEFT OUTER JOIN (' + @nvcStmtOUT2 + N') dd ON dd.LoginDate >= CONVERT(datetime2, CONVERT(CHAR(8), DATEADD(D, -7, GETDATE()), 112))
													 AND bb.CharacterID = dd.CharacterID AND dd.PCRoomFlag = CONVERT(BIT, 0)
	 GROUP BY aa.AccountID, aa.LoginDateCount, aa.AveragePlayTime, aa.TotalSessionCount;
	'

	IF @nvcStmtOUT1 = N'' OR @nvcStmtOUT1 IS NULL
		SELECT 0 AS AccountID, 0 AS PCRoomLoginDateCount, 0 AS HomeLoginDateCount, 0 AS LoginDateCount, 0 AS AveragePlayTime, 0 AS TotalSessionCount
		WHERE 1 < 1
	ELSE
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
