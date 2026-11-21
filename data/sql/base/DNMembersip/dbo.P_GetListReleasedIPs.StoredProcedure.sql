USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListReleasedIPs]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListReleasedIPs]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListReleasedIPs] AS' 
END
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-04-10
description : ??? ??? IP ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListReleasedIPs]
	  @vchStartIP varchar(15) --// ??? IP ?? ?? IP
	, @vchEndIP varchar(15) --// ??? IP ?? ? IP
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE	
	@intStartIP int,
	@intEndIP int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END
BEGIN TRY
	SET @intStartIP = ISNULL(dbo.FN_IP2IPNumber(@vchStartIP), dbo.FN_IP2IPNumber(@vchEndIP));
	SET @intEndIP = ISNULL(dbo.FN_IP2IPNumber(@vchEndIP), @intStartIP);

	/**_# [ReleasedIPs] ????? Select ???.*/
	
	SELECT dbo.FN_IPNumber2IP(StartIP) AS StartIP, dbo.FN_IPNumber2IP(EndIP) AS EndIP, BlockedDate, BlockedDescription, ReleasedDate
	FROM dbo.ReleasedIPs WITH (NOLOCK)
	WHERE ((@intStartIP BETWEEN StartIP AND EndIP)
	OR (@intEndIP BETWEEN StartIP AND EndIP)
	OR (StartIP BETWEEN @intStartIP AND @intEndIP)
	OR (EndIP BETWEEN @intStartIP AND @intEndIP))

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
