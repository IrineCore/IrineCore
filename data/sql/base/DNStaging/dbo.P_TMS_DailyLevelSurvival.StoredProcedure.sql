USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_DailyLevelSurvival]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2012-10-15

description :
	(TIMOSE) ??? ??? (dbo.ST_DailyLevelSurvival)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1 = ????
	2~4 = ?? ??? ??? ?? ?? by ??? at 2012-11-29
**/

CREATE PROCEDURE [dbo].[P_TMS_DailyLevelSurvival]
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
	IF @dtBasisDate >= '2012-12-07 00:00' AND @dtBasisDate < '2012-12-07 00:00'
	BEGIN
		SELECT BasisDate, CharacterLevel, CreateMonth, RU, UVDay, UVWeek, UVMonth, RevisitCount, NationalityCode, NULL AS PublisherID, NULL AS PartitionID, NULL AS WorldID
		  FROM dbo.ST_DailyLevelSurvival WITH (NOLOCK);
	END
	ELSE
	BEGIN
		SELECT BasisDate, CharacterLevel, CreateMonth, RU, UVDay, UVWeek, UVMonth, RevisitCount, NationalityCode, NULL AS PublisherID, NULL AS PartitionID, NULL AS WorldID
		  FROM dbo.ST_DailyLevelSurvival WITH (NOLOCK)
		 WHERE (BasisDate >= @dtBasisDate)
		   AND (BasisDate < DATEADD(day, 1, @dtBasisDate));
	END
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
