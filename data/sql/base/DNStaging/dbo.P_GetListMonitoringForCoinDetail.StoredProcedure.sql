USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMonitoringForCoinDetail]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-22

description :
	???? ??? ?? ??
	??? ????? ????? ??? ??? ?? (?? ???? ??? ?? ????? ??)
result set : 
	Coin			bigint
	CharacterName	nvarchar(30)
	CharacterLevel	tinyint
	JobCode			tinyint
	CharacterID		bigint
	AccountID		int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	6 = ?? ??
	7 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07	
**/

CREATE PROCEDURE [dbo].[P_GetListMonitoringForCoinDetail]
	  @intWorldID		int			--// ??
    , @inbCharacterID	bigint		--// ??? ID
    , @dtmStartDate		datetime	--// ?? ???
    , @dtmEndDate		datetime	--// ?? ???
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'MailLogs'
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
	
	/**_# ??? ????? ????? ??? ??? ??? ?????. */
	SET @nvcStmt = N'
	SELECT SUM(VML.AttachCoin) AS SendCoin
		 , DC.CharacterName
		 , DC.CharacterLevel
		 , DC.JobCode
		 , DC.CharacterID
		 , DC.AccountID
	  FROM dbo.DimCharacters DC WITH (NOLOCK)
		   INNER JOIN(' + @nvcStmtOUT + N') VML ON (DC.CharacterID = VML.SenderCharacterID)
	 WHERE (DC.WorldID = @intWorldID)
	   AND (DC.CharacterID = @inbCharacterID)
	   AND (VML.SendDate >= @dtmStartDate)
	   AND (VML.SendDate <= @dtmEndDate)
	 GROUP BY DC.CharacterName, DC.CharacterLevel, DC.JobCode, DC.CharacterID, DC.AccountID
	 ORDER BY SUM(VML.AttachCoin) DESC;
	';
	EXEC sp_executesql @nvcStmt, N'
		  @intWorldID int		
		, @inbCharacterID	bigint	
		, @dtmStartDate datetime
		, @dtmEndDate datetime'
		, @intWorldID = @intWorldID
		, @inbCharacterID = @inbCharacterID
		, @dtmStartDate = @dtmStartDate
		, @dtmEndDate = @dtmEndDate;
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
