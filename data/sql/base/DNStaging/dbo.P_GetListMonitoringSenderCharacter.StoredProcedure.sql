USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMonitoringSenderCharacter]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-02-27
description : ?? ?? ?? ?? ????? ??? ?? ???? ?????.
result set : SenderCharacterID/bigint, CharacterName/nvarchar(50), TotalCoin/int, TotalItemCount/int, TotalMailCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListMonitoringSenderCharacter]
	  @dt2StratDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ?? ??
	, @inbCharacterID bigint --// ??? ???ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE 
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcDBName nvarchar(50) = N'DNStaging',
	@nvcTblName nvarchar(50) = N'MailLogs',
	@nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StratDate, 112),
	@nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112),
	@nvcHint nvarchar(50) = N'NOLOCK',
	@nvcStmtOUT nvarchar(max) = N'';

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
	/**_# ?? ??? ?????.*/
	SET @nvcStmt = N'
SELECT V.SenderCharacterID, DC.CharacterName AS SenderCharacterName, SUM(AttachCoin) AS TotalCoin, SUM(CAST(AttachItemFlag AS int)) AS TotalItemCount, COUNT(*) AS TotalMailCount
FROM (' + @nvcStmtOUT + N') V INNER JOIN dbo.DimCharacters DC WITH(NOLOCK) ON V.SenderCharacterID = DC.CharacterID
WHERE ReceiverCharacterID = @inbCharacterID
  AND MailTypeCode = 1
GROUP BY V.SenderCharacterID, DC.CharacterName';
	
	EXEC sys.sp_executesql @nvcStmt, N'@inbCharacterID int', @inbCharacterID = @inbCharacterID;

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
