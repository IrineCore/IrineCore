USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddPetalIncomeForEmployee]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_AddPetalIncomeForEmployee]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_AddPetalIncomeForEmployee] AS' 
END
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2013-05-22

description :
	??? ?? ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DLS_AddPetalIncomeForEmployee]
	  @xmlAccountNames xml --// ??? ?? ?? '<root><Account AccountName="" /></root>'
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @tblAccountNames TABLE (AccountName nvarchar(50) PRIMARY KEY, AccountID int);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ?? ???? ?? ID ? ?????. */
	;WITH T AS (
		SELECT T.col.value('@AccountName', 'nvarchar(50)') AS AccountName
		  FROM @xmlAccountNames.nodes('root/Account') T(col)
	)
	INSERT @tblAccountNames (AccountName, AccountID)
	SELECT T.AccountName, A.AccountID
	  FROM T
		   LEFT OUTER JOIN dbo.Accounts A WITH (NOLOCK) ON (T.AccountName = A.AccountName);

	/**_# ??? ?????. */
	;WITH T1 AS (		SELECT T.AccountID, ISNULL(I.PetalAmount, 0) - ISNULL(O.PetalAmount, 0) AS Balance		  FROM @tblAccountNames T			   LEFT OUTER JOIN dbo.VIX_PetalIncome I WITH (NOEXPAND, NOLOCK) ON (T.AccountID = I.AccountID)			   LEFT OUTER JOIN dbo.VIX_PetalOutgo O WITH (NOEXPAND, NOLOCK) ON (T.AccountID = O.AccountID)		 WHERE T.AccountID IS NOT NULL	), T2 AS (		SELECT AccountID, CASE WHEN Balance >= 30000 THEN 0 ELSE 30000 - Balance END AS PetalAmount
		  FROM T1
	)
	SELECT AccountID, PetalAmount
	  INTO #Temp
	  FROM T2
	 WHERE PetalAmount > 0;

	INSERT dbo.PetalIncome (AccountID, PetalAmount, IncomeDate, PetalIncomeCode)
	SELECT AccountID, PetalAmount, GETDATE(), 3 FROM #Temp;

	/**_# ?? ?? ??? ?????. */
	SELECT T.AccountName, T.AccountID, ISNULL(I.PetalAmount, 0) AS PetalAmount
	  FROM @tblAccountNames T		   LEFT OUTER JOIN #Temp I ON (T.AccountID = I.AccountID)
	 ORDER BY T.AccountName;

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
