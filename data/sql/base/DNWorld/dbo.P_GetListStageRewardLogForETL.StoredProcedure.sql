USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListStageRewardLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-09-26
description : ??? ? ???? ?? ??? ?????. DNStaging ???
result set : StageStartDate, StageEndDate, StageStartID, CharacterID, RewardBoxCode, RewardItemSerial, RewardItemID, WorldID, RewardFailReason 	
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListStageRewardLogForETL]
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
		SELECT L.StageStartDate, L.StageEndDate, RL.StageStartID, RL.CharacterID, RL.RewardBoxCode, RL.RewardItemSerial, RL.RewardItemID, @intWorldID AS WorldID, RL.RewardFailReason
		FROM dbo.StageStartIDs L WITH (NOLOCK)
			INNER JOIN dbo.StageRewardLogs0 RL WITH (NOLOCK) ON L.StageStartID = RL.StageStartID
		WHERE L.StageStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND L.StageStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE IF @intLogTableSuffix = 1
		SELECT L.StageStartDate, L.StageEndDate, RL.StageStartID, RL.CharacterID, RL.RewardBoxCode, RL.RewardItemSerial, RL.RewardItemID, @intWorldID AS WorldID, RL.RewardFailReason
		FROM dbo.StageStartIDs L WITH (NOLOCK)
			INNER JOIN dbo.StageRewardLogs1 RL WITH (NOLOCK) ON L.StageStartID = RL.StageStartID
		WHERE L.StageStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND L.StageStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE
		SELECT L.StageStartDate, L.StageEndDate, RL.StageStartID, RL.CharacterID, RL.RewardBoxCode, RL.RewardItemSerial, RL.RewardItemID, @intWorldID AS WorldID, RL.RewardFailReason
		FROM dbo.StageStartIDs L WITH (NOLOCK)
			INNER JOIN dbo.StageRewardLogs2 RL WITH (NOLOCK) ON L.StageStartID = RL.StageStartID
		WHERE L.StageStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND L.StageStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
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
