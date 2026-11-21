USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildEXP]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-24
description : ?? ???? ?????.
result set : TotalGuildEXP/int, DailyStagePoint/int, DailyMissionPoint/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetGuildEXP]
	  @intGuildID int --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Today datetime2(0) = CAST(GETDATE() AS date);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	SELECT TotalGuildEXP
		, CASE WHEN StagePointLastModDate >= @dt2Today THEN DailyStagePoint ELSE 0 END AS DailyStagePoint
		, CASE WHEN MissionPointLastModDate >= @dt2Today THEN DailyMissionPoint ELSE 0 END AS DailyMissionPoint
	FROM dbo.Guilds WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK)
	WHERE GuildID = @intGuildID
	OPTION(KEEPFIXED PLAN);

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
