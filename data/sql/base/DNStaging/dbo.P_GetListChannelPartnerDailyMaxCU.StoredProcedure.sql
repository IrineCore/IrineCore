USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListChannelPartnerDailyMaxCU]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-08-12
description : ?? ??? ?????.(???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListChannelPartnerDailyMaxCU]
	  @sdtStartDate smalldatetime --// ?? ?? ??
	, @sdtEndDate smalldatetime --// ?? ? ??
	, @inyChannelPartnerCode tinyint
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

	SELECT BasisDate, LogDate, SUM(CUCount) AS CUCount
	FROM dbo.ST_DailyMaxCU WITH(NOLOCK)
	WHERE BasisDate BETWEEN @sdtStartDate AND @sdtEndDate
	  AND ChannelPartnerCode = @inyChannelPartnerCode
	GROUP BY BasisDate, LogDate;
	
END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
