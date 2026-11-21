USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDarkLairLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-14
description : ???? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = SET ANSI_WARNINGS OFF ?? ?? at 2011-02-05
**/

CREATE PROCEDURE [dbo].[P_GetListDarkLairLogForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET ANSI_WARNINGS OFF;

DECLARE @intReturnValue int;
DECLARE @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP 1 @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;


	WITH C1 AS (
		SELECT ROW_NUMBER() OVER(PARTITION BY DarkLairRankingID ORDER BY CharacterID) AS Seq
			, DarkLairRankingID, CharacterID, CharacterLevel, JobCode
		FROM dbo.DarkLairPartyMembers WITH (NOLOCK)
	), C2 AS (
		SELECT DarkLairRankingID
			, MAX(CASE Seq WHEN 1 THEN CharacterID END) AS CharacterID1
			, MAX(CASE Seq WHEN 1 THEN CharacterLevel END) AS CharacterLevel1
			, MAX(CASE Seq WHEN 1 THEN JobCode END) AS JobCode1
			, MAX(CASE Seq WHEN 2 THEN CharacterID END) AS CharacterID2
			, MAX(CASE Seq WHEN 2 THEN CharacterLevel END) AS CharacterLevel2
			, MAX(CASE Seq WHEN 2 THEN JobCode END) AS JobCode2
			, MAX(CASE Seq WHEN 3 THEN CharacterID END) AS CharacterID3
			, MAX(CASE Seq WHEN 3 THEN CharacterLevel END) AS CharacterLevel3
			, MAX(CASE Seq WHEN 3 THEN JobCode END) AS JobCode3
			, MAX(CASE Seq WHEN 4 THEN CharacterID END) AS CharacterID4
			, MAX(CASE Seq WHEN 4 THEN CharacterLevel END) AS CharacterLevel4
			, MAX(CASE Seq WHEN 4 THEN JobCode END) AS JobCode4
		FROM C1
		GROUP BY DarkLairRankingID
	)
	SELECT DLR.RegisterDate AS LogDate, @intWorldID AS WorldID, DLR.MapID, DLR.PartyUserCount AS PartySize, DLR.PartyName, DLR.PlayRound, DLR.PlayTime
		, C2.CharacterID1, C2.CharacterLevel1, C2.JobCode1
		, C2.CharacterID2, C2.CharacterLevel2, C2.JobCode2
		, C2.CharacterID3, C2.CharacterLevel3, C2.JobCode3
		, C2.CharacterID4, C2.CharacterLevel4, C2.JobCode4
	FROM dbo.DarkLairRanking DLR WITH (NOLOCK)
		INNER JOIN C2 ON DLR.DarkLairRankingID = C2.DarkLairRankingID
	WHERE DLR.RegisterDate >= @dtmBasisDate AND DLR.RegisterDate < (@dtmBasisDate + 1)
	OPTION (MAXDOP 1);
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
