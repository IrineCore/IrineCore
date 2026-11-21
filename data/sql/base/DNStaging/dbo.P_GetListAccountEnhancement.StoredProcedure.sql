USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountEnhancement]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-28
description : ??? AccountID? ?? ?? ?? ??? ?????. (??? ??? IP, ?? ??, ?? ??)
result set : AccountID, AccountName, SaleAmount, LoginIP
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history : 
10~12 = ????? VIEW ???? ???? ?? by ??? at 2012-02-06
13 = ?? ??? ???? ???? ????? ?? by ??? at 2013-05-21
**/

CREATE PROCEDURE [dbo].[P_GetListAccountEnhancement]
	  @datBasisDate date --// ?? ?? ??
	, @vchAccountIDs varchar(max) --// ,? ?????? AccountID ??? (?? 500?)
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @nvcStmt nvarchar(max)
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'AccountLoginLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DAY, 1, @datBasisDate), 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;
	
	SET @nvcStmt = N'
	DECLARE @tblAccountIDs table (AccountID int NOT NULL PRIMARY KEY);

	INSERT @tblAccountIDs (AccountID)
	SELECT DISTINCT string
	FROM dbo.IF_Split(@vchAccountIDs, '','');

	SELECT F.AccountID, ISNULL(SUM(ISNULL(PO.PaidCashAmount, PO.Price)), 0) AS SaleAmount
	INTO #tmp001
	FROM @tblAccountIDs F
		INNER JOIN dbo.DimCharacters DC WITH (NOLOCK, FORCESEEK) ON F.AccountID = DC.AccountID
		LEFT OUTER JOIN dbo.PurchaseOrders PO WITH (NOLOCK, FORCESEEK) ON DC.CharacterID = PO.CharacterID AND PO.PayMethodCode = 2 AND PO.OrderStatusCode = 2
	GROUP BY F.AccountID;

	WITH C1 AS (
		SELECT ROW_NUMBER() OVER(PARTITION BY L.AccountID ORDER BY LoginDate DESC) AS Seq
			, T.AccountID, T.SaleAmount, L.LoginIP
		FROM #tmp001 T
			INNER JOIN (' + @nvcStmtOUT + N') L ON T.AccountID = L.AccountID
		WHERE L.LoginDate >= ''' + CONVERT(nvarchar(10), @datBasisDate, 121) + ''' AND L.LoginDate < ''' + CONVERT(nvarchar(10), DATEADD(DAY, 1, @datBasisDate), 121) + '''
	)
	SELECT C1.AccountID, DA.AccountName, C1.SaleAmount, dbo.FN_IPNumber2IP(C1.LoginIP) AS LoginIP
	FROM C1
		INNER JOIN dbo.DimAccounts DA WITH (NOLOCK, FORCESEEK) ON C1.AccountID = DA.AccountID
	WHERE C1.Seq = 1;

	DROP TABLE #tmp001;'

	EXEC sp_executesql @nvcStmt, N'@vchAccountIDs varchar(max)', @vchAccountIDs = @vchAccountIDs;
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
