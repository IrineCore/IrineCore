USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterLoginLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-27
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListCharacterLoginLog]
	  @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CharacterLogoutLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ??**/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
	SET @nvcStmt = N'
	SELECT DISTINCT B.AccountName, A.CharacterID, A.CharacterName, C.JobName, D.JobName AS CharacterClassName
	FROM dbo.DimCharacters A WITH(NOLOCK) 
		INNER JOIN dbo.DimAccounts B WITH(NOLOCK) ON A.AccountID = B.AccountID
		INNER JOIN dbo.DimJobs C WITH(NOLOCK) ON A.JobCode = C.JobCode
		INNER JOIN dbo.DimJobs D WITH(NOLOCK) ON A.CharacterClassCode = D.JobCode
		INNER JOIN (' + @nvcStmtOUT + N') E ON A.CharacterID = E.CharacterID
	WHERE E.LoginDate <= @dt2EndDate AND E.LogoutDate >= @dt2StartDate
      AND A.DeleteFlag = 0;'

	EXEC sp_executesql @nvcStmt, N'@dt2StartDate datetime2(0), @dt2EndDate datetime2(0)',@dt2StartDate = @dt2StartDate, @dt2EndDate = @dt2EndDate;

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
