USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterLevelLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-22
description : ??? ?? ?? ??? ?????.
result set : LogDate/datetime2, CharacterLevelChangeCode/tinyint, CurrentCharacterLevel/tinyint, MapName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104101 = ?? ?? ??? ????.
history :
4~6 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetListCharacterLevelLog]
	  @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CharacterLevelLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(MM, -3, GETDATE()), 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DD, -1, GETDATE()), 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK, FORCESEEK'
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
	SELECT LogDate, CharacterLevelChangeCode, CurrentCharacterLevel, MapID
	FROM (' + @nvcStmtOUT + N') V
	WHERE CharacterID = @inbCharacterID
	ORDER BY LogDate;
	'
	EXEC sp_executesql @nvcStmt, N'@inbCharacterID bigint', @inbCharacterID = @inbCharacterID;
	
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
