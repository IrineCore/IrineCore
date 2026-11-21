USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_UpsDarkLairRank]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-02-22
description : ???? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4~6 = ???? ?? ??? ?? ?? at 2012-07-10 at ???
**/

CREATE PROCEDURE [dbo].[P_UpsDarkLairRank]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dtmBasisDate datetime = CAST(CAST(GETDATE() - 1 AS date) AS datetime);
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'DarkLairLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DAY, 1, @dtmBasisDate), 112)
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/*
	1. ??? ????? ??? ??? ??? ??
	2. ??? ??? ???? dbo.DarkLairRank ? ?? ?? ?? ??? update ?? ??? insert
	   -. Update
		  ?? ???? ? ??? ?? PlayRound ? ???       : ?? ??? ????
		  ?? ???? ? ??? ?? PlayRound ? ???       : ? ? ????
		  ?? ???? ? ??? ?? PlayRound ? ???       : PlayTime ??
		  ?? ???? ? ??? ?? PlayTime ? ???        : ?? ? ????
		  ?? ???? ? ??? ?? PlayTime ? ??? ??? : ? ? ???? (PlayTime ?? ??? LogDate DESC ?? ??? ?? ?? ? ??? ???? ??.)
		-. Insert
		  ??? ????? ??
	3. WorldRank > 10000 ?? ?? ??
	*/
	/* ?? ??? ?? ?? ?? ?? ?? */
	EXEC dbo.P_MakeTableString
	  @nvcDBName = @nvcDBName
	, @nvcTblName = @nvcTblName
	, @nvcStartDate = @nvcStartDate
	, @nvcEndDate = @nvcEndDate
	, @nvcHint = @nvcHint
	, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	

	SET @nvcStmt = N'
	;WITH DailyRanking AS
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

	MERGE dbo.DarkLairRank a
	USING
		DailyRanking b
	ON 
		a.WorldID          = b.WorldID
		AND a.MapID        = b.MapID
		AND a.PartySize    = b.PartySize
		AND a.CharacterID1 = b.CharacterID1
		AND ISNULL(a.CharacterID2,'''') = ISNULL(b.CharacterID2,'''')
		AND ISNULL(a.CharacterID3,'''') = ISNULL(b.CharacterID3,'''')
		AND ISNULL(a.CharacterID4,'''') = ISNULL(b.CharacterID4,'''')
	WHEN MATCHED THEN
		UPDATE SET 
			  a.PartyName       = CASE WHEN a.PlayRound < b.PlayRound THEN b.PartyName 
									   WHEN a.PlayRound > b.PlayRound THEN a.PartyName 
									   WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.PartyName
												 WHEN a.PlayTime >= b.PlayTime THEN b.PartyName END END		
									             
			, a.PlayRound       = CASE WHEN a.PlayRound < b.PlayRound THEN b.PlayRound 
									   WHEN a.PlayRound > b.PlayRound THEN a.PlayRound 
									   WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.PlayRound
												 WHEN a.PlayTime >= b.PlayTime THEN b.PlayRound END END		
			
			, a.PlayTime        =  CASE WHEN a.PlayRound < b.PlayRound THEN b.PlayTime 
										WHEN a.PlayRound > b.PlayRound THEN a.PlayTime 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.PlayTime
												 WHEN a.PlayTime >= b.PlayTime THEN b.PlayTime END END
									             
			, a.CharacterLevel1 =  CASE WHEN a.PlayRound < b.PlayRound THEN b.CharacterLevel1 
										WHEN a.PlayRound > b.PlayRound THEN a.CharacterLevel1 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.CharacterLevel1
												 WHEN a.PlayTime >= b.PlayTime THEN b.CharacterLevel1 END END
									             
			, a.JobCode1		=  CASE WHEN a.PlayRound < b.PlayRound THEN b.JobCode1 
										WHEN a.PlayRound > b.PlayRound THEN a.JobCode1 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.JobCode1
												 WHEN a.PlayTime >= b.PlayTime THEN b.JobCode1 END END
									             
			, a.CharacterLevel2 =  CASE WHEN a.PlayRound < b.PlayRound THEN b.CharacterLevel2 
										WHEN a.PlayRound > b.PlayRound THEN a.CharacterLevel2 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.CharacterLevel2
												 WHEN a.PlayTime >= b.PlayTime THEN b.CharacterLevel2 END END
									             
			, a.JobCode2		=  CASE WHEN a.PlayRound < b.PlayRound THEN b.JobCode2 
										WHEN a.PlayRound > b.PlayRound THEN a.JobCode2 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.JobCode2
												 WHEN a.PlayTime >= b.PlayTime THEN b.JobCode2 END END
									             
			, a.CharacterLevel3	=  CASE WHEN a.PlayRound < b.PlayRound THEN b.CharacterLevel3 
										WHEN a.PlayRound > b.PlayRound THEN a.CharacterLevel3 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.CharacterLevel3
												 WHEN a.PlayTime >= b.PlayTime THEN b.CharacterLevel3 END END
									             
			, a.JobCode3		=  CASE WHEN a.PlayRound < b.PlayRound THEN b.JobCode3 
										WHEN a.PlayRound > b.PlayRound THEN a.JobCode3 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.JobCode3
												 WHEN a.PlayTime >= b.PlayTime THEN b.JobCode3 END END
									             
			, a.CharacterLevel4 =  CASE WHEN a.PlayRound < b.PlayRound THEN b.CharacterLevel4 
										WHEN a.PlayRound > b.PlayRound THEN a.CharacterLevel4 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.CharacterLevel4
												 WHEN a.PlayTime >= b.PlayTime THEN b.CharacterLevel4 END END
									             
			, a.JobCode4		=  CASE WHEN a.PlayRound < b.PlayRound THEN b.JobCode4 
										WHEN a.PlayRound > b.PlayRound THEN a.JobCode4 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.JobCode4
												 WHEN a.PlayTime >= b.PlayTime THEN b.JobCode4 END END
									             
			, a.LogDate		    =  CASE WHEN a.PlayRound < b.PlayRound THEN b.LogDate 
										WHEN a.PlayRound > b.PlayRound THEN a.LogDate 
										WHEN a.PlayRound = b.PlayRound THEN
											CASE WHEN a.PlayTime <  b.PlayTime THEN a.LogDate
												 WHEN a.PlayTime >= b.PlayTime THEN b.LogDate END END
	WHEN NOT MATCHED BY TARGET THEN
		INSERT 
		(
			  WorldID, MapID, PartySize, PartyName, PlayRound, PlayTime, CharacterID1, CharacterLevel1, JobCode1, CharacterID2, CharacterLevel2, JobCode2
			, CharacterID3, CharacterLevel3, JobCode3, CharacterID4, CharacterLevel4, JobCode4, LogDate, WorldRank, TotalRank		
		)
		VALUES
		(
			  b.WorldID, b.MapID, b.PartySize, b.PartyName, b.PlayRound, b.PlayTime, b.CharacterID1, b.CharacterLevel1, b.JobCode1, b.CharacterID2
			, b.CharacterLevel2, b.JobCode2, b.CharacterID3, b.CharacterLevel3, b.JobCode3, b.CharacterID4, b.CharacterLevel4, b.JobCode4, b.LogDate, 0, 0
		);'
	
	EXEC sp_executesql @nvcStmt;

	/* ??? ????? ?? ???? */
	UPDATE a SET 
		  WorldRank = b.WorldRank
		, TotalRank = b.TotalRank
	FROM
		dbo.DarkLairRank a
	LEFT OUTER JOIN
	(
		SELECT
		      DarkLairRankID
			, WorldID
			, MapID
			, PartySize
			, CharacterID1
			, CharacterID2
			, CharacterID3
			, CharacterID4
			, WorldRank = ROW_NUMBER() OVER(PARTITION BY WorldID, MapID, PartySize ORDER BY PlayRound DESC, PlayTime ASC, LogDate DESC)
			, TotalRank = CASE WHEN ROW_NUMBER() OVER(PARTITION BY MapID, PartySize ORDER BY PlayRound DESC, PlayTime ASC, LogDate DESC) > 10000 THEN 10000
						  ELSE ROW_NUMBER() OVER(PARTITION BY MapID, PartySize ORDER BY PlayRound DESC, PlayTime ASC, LogDate DESC) END
		FROM
			dbo.DarkLairRank
	) b
	ON a.DarkLairRankID = b.DarkLairRankID     

	/* WorldRanking > 10000 ??? ?? */
	DELETE dbo.DarkLairRank WHERE WorldRank > 10000;
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
