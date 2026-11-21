USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_JobAndLevelType4]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-10-05

description :
	(TIMOSE) ?? ?? ???? ??, ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ????
	3 = ST_ ???? ?? ?? by dailykim.20111227
**/

CREATE PROCEDURE [dbo].[P_TMS_JobAndLevelType4]
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

	SELECT BasisDate
         , ISNULL(WorldID, 0) AS WorldID
	     , JobCode
	     , CharacterLevel
	     , CharacterCount
		 , NULL AS PublisherID
	     , NULL AS NationalityCode
	     , NULL AS PartitionID
	  FROM dbo.ST_JobAndLevelType4 WITH (NOLOCK)
	 WHERE (BasisDate >= CONVERT(smalldatetime, @dtBasisDate))
	   AND (BasisDate < CONVERT(smalldatetime, DATEADD(day, 1, @dtBasisDate)))
	OPTION (MAXDOP 1);

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
