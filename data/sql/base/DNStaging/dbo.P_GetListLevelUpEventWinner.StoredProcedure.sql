USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListLevelUpEventWinner]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-09-09
description : ??? ??? ??? ??? ?? ?? ???? ?????.
result set : WorldName, CharacterName, CharacterID, CompleteTime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3~4 = ??? ???? ??? ????? ??, ???? ??? ?? ? ??? ?? by ??? at 2011-12-07
5~7 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetListLevelUpEventWinner]
	  @dtmStartDate datetime --// ?? ?? ??
	, @dtmEndDate datetime --// ?? ? ??
	, @vchJobCodes varchar(max) --// ??? ???? ??? (??? ,)
	, @inyCharacterLevel tinyint --// ??? ??
    , @dtmStartCreateDate datetime --//????? ? ??? ?? ?? ??
    , @dtmEndCreateDate datetime --//????? ? ??? ?? ? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CharacterLevelLogs'
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
	DECLARE @tblJobCodes table (
	JobCode tinyint NOT NULL PRIMARY KEY
	);
	INSERT @tblJobCodes (JobCode)
	SELECT CAST(string AS tinyint) FROM dbo.IF_Split(@vchJobCodes, '','');
	
	/**_# ??? ??? ??? ?? ?????? ?????.*/
	WITH T1 AS (
		SELECT DC.CharacterID, DC.CharacterName, DC.WorldID 
		FROM dbo.DimCharacters DC WITH(NOLOCK) JOIN @tblJobCodes JC ON DC.JobCode = JC.JobCode
		WHERE DC.CreateDate >= @dtmStartCreateDate AND DC.CreateDate < @dtmEndCreateDate
		and DC.DeleteFlag = 0
	)
	SELECT DW.WorldName, A.CharacterID, A.CharacterName, VL.LogDate 
	FROM (' + @nvcStmtOUT + N') VL JOIN T1 A ON VL.CharacterID = A.CharacterID
												  JOIN dbo.DimWorlds DW ON A.WorldID = DW.WorldID 
	WHERE VL.LogDate >= @dtmStartDate and VL.LogDate < @dtmEndDate
	AND VL.CurrentCharacterLevel = @inyCharacterLevel
	AND VL.CharacterLevelChangeCode = 1 
    AND VL.ChangedCharacterLevel > 0;
	'
	
	EXEC sp_executesql @nvcStmt, N'
		  @vchJobCodes varchar(max)
		, @inyCharacterLevel tinyint
		, @dtmStartCreateDate datetime
		, @dtmEndCreateDate datetime
		, @dtmStartDate datetime
		, @dtmEndDate datetime'
		, @vchJobCodes = @vchJobCodes
		, @inyCharacterLevel = @inyCharacterLevel
		, @dtmStartCreateDate = @dtmStartCreateDate
		, @dtmEndCreateDate = @dtmEndCreateDate
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
