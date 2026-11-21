USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPCRoomCharacterLogoutLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-30
description : ??? ??? ???? ??? ?????.
result set : LoginDate/datetime2, LogoutDate/datetime2, LoginIP/varchar, AccountID/int, AccountName/nvarchar, WorldID/int, CharacterID/bigint, CharacterName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104101 = ?? ?? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListPCRoomCharacterLogoutLog]
	  @nvcAccountName nvarchar(50) = NULL --// ??? ?? ??
	, @nvcCharacterName nvarchar(30) = NULL --// ??? ??
	, @intWorldID int = NULL --// ?? ID
	, @vchStartLoginIP varchar(15) = NULL --// ?? ??? IP ??
	, @vchEndLoginIP varchar(15) = NULL --// ? ??? IP ??
	, @dt2StartLogoutDate datetime2(0) --// (????) ?? ?? ?? - ??? ?? ?? ??
	, @dt2EndLogoutDate datetime2(0) --// (????) ?? ? ?? - ??? ?? ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@intStartLoginIP int = dbo.FN_IP2IPNumber(@vchStartLoginIP),
	@intEndLoginIP int = dbo.FN_IP2IPNumber(@vchEndLoginIP),
	@nvcStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;
DECLARE @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CharacterLogoutLogs'
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
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @intColumnCount = 7;

	SET @inbNullBitmap = 
		CASE WHEN @dt2EndLogoutDate IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @dt2EndLogoutDate IS NOT NULL THEN POWER(2, 1) ELSE 0 END +
		CASE WHEN @nvcAccountName IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @nvcCharacterName IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @intWorldID IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @intStartLoginIP IS NOT NULL THEN POWER(2, 5) ELSE 0 END + 
		CASE WHEN @intStartLoginIP IS NOT NULL THEN POWER(2, 6) ELSE 0 END;

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;

	SET @nvcStmt = N'

SELECT ROW_NUMBER() OVER(ORDER BY V.LogoutDate) AS RowNumber, V.LoginDate, V.LogoutDate, dbo.FN_IPNumber2IP(V.LoginIP) AS LoginIP, DA.AccountID, DA.AccountName, DC.WorldID, DC.CharacterID, DC.CharacterName
FROM (' + @nvcStmtOUT + N') V
	INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON V.CharacterID = DC.CharacterID
	INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
WHERE (V.LogoutDate BETWEEN ''' + CONVERT(nvarchar(19), @dt2StartLogoutDate, 121) + N''' AND ''' + CONVERT(nvarchar(19), @dt2EndLogoutDate, 121) + N'''
	OR V.LoginDate BETWEEN ''' + CONVERT(nvarchar(19), @dt2StartLogoutDate, 121) + N''' AND ''' + CONVERT(nvarchar(19), @dt2EndLogoutDate, 121) + N''')
	AND PCRoomFlag = 1
'	+ CASE WHEN @inbNullBitmap > 3 THEN N' AND ' ELSE N'' END
	+ dbo.FN_DynamicWhereClause2(N'DA.AccountName', N'@nvcAccountName', @inbNullBitmap, 3, @intColumnCount, '=')
	+ dbo.FN_DynamicWhereClause2(N'DC.CharacterName', N'@nvcCharacterName', @inbNullBitmap, 4, @intColumnCount, '=')
	+ dbo.FN_DynamicWhereClause2(N'DC.WorldID', N'@intWorldID', @inbNullBitmap, 5, @intColumnCount, '=')
	+ dbo.FN_DynamicWhereClause2(N'V.LoginIP', N'@intStartLoginIP', @inbNullBitmap, 6, @intColumnCount, '>=')
	+ dbo.FN_DynamicWhereClause2(N'V.LoginIP', N'@intEndLoginIP', @inbNullBitmap, 7, @intColumnCount, '<=') + N'
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcAccountName nvarchar(50)
			, @nvcCharacterName nvarchar(30)
			, @intWorldID int
			, @intStartLoginIP int
			, @intEndLoginIP int
		'
		, @nvcAccountName = @nvcAccountName
		, @nvcCharacterName = @nvcCharacterName
		, @intWorldID = @intWorldID
		, @intStartLoginIP = @intStartLoginIP
		, @intEndLoginIP = @intEndLoginIP;

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
