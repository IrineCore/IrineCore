USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_DailyItemCount]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-09
description : (TIMOSE) ?? ??? ??/??/??? (dbo.ST_DailyItemCount)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-04-09
3 = ???? by ??? at 2013-04-29
**/

CREATE PROCEDURE [dbo].[P_TMS_DailyItemCount]
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
	SELECT BasisDate, WorldID, CharacterLevel, JobCode, ItemID, ItemLevel, ItemUseCode, ItemOptionGroupCode, ActiveUserFlag, ItemLocationCode, ItemCount, NULL AS PublisherID, NULL AS NationalityCode, NULL AS PartitionID, ItemCategoryID
	FROM dbo.ST_DailyItemCount WITH (NOLOCK)
	WHERE BasisDate >= @datBasisDate AND BasisDate < DATEADD(DD, 1, @datBasisDate)
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
