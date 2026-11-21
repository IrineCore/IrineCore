USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_HourlyMaxCU]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-10-05

description :
	(TIMOSE) ??? ?? ?? (dbo.ST_HourlyMaxCU)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1~5 = ????
	6 = ??, ?? ?? ?? by ??? at 2013-04-08
**/

CREATE PROCEDURE [dbo].[P_TMS_HourlyMaxCU]
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

	SELECT BasisDate, WorldID, CUCount, LogDate, NULL AS PublisherID, NULL AS NationalityCode, NULL AS PartitionID, CUCountMIN, CUCountAVG
	  FROM dbo.ST_HourlyMaxCU WITH (NOLOCK)
	 WHERE (BasisDate >= CONVERT(smalldatetime, @dtBasisDate))
	   AND (BasisDate < CONVERT(smalldatetime, DATEADD(day, 1, @dtBasisDate)))
	   AND ChannelPartnerCode IS NULL;
	   
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
