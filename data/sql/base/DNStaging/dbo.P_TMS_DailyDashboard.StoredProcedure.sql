USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_DailyDashboard]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-12-07

description :
	(TIMOSE) ?? ?? ?? (dbo.ST_DailyDashboard)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1~2 = ????
	4 = PC? ?? ?? (UVPCRoom, DTForPCRoom) by ??? at 2013-06-17
**/

CREATE PROCEDURE [dbo].[P_TMS_DailyDashboard]
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

	SELECT BasisDate, RUCount, UVCount, RevisitType1, RevisitType2, AvgDT, AvgDTForNewbie, SaleAmount, NationalityCode, NULL AS PublisherID, NULL AS PartitionID, NULL AS WorldID, LeaveAccountCount, RevisitCount3
		, UVPCRoom, DTForPCRoom
	  FROM dbo.ST_DailyDashboard WITH (NOLOCK)
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
