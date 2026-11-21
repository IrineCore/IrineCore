USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDailyMapVisit]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-12-01
description : ??? ?? ????? ?????.
result set : BasisDate, MapID, UVCount, VisitCount, AvgDT
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDailyMapVisit]
	  @datBasisDate date --// ?? ?? ??	
	, @intMapID int = NULL --// ? ID	
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
	/**_# [ST_DailyMapVisit] ???? ??? Select???.*/
	SELECT BasisDate, MapID, UVCount, VisitCount, AvgDT
	  FROM dbo.ST_DailyMapVisit WITH (NOLOCK)
	 WHERE BasisDate = @datBasisDate
       AND MapID = COALESCE(@intMapID, MapID)
	 ORDER BY BasisDate;
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
