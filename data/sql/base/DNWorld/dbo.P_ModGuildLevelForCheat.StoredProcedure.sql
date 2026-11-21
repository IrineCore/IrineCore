USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildLevelForCheat]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-03
description : ?? ?? ? ???, ?????? ?????.(???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ModGuildLevelForCheat]
	  @intGuildID int --// ?? ID
	, @inyCheatType tinyint --//?? ?? 1=????, 2=????? ???
	, @intGuildPoint int --// ??? ?? ???
    , @inyGuildLevel tinyint --//??? ?? ?? 
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
	IF @inyCheatType = 1 -- ????
		UPDATE dbo.Guilds	
		SET TotalGuildEXP = @intGuildPoint
          , GuildLevel = @inyGuildLevel
		FROM dbo.Guilds WITH (INDEX(PK_Guilds), FORCESEEK)
		WHERE GuildID = @intGuildID
		OPTION (KEEPFIXED PLAN);
	ELSE IF @inyCheatType = 2 -- ????? ???
		UPDATE dbo.Guilds	
		SET DailyStagePoint = 0
          , DailyMissionPoint = 0
		FROM dbo.Guilds WITH (INDEX(PK_Guilds), FORCESEEK)
		WHERE GuildID = @intGuildID
		OPTION (KEEPFIXED PLAN);

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
