USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_GetListIndexUsages]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-03
result set :
description : (Timose) ??? DMV ?? ??
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_TMS_GetListIndexUsages]
	  @dtBasisDate date --// ?? ??
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
	SELECT A.BasisDate, A.ServerName, A.DatabaseName, A.TableName, A.IndexName
		, (B.UserScans - A.UserScans) AS UserScans
		, (B.UserSeeks - A.UserSeeks) AS UserSeeks	
		, (B.UserUpdates - A.UserUpdates) AS UserUpdates	
		, (B.UserLookups - A.UserLookups) AS UserLookups
		, NULL AS PublisherID
	    , NULL AS NationalityCode
	    , NULL AS PartitionID
	    , NULL AS WorldID
	FROM (SELECT * FROM dbo.IndexUsages WITH(NOLOCK) WHERE BasisDate = @dtBasisDate) A
		CROSS JOIN (SELECT * FROM dbo.IndexUsages WITH(NOLOCK) WHERE BasisDate = DATEADD(DAY, 1, @dtBasisDate)) B
	WHERE A.BasisDate = DATEADD(DAY, -1, B.BasisDate) 
	  AND A.ServerName = B.ServerName 
	  AND A.DatabaseName = B.DatabaseName
	  AND A.TableName = B.TableName
	  AND A.IndexName = B.IndexName
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
