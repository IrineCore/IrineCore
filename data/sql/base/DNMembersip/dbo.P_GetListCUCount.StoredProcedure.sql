USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCUCount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListCUCount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListCUCount] AS' 
END
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-04
description : ????? ??? ?????.
result set : SnapshotDate, CUCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListCUCount]
	  @dt2StartDate datetime2(0) = NULL --// ?? ?? ??.
	, @dt2EndDate datetime2(0) = NULL --// ?? ? ??.
	, @intWorldID int = NULL --// ?? ID. ?? ?? ??? NULL ?????.
	, @bitTop1 bit = 0 --// ?? ?? ?? ?? ??
	, @inyChannelPartnerCode tinyint = NULL --// ?? ??? ??
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
	IF @bitTop1 = 1
		SELECT TOP (1) @dt2StartDate = SnapshotDate, @dt2EndDate = SnapshotDate
		FROM dbo.CUCount WITH (NOLOCK)
		WHERE WorldID = COALESCE(@intWorldID, WorldID) AND ChannelPartnerCode = COALESCE(@inyChannelPartnerCode, ChannelPartnerCode)
		ORDER BY SnapshotDate DESC;

	ELSE
	BEGIN
		SET @dt2StartDate = CASE WHEN @dt2StartDate IS NULL THEN '2010-01-01' ELSE @dt2StartDate END;
		SET @dt2EndDate = CASE WHEN @dt2EndDate IS NULL THEN '2050-01-01' ELSE @dt2EndDate END;
	END

	IF @intWorldID IS NOT NULL
		SELECT SnapshotDate, CUCount
		FROM dbo.CUCount WITH (NOLOCK)
		WHERE SnapshotDate BETWEEN @dt2StartDate AND @dt2EndDate AND WorldID = @intWorldID AND ChannelPartnerCode = COALESCE(@inyChannelPartnerCode, ChannelPartnerCode);

	ELSE
		SELECT SnapshotDate, SUM(CUCount) AS CUCount
		FROM dbo.CUCount WITH (NOLOCK)
		WHERE SnapshotDate BETWEEN @dt2StartDate AND @dt2EndDate AND ChannelPartnerCode = COALESCE(@inyChannelPartnerCode, ChannelPartnerCode)
		GROUP BY SnapshotDate;

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
