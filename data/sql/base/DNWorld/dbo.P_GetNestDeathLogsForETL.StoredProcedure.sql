USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetNestDeathLogsForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-08-16
description : ??? ?? ??? ?????.(ETL ???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetNestDeathLogsForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dtmBasisDate);
DECLARE @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;
																																																																																																																																																																																											
	IF @intLogTableSuffix = 0
		SELECT B.LogDate, B.CharacterID, B.CharacterLevel, B.JobCode ,B.MonsterSkillCode, B.MosterCode, B.StageStartID, A.MapID, A.PartySize, @intWorldID AS WorldID 
		FROM dbo.StageStartIDs A  
			INNER JOIN dbo.NestDeathLogs0 B ON A.StageStartID = B.StageStartID		
		WHERE B.LogDate BETWEEN CAST(@dtmBasisDate AS datetime2(0)) AND CAST(@dtmBasisDate + 1  AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE IF @intLogTableSuffix = 1
		SELECT B.LogDate, B.CharacterID, B.CharacterLevel, B.JobCode ,B.MonsterSkillCode, B.MosterCode, B.StageStartID, A.MapID, A.PartySize, @intWorldID AS WorldID 
		FROM dbo.StageStartIDs A  
			INNER JOIN dbo.NestDeathLogs1 B ON A.StageStartID = B.StageStartID		
		WHERE B.LogDate BETWEEN CAST(@dtmBasisDate AS datetime2(0)) AND CAST(@dtmBasisDate + 1  AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE
		SELECT B.LogDate, B.CharacterID, B.CharacterLevel, B.JobCode ,B.MonsterSkillCode, B.MosterCode, B.StageStartID, A.MapID, A.PartySize, @intWorldID AS WorldID 
		FROM dbo.StageStartIDs A  
			INNER JOIN dbo.NestDeathLogs2 B ON A.StageStartID = B.StageStartID		
		WHERE B.LogDate BETWEEN CAST(@dtmBasisDate AS datetime2(0)) AND CAST(@dtmBasisDate + 1  AS datetime2(0))
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
