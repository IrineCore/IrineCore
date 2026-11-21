USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListStageClearTime]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author :  sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-10-21

description :
	?? ?? ???? ??? ??? ???? ??? ??
result set : 
	StageStartDate	datetime2(0)
	StageEndDate	datetime2(0)
	WorldID			int
	DifficultyCode	tinyint
	PartySize		tinyint
	AccountID		int
	CharacterName	nvarchar(30)
	CharacterID		bigint
	CharacterLevel	tinyint
	ClearTime		int
	StageStartID	bigint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	4 = MapID ?? ??? StageID ?? ???? ?? by 20110331.wgkim
	5~7 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
	8 = StageID ???? ?? ? ???? ?? ? ? ??? ?? by ??? at 2012-05-15
	9 = WorldID ??? DimCharacters ???? ?? by ??? at 2012-05-15
**/

CREATE PROCEDURE [dbo].[P_GetListStageClearTime]
	  @intStageID		int				--// StageID
	, @intDurationTime	int				--// ?? ?? ??
	, @dt2StartDate		datetime2(0)	--// ?? ?? ??
	, @dt2EndDate		datetime2(0)	--// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'StageEndLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112)
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
	SELECT SEL.StageStartDate
		 , SEL.StageEndDate
		 , DC.WorldID
		 , SEL.DifficultyCode
		 , SEL.PartySize
		 , DC.AccountID
		 , DC.CharacterName
		 , SEL.CharacterID
		 , SEL.CharacterLevel
		 , SEL.ClearTime
		 , SEL.StageStartID
	  FROM (' + @nvcStmtOUT + N') SEL
		   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (SEL.CharacterID = DC.CharacterID)
		   INNER JOIN dbo.DimMaps DM WITH (NOLOCK) ON (SEL.MapID = DM.MapID)
	 WHERE SEL.StageStartDate >= @dt2StartDate
	   AND SEL.StageStartDate <= @dt2EndDate
	   AND DM.StageID = @intStageID
	   AND SEL.ClearFlag = 1
	   AND SEL.ClearTime <= @intDurationTime
	 ORDER BY SEL.ClearTime;
	';
	EXEC sp_executesql @nvcStmt, N'
		  @dt2StartDate datetime2(0)
		, @dt2EndDate datetime2(0)
		, @intStageID int
		, @intDurationTime int'
		, @dt2StartDate = @dt2StartDate
		, @dt2EndDate = @dt2EndDate
		, @intStageID = @intStageID
		, @intDurationTime = @intDurationTime;		
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
