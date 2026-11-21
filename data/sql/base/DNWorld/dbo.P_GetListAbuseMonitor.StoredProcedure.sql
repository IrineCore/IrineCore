USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAbuseMonitor]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-20
description : Abuse Monitor ??
result set : CharacterID/bigint, CharacterName/nvarchar, AbuseCount1/int, AbuseCount2/int, PlayRestraint/int, ResetRestraint/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = Tuning. Plan ??     by 20110601.wgkim
5 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
**/

CREATE PROCEDURE [dbo].[P_GetListAbuseMonitor]
	  @intStart int --// Range ?? ?
	, @intEnd int --// Range ? ?
	, @inyWhichValue tinyint --// 1=AbuseCount1, 2=AbuseCount2
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intPlayRestraint int
	, @intResetRestraint int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	EXEC dbo.P_GetRestraintConstant
		  @intPlayRestraint = @intPlayRestraint OUTPUT
		, @intResetRestraint = @intResetRestraint OUTPUT;

	IF @inyWhichValue = 1
		SELECT CH.CharacterID, CH.CharacterName, A.AbuseCount1, A.AbuseCount2, ISNULL(A.PlayRestraint, @intPlayRestraint) AS PlayRestraint, ISNULL(A.ResetRestraint, @intResetRestraint) AS ResetRestraint
		FROM dbo.AbuseMonitor A WITH (NOLOCK, INDEX(IX_NNI_AbuseMonitor_AbuseCount1))
			INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters)) ON A.CharacterID = CH.CharacterID
		WHERE A.AbuseCount1 BETWEEN @intStart AND @intEnd
		ORDER BY A.AbuseCount1 DESC
		OPTION (MAXDOP 1, FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	ELSE
		SELECT CH.CharacterID, CH.CharacterName, A.AbuseCount1, A.AbuseCount2, ISNULL(A.PlayRestraint, @intPlayRestraint) AS PlayRestraint, ISNULL(A.ResetRestraint, @intResetRestraint) AS ResetRestraint
		FROM dbo.AbuseMonitor A WITH (NOLOCK, INDEX(IX_NNI_AbuseMonitor_AbuseCount2))
			INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters)) ON A.CharacterID = CH.CharacterID
		WHERE A.AbuseCount2 BETWEEN @intStart AND @intEnd
		ORDER BY A.AbuseCount2 DESC
		OPTION (MAXDOP 1, FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
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
