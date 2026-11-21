USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFieldDeleteLogs]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-23

description :
	??? ?? ?? ??
result set : 
	@intSelectClauseCode = 1 : LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ?? ??
	3~5 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetListFieldDeleteLogs]
	  @intWorld int = NULL --// ??ID
	, @dtmStartDate datetime --// ?? ??? (?? or ?? ??)
	, @dtmEndDate datetime --// ?? ??? (?? or ?? ??)
	, @inbCharacterID bigint = NULL --// ???ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'FieldDeleteLogs'
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
	SELECT DC.WorldID, FL.LogDate, FL.FarmID, FL.FieldIndex, DC.CharacterName, DA.AccountName, DI.ItemName, FL.PlantDate, FL.ElapsedTime, FL.HarvestFlag, DC.CharacterID, DA.AccountID, DI.ItemID
	  FROM (' + @nvcStmtOUT + N') FL
	       INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (FL.CharacterID = DC.CharacterID)
	       INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DC.AccountID = DA.AccountID)
	       LEFT OUTER JOIN dbo.DimItems DI WITH (NOLOCK) ON (FL.ItemID = DI.ItemID)
	 WHERE FL.LogDate >= @dtmStartDate
	   AND FL.LogDate < @dtmEndDate
	   AND (@inbCharacterID IS NULL OR FL.CharacterID = @inbCharacterID)
	   AND (@intWorld IS NULL OR DC.WorldID = @intWorld);
	';
	EXEC sp_executesql @nvcStmt, N'
		  @intWorld int
		, @dtmStartDate datetime
		, @dtmEndDate datetime
		, @inbCharacterID bigint'
		, @intWorld = @intWorld
		, @dtmStartDate = @dtmStartDate
		, @dtmEndDate = @dtmEndDate
		, @inbCharacterID = @inbCharacterID;
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
