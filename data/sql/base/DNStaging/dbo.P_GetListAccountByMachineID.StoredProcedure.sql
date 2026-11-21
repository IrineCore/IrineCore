USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountByMachineID]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-27
description : ?? MachineID? ??? Account ??? ?????.
result set : 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104101 = ?? ?? ??? ????.
history
3 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetListAccountByMachineID]
	  @dt2StartLogoutDate datetime2(0) --// (?? ??) ?? ?? ?? - ???? ?? ??
	, @dt2EndLogoutDate datetime2(0) --// (?? ??) ?? ? ?? - ???? ?? ??
	, @binMachineID binary(16) --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'AccountLogoutLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartLogoutDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndLogoutDate, 112)	
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
	;WITH C0 AS (
		SELECT DISTINCT AccountID
		FROM (' + @nvcStmtOUT + N') L
		WHERE LogoutDate >= @dt2StartLogoutDate AND LogoutDate < @dt2EndLogoutDate AND MachineID = @binMachineID
	)
	SELECT A.AccountID, A.AccountName
	FROM C0
		INNER JOIN dbo.DimAccounts A WITH (NOLOCK) ON C0.AccountID = A.AccountID;
	'
	EXEC sp_executesql @nvcStmt, N'
		  @dt2StartLogoutDate datetime2(0)
		, @dt2EndLogoutDate datetime2(0)
		, @binMachineID binary(16)'
		, @dt2StartLogoutDate = @dt2StartLogoutDate
		, @dt2EndLogoutDate = @dt2EndLogoutDate
		, @binMachineID = @binMachineID;
	
	/**_## ??? ???? ?? ???? ???, 104101? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 104101;
		GOTO ErrorHandler;
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
