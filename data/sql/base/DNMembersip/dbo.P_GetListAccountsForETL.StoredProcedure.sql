USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountsForETL]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListAccountsForETL]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListAccountsForETL] AS' 
END
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-20
description : ?? ??? ?????.(DailyETL ?)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListAccountsForETL]
	   @dtmBasisDate datetime --// ?? ?? 
--WITH EXECUTE AS 'SPExecutor'
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
	SELECT AccountID, AccountName, JoinIP, RegisterDate, GenderCode, BirthDate, NationalityCode, PublisherCode, ChannelPartnerCode
	FROM dbo.Accounts WITH (NOLOCK)
	WHERE RegisterDate >= @dtmBasisDate AND RegisterDate < DATEADD(DAY, 1, @dtmBasisDate);
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
