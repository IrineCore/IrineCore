USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarFinalResultsMatchList]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-10

description :
    ??? ?? ??? ??
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    3 = ?? ??
	6 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarFinalResultsMatchList]
	  @insGuildWarScheduleID smallint --// ??? ??
	, @vchGuildID varchar(max) --// ?? ID (???=',')
	, @vchMatchSequence varchar(max) --// ??? (???=',')
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

	/**_# ??? ?? ???? ?????. */
	INSERT dbo.GuildWarFinalResults (GuildID, GuildWarScheduleID, MatchSequence, MatchTypeCode, MatchResultCode, RegisterDate, ChangeDate)
    SELECT A.string, @insGuildWarScheduleID, B.string, 0, 0, GETDATE(), GETDATE()
    FROM dbo.TF_Split(@vchGuildID, ',') A
		INNER JOIN dbo.TF_Split(@vchMatchSequence, ',') B ON (A.seq = B.seq)
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

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
