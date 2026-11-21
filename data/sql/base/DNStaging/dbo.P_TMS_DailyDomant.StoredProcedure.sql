USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_DailyDomant]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-08
description : (TIMOSE) ?? ??, ?? ?? (dbo.ST_DailyDomant)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ????by ??? at 2013-04-08
**/

CREATE PROCEDURE [dbo].[P_TMS_DailyDomant]
	@datBasisDate date
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
	SELECT BasisDate, RegisterCode, D7LeaveCount, D15LeaveCount, D30LeaveCount, D7ReviveCount, D15ReviveCount, D30ReviveCount, NULL AS PublisherID, NULL AS NationalityCode, NULL AS PartitionID
	  FROM dbo.ST_DailyDomant WITH (NOLOCK)
	 WHERE BasisDate >= DATEADD(DD, -30, @datBasisDate) AND BasisDate < DATEADD(DD, -29, @datBasisDate);
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
