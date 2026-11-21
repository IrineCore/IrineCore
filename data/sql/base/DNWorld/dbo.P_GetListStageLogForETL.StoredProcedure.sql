USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListStageLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-06
description : ???? ??? ?????. DNStaging ???
result set : StageStartDate/datetime2, StageEndDate/datetime2, StageStartID/bigint, PartyStartID/bigint, PartySize/tinyint, MapID/int, DifficultyCode/tinyint, WhereToGoCode/tinyint, WorldID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = ?? ?? ?? at 2010-08-18
**/

CREATE PROCEDURE [dbo].[P_GetListStageLogForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;

	SELECT L.StageStartDate, L.StageEndDate, L.StageStartID, L.PartyStartID, L.PartySize, L.MapID, L.DifficultyCode, L.WhereToGoCode, @intWorldID AS WorldID
	FROM dbo.StageStartIDs L WITH (NOLOCK)
	WHERE L.StageStartDate >= CAST(@dtmBasisDate AS datetime2(0)) AND StageStartDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
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
