USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetTradeItemInfo]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-02-15
description : ?? ??? ???? ??? ?????.
result set : ItemID/int, ItemLevel/tinyint, SealCount/tinyint, ItemDurability/smallint, ItemCount/smallint, RandomSeed/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetTradeItemInfo]
	  @inbCharacterID int --//?? ??? CharacterID
	, @intTradeID int --// ?? ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcLogTableSuffix nvarchar(8) = CONVERT(nvarchar(8), GETDATE(), 112);
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'TradeSuccessLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(MM, -3, GETDATE()), 112)
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
	EXEC dbo.P_MakeTableString
	  @nvcDBName = @nvcDBName
	, @nvcTblName = @nvcTblName
	, @nvcStartDate = @nvcStartDate
	, @nvcEndDate = @nvcEndDate
	, @nvcHint = @nvcHint
	, @nvcStmtOUT = @nvcStmtOUT OUTPUT;

	/**_# [TradeSuccessLogs] ?????.*/
	SET @nvcStmt = N'
SELECT ItemID, ItemLevel, SealCount, ItemDurability, ItemCount, RandomSeed
FROM (' + @nvcStmtOUT + N') V
WHERE TradeID = @intTradeID AND CharacterID =  @inbCharacterID'
	EXEC sp_executesql @nvcStmt, N'@intTradeID int, @inbCharacterID bigint', @intTradeID = @intTradeID, @inbCharacterID = @inbCharacterID;

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
