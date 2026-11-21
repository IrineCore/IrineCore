USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildWarRewardType]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-12-19
description : ??? ?? ??(??)? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetGuildWarRewardType]
	  @insGuildWarScheduleID smallint --// ??? ??
    , @insGuildWarRewardType smallint = NULL OUTPUT --// ??? ?? ????(0,30,45)
   
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

	/**_# ??? ?? ?? ??? ?????. ???? ??? ?? 0? ??.*/
	SELECT @insGuildWarRewardType = ISNULL(DATEDIFF(DD, RewardEndDate, RewardExpirationDate),0) 
    FROM dbo.GuildWarSchedule WITH (NOLOCK)
    WHERE GuildWarScheduleID = @insGuildWarScheduleID;
	
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
