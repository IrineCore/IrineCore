USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListItemLogForOwnership]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-26
description : ?? ??? ???? ??? ??? ?????. ?, ?? ???? ???? ???? ????.
result set : LogDate/datetime2, ItemSerial/bigint, ItemID/int, ItemName/nvarchar, ItemLogCode/tinyint, NextOwnerCharacterID/bigint, NextOwnerCharacterName/nvarchar, IP/varchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104101 = ?? ?? ??? ????.
history :
5~7 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetListItemLogForOwnership]
	  @inbCharacterID bigint --// ???? ???? ??? ID
	, @dt2BasisDate datetime2(0) --// ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @nvcStmt nvarchar(max)
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'ItemLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2BasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), GETDATE(), 112)	
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
SELECT V.LogDate, V.ItemSerial, V.ItemID, DI.ItemName, V.ItemLogCode, V.OwnerCharacterID AS NextOwnerCharacterID, DC1.CharacterName AS NextOwnerCharacterName, dbo.FN_IPNumber2IP(V.IP) AS IP
FROM (' + @nvcStmtOUT + N') V
	LEFT OUTER JOIN dbo.DimCharacters DC1 WITH (NOLOCK) ON V.OwnerCharacterID = DC1.CharacterID
	LEFT OUTER JOIN dbo.DimItems DI WITH (NOLOCK) ON V.ItemID = DI.ItemID
WHERE V.LogDate >= ''' + CONVERT(nvarchar(19), @dt2BasisDate, 121) + N'''
	AND V.OwnerCharacterIDBefore = @inbCharacterID
	AND V.OwnershipStartDate <= @dt2BasisDate
ORDER BY V.LogDate';

	EXEC dbo.sp_executesql @nvcStmt, N'@inbCharacterID bigint, @dt2BasisDate datetime2(0)', @inbCharacterID = @inbCharacterID, @dt2BasisDate = @dt2BasisDate;

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
