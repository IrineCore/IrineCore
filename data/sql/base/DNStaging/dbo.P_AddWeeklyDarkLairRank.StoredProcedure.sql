USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddWeeklyDarkLairRank]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-02-22
description : ?? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddWeeklyDarkLairRank]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int;

DECLARE @dtmBasisDate datetime = CAST(CAST(GETDATE() - 1 AS date) AS datetime);
DECLARE @dt2StartDate datetime2(0) = @dtmBasisDate - DATEPART(WEEKDAY, @dtmBasisDate) + 1;
DECLARE @dt2EndDate datetime2(0) = DATEADD(DAY, 7, @dt2StartDate);
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'DarkLairLogs'
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
	EXEC dbo.P_MakeTableString
	  @nvcDBName = @nvcDBName
	, @nvcTblName = @nvcTblName
	, @nvcStartDate = @nvcStartDate
	, @nvcEndDate = @nvcEndDate
	, @nvcHint = @nvcHint
	, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
	
	TRUNCATE TABLE dbo.WeeklyDarkLairRank;
	SET @nvcStmt = N'
	/* ?? ??? ?? ??? ?? ?? */
	WITH WeeklyRanking AS
	(
		SELECT
			  WorldID
			, MapID
			, PartySize
			, PartyName
			, PlayRound
			, PlayTime
			, CharacterID1
			, CharacterLevel1
			, JobCode1
			, CharacterID2
			, CharacterLevel2
			, JobCode2
			, CharacterID3
			, CharacterLevel3
			, JobCode3
			, CharacterID4
			, CharacterLevel4
			, JobCode4
			, LogDate
		FROM
		(
			SELECT
				  WorldID
				, Rkey = ROW_NUMBER() OVER(PARTITION BY WorldID, MapID, PartySize, CharacterID1, CharacterID2, CharacterID3, CharacterID4 ORDER BY PlayRound DESC, PlayTime ASC, LogDate DESC)
				, MapID
				, PartySize
				, PartyName
				, PlayRound
				, PlayTime
				, CharacterID1
				, CharacterLevel1
				, JobCode1
				, CharacterID2
				, CharacterLevel2
				, JobCode2
				, CharacterID3
				, CharacterLevel3
				, JobCode3
				, CharacterID4
				, CharacterLevel4
				, JobCode4
				, LogDate
			FROM 
				(' + @nvcStmtOUT + N') A
		) a
		WHERE
			Rkey = 1
	)

	/* ??? ????? ?? ???? */
	INSERT INTO dbo.WeeklyDarkLairRank
	(
		  BasisDate, WorldID, MapID, PartySize, PartyName, PlayRound, PlayTime, CharacterID1, CharacterLevel1, JobCode1, CharacterID2, CharacterLevel2
		, JobCode2, CharacterID3, CharacterLevel3, JobCode3, CharacterID4, CharacterLevel4, JobCode4, LogDate, WorldRank, TotalRank
	)
	SELECT
		  BasisDate = @dt2StartDate
		, WorldID
		, MapID
		, PartySize
		, PartyName
		, PlayRound
		, PlayTime
		, CharacterID1
		, CharacterLevel1
		, JobCode1
		, CharacterID2
		, CharacterLevel2
		, JobCode2
		, CharacterID3
		, CharacterLevel3
		, JobCode3
		, CharacterID4
		, CharacterLevel4
		, JobCode4
		, LogDate		
		, WorldRank
		, TotalRank
	FROM
	(
		SELECT
			  WorldID
			, MapID
			, PartySize
			, PartyName
			, PlayRound
			, PlayTime
			, CharacterID1
			, CharacterLevel1
			, JobCode1
			, CharacterID2
			, CharacterLevel2
			, JobCode2
			, CharacterID3
			, CharacterLevel3
			, JobCode3
			, CharacterID4
			, CharacterLevel4
			, JobCode4
			, LogDate		
			, WorldRank = ROW_NUMBER() OVER(PARTITION BY WorldID, MapID, PartySize ORDER BY PlayRound DESC, PlayTime ASC, LogDate DESC)
			, TotalRank = CASE WHEN ROW_NUMBER() OVER(PARTITION BY MapID, PartySize ORDER BY PlayRound DESC, PlayTime ASC, LogDate DESC) > 10000 THEN 10000
						  ELSE ROW_NUMBER() OVER(PARTITION BY MapID, PartySize ORDER BY PlayRound DESC, PlayTime ASC, LogDate DESC) END
		FROM
			WeeklyRanking
	) a
	WHERE
		WorldRank <= 10000;'

	EXEC sp_executesql @nvcStmt, N'
		  @dt2StartDate datetime2(0)
		, @dt2EndDate datetime2(0)'
		, @dt2StartDate = @dt2StartDate
		, @dt2EndDate = @dt2EndDate


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
