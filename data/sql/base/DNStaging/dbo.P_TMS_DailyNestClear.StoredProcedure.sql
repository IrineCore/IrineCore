USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_DailyNestClear]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-12-07

description :
	(TIMOSE) ??? ?? ?? (dbo.ST_DailyNestClear)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1 = ????
**/

CREATE PROCEDURE [dbo].[P_TMS_DailyNestClear]
	@dtBasisDate date
WITH EXECUTE AS 'SPExecutor'
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

	SELECT BasisDate, MapID, JobCode, CharacterLevel, ChallengeCount, ClearCount, NULL AS PublisherID, NULL AS NationalityCode, NULL AS PartitionID, NULL AS WorldID
	  FROM dbo.ST_DailyNestClear WITH (NOLOCK)
	 WHERE (BasisDate >= @dtBasisDate)
	   AND (BasisDate < DATEADD(day, 1, @dtBasisDate));

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
