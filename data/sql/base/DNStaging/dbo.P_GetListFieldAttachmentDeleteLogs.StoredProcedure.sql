USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFieldAttachmentDeleteLogs]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-23

description :
	??? ?? ??? ?? ?? ??
result set : 
	@intSelectClauseCode = 1 : LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ?? ??
	3 = input parameter ?? (@inbCharacterID)     by 20111017.wgkim
	4~6 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07	
**/

CREATE PROCEDURE [dbo].[P_GetListFieldAttachmentDeleteLogs]
	  @dtmStartDate datetime --// ?? ??? (?? or ?? ??)
	, @dtmEndDate datetime --// ?? ??? (?? or ?? ??)
	, @intFarmID int --// ??ID
	, @insFieldIndex smallint --// ??Index
	, @inbCharacterID bigint --// ???ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'FieldAttachmentDeleteLogs'
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
	SELECT FL.Attachments
	  FROM (' + @nvcStmtOUT + N') FL
	 WHERE FL.LogDate >= @dtmStartDate
	   AND FL.LogDate < @dtmEndDate
	   AND FL.FarmID = @intFarmID
	   AND FL.FieldIndex = @insFieldIndex
	   AND FL.CharacterID = @inbCharacterID;
	';
	EXEC sp_executesql @nvcStmt, N'
		  @dtmStartDate datetime
		, @dtmEndDate datetime
		, @intFarmID int
		, @insFieldIndex smallint
		, @inbCharacterID bigint'
		, @dtmStartDate = @dtmStartDate
		, @dtmEndDate = @dtmEndDate
		, @intFarmID = @intFarmID
		, @insFieldIndex = @insFieldIndex
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
