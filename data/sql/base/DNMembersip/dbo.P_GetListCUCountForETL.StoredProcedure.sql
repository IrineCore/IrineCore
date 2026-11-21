USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCUCountForETL]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListCUCountForETL]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListCUCountForETL] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-23
description : ????? ??? ?????.
result set : SnapshotDate, WorldID, ChannelPartnerCode, CUCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListCUCountForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2BasisDate datetime2(0) = CAST(CAST(@dtmBasisDate AS date) AS datetime2(0));

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT SnapshotDate, WorldID, ChannelPartnerCode, SUM(CUCount) AS CUCount
	FROM dbo.CUCount WITH (NOLOCK)
	WHERE SnapshotDate >= @dt2BasisDate AND SnapshotDate < DATEADD(DAY, 1, @dt2BasisDate)
	GROUP BY SnapshotDate, WorldID, ChannelPartnerCode;
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
