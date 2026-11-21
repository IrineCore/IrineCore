USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_WeeklyStageEnd]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-05-03
description : (TIMOSE) ?? ???? ??? (dbo.ST_WeeklyStageEnd)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ????by ??? at 2013-05-03
**/

CREATE PROCEDURE [dbo].[P_TMS_WeeklyStageEnd]
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
	SELECT BasisDate, WorldID, MapID, ClearCount, EntranceCount, CharacterCount, PlayTimeCode, NULL AS PublisherID, NULL AS NationalityCode, NULL AS PartitionID
	  FROM dbo.ST_WeeklyStageEnd WITH (NOLOCK)
	 WHERE BasisDate >= @datBasisDate AND BasisDate < DATEADD(DD, 1, @datBasisDate);
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
