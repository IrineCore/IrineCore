USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_DailySalesByProduct]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-10-05

description :
	(TIMOSE) ??? ? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ????
**/

CREATE PROCEDURE [dbo].[P_TMS_DailySalesByProduct]
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

	SELECT NULL AS PublisherID
	     , ISNULL(NationalityCode, 0) AS NationalityCode
	     , NULL AS PartitionID
	     , NULL AS WorldID
	     , BasisDate
	     , ProductID
	     , UniqueBuyerCount
	     , UniqueCharacterCount
	     , SaleCount
	     , SaleAmount
	  FROM dbo.ST_DailySalesByProduct WITH (NOLOCK)
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
