USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_CancelGuildWarFinal]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-11-05
description : ???? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_CancelGuildWarFinal]	  
	  @insGuildWarScheduleID smallint --// ??? ??	
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
	
	BEGIN TRAN;

	/**_# [CanceledGuildWarFinalResults] ???? INSERT???.*/
	IF EXISTS (SELECT * FROM dbo.CanceledGuildWarFinalResults WITH (NOLOCK) WHERE GuildWarScheduleID = @insGuildWarScheduleID)
		DELETE dbo.CanceledGuildWarFinalResults WHERE GuildWarScheduleID = @insGuildWarScheduleID;

	INSERT dbo.CanceledGuildWarFinalResults (GuildID, GuildWarScheduleID, MatchSequence, MatchTypeCode, MatchResultCode, RegisterDate, ChangeDate)
	SELECT GuildID, GuildWarScheduleID, MatchSequence, MatchTypeCode, MatchResultCode, RegisterDate, ChangeDate
	  FROM dbo.GuildWarFinalResults
	 WHERE GuildWarScheduleID = @insGuildWarScheduleID;

	
	/**_# ????? ?? ??? ??? ???.*/
	
	UPDATE dbo.GuildWarFinalResults
	   SET MatchSequence = 0, MatchTypeCode = 0, MatchResultCode = 0, ChangeDate = GETDATE()
	 WHERE GuildWarScheduleID = @insGuildWarScheduleID;

	/**_# ?? ??? ???? ?? ????? ?????.*/

	UPDATE dbo.GuildWarSchedule
	   SET FinalProgressFlag = 0
	 WHERE GuildWarScheduleID = @insGuildWarScheduleID;

	COMMIT TRAN;
	
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
