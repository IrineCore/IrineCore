USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAbuseMonitorCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-06-22
description : ?? ??? ???? ?????.
result set : CharacterID/bigint, AbuseCount1/int, AbuseCount2/int, PlayRestraint/int, ResetRestraint/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
**/

CREATE PROCEDURE [dbo].[P_GetListAbuseMonitorCount]
	  @intAccountID bigint --// ?? ID
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
	SELECT AM.CharacterID, AM.AbuseCount1, AM.AbuseCount2
		, COALESCE(AM.PlayRestraint, AC.PlayRestraint) AS PlayRestraint
		, COALESCE(AM.ResetRestraint, AC.ResetRestraint) AS ResetRestraint
	FROM dbo.Characters CH WITH (NOLOCK, INDEX(IX_NNI_Characters_AccountID), FORCESEEK)
		INNER JOIN dbo.AbuseMonitor AM WITH (NOLOCK, INDEX(PK_AbuseMonitor), FORCESEEK) ON CH.CharacterID = AM.CharacterID
		CROSS JOIN dbo.AbuseMonitorRestraintConstants AC WITH (NOLOCK)
	WHERE CH.AccountID = @intAccountID AND CH.DeleteFlag = CAST(0 AS bit) AND AM.AbuseCount1 > 0;;
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
