USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListEventGroups]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-07-06
description : ??? ?? ???? ?? ???.
result set : EventGroupID/int, EventName/nvarchar, EventStartDate/datatime, EventEndDate/datatime, EventDescription/nvarchar, RegisterID/int, RegisteDate/datetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListEventGroups]
	  @inyEventScheduleCode tinyint = NULL --// ??? ??? ?? ??(NULL = ??, 1=???, 2=??, 3=??)	
	, @intRegisterID int = NULL --// ??? ??? ID	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @sql varchar(MAX);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @sql = 
	' 
	DECLARE @Now datetime;
	SET @Now = GETDATE();

	SELECT EventGroupID, EventName, EventStartDate, EventEndDate, EventDescription, RegisterID, RegisteDate
	FROM dbo.EventGroups WITH (NOLOCK)
	WHERE EventGroupID IS NOT NULL ';

	IF @inyEventScheduleCode = 1
		SET @sql = @sql + ' AND (EventStartDate <= @Now AND EventEndDate > @Now) ';

	IF @inyEventScheduleCode = 2
		SET @sql = @sql + ' AND EventStartDate > @Now ';

	IF @inyEventScheduleCode = 3
		SET @sql = @sql + ' AND EventStartDate < @Now ';

	IF @intRegisterID IS NOT NULL 
		SET @sql = @sql + ' AND RegisterID = ' + Cast(@intRegisterID AS varchar);

	EXEC(@sql);
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103177;
		GOTO ErrorHandler;
	END

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
