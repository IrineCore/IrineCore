USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMapStayLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-25
description : ? ?? ??? ?????.
result set : EntranceDate/datetime2, ExitDate/datetime2, MapID/int, JobCode/tinyint, CharacterLevel/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104101 = ?? ?? ??? ????.
history :
3~5 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetListMapStayLog]
	  @inbCharacterID bigint --// ??? ID
	, @dt2StartExitDate datetime2(0) --// ?? ?? ?? (??? ?? ?? ??)
	, @dt2EndExitDate datetime2(0) --// ?? ? ?? (??? ?? ?? ??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @nvcStmt nvarchar(max)
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'MapStayLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartExitDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndExitDate, 112)
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
SELECT EntranceDate, ExitDate, MapID, JobCode, CharacterLevel
FROM (' + @nvcStmtOUT + N') V
WHERE CharacterID = @inbCharacterID
	AND ExitDate BETWEEN ''' + CONVERT(nvarchar(19), @dt2StartExitDate, 121) + N''' AND ''' + CONVERT(nvarchar(19), @dt2EndExitDate, 121) + N'''
ORDER BY ExitDate';

	EXEC dbo.sp_executesql @nvcStmt, N'@inbCharacterID bigint', @inbCharacterID = @inbCharacterID;

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
