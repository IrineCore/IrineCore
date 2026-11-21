USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDonateCharacter]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-13
description : ?? ??? ?? ???? ?? ?? ?????.
result set : CharacterID/bigint, CharacterName/nvarchar, DonateDate/datetime2(0) ,DonateAmount/int 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3~4 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
6 = WorldID Input ???? ?? by ??? at 2012-03-12
7 = ???? ?? ???? ???? ?? by ??? at 2012-03-13
**/

CREATE PROCEDURE [dbo].[P_GetDonateCharacter]
	  @dtmStartDate datetime --// ??? ?? ??
	, @dtmEndDate datetime --// ??? ? ??
	, @inbCharacterID bigint --// ??? ??? ID
    , @intWorldID int --// ??ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CoinLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtmStartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtmEndDate, 112)		
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
SELECT V.CharacterID, DC.CharacterName, V.LogDate AS DonateDate, ABS(V.ChangedCoin) AS DonateAmount
FROM (' + @nvcStmtOUT + N') V JOIN dbo.dimCharacters DC WITH (NOLOCK) ON V.CharacterID = DC.CharacterID 
WHERE CoinChangeCode = 34
  AND LogDate >= @dtmStartDate AND LogDate <= @dtmEndDate
  AND V.CharacterID = @inbCharacterID
  AND DC.DeleteFlag = 0
  AND DC.WorldID = @intWorldID
ORDER BY LogDate;
	';
	EXEC sp_executesql @nvcStmt, N'
		  @dtmStartDate datetime
		, @dtmEndDate datetime
		, @inbCharacterID bigint
		, @intWorldID int'
		, @dtmStartDate = @dtmStartDate
		, @dtmEndDate = @dtmEndDate
		, @inbCharacterID = @inbCharacterID
		, @intWorldID = @intWorldID;
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
