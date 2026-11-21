USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDailyMission]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-15
description : ?? ?? ??? ?????.
result set : AssignDate/datetime2, MissionIndex/tinyint, MissionID/int, MissionCounter/int, AchieveFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDailyMission]
	  @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT AM.AssignDate , AMD.MissionIndex, AMD.MissionID, AMD.MissionCounter, AMD.AchieveFlag
	FROM dbo.AssignMissionDetails AMD WITH (NOLOCK)
		INNER JOIN dbo.AssignMissions AM WITH (NOLOCK) ON AMD.CharacterID = AM.CharacterID AND AMD.MissionTypeCode = AM.MissionTypeCode
	WHERE AM.CharacterID = @inbCharacterID AND AM.MissionTypeCode = 1;
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
