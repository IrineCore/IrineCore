USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModVIPPoint]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModVIPPoint]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModVIPPoint] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-07
description : VIP ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101167 = VIP ???? ????.
history :
3 = input parameter @intVIPPoint ?? ?? at 2010-07-09
**/

ALTER PROCEDURE [dbo].[P_ModVIPPoint]
	  @inbCharacterID bigint --// ??? ID
	, @intVIPPoint int --// ??? VIP ??? (?? : 10,000)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intVIPTotalPoint int
	, @intAdjustedPoint int
	, @dtsVIPStartDate smalldatetime
	, @dtsVIPEndDate smalldatetime
	, @bitAutoPayFlag bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intVIPPoint = CASE WHEN @intVIPPoint > 10000 THEN 10000 WHEN @intVIPPoint < 0 THEN 0 ELSE @intVIPPoint END;

	EXEC @intReturnValue = dbo.P_GetVIPPoint
		  @inbCharacterID = @inbCharacterID
		, @intVIPTotalPoint = @intVIPTotalPoint OUTPUT
		, @dtsVIPStartDate = @dtsVIPStartDate OUTPUT
		, @dtsVIPEndDate = @dtsVIPEndDate OUTPUT
		, @bitAutoPayFlag = @bitAutoPayFlag OUTPUT;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	SET @intAdjustedPoint = @intVIPPoint - @intVIPTotalPoint;

	UPDATE dbo.VIPPoints
	SET AdjustedPoint += @intAdjustedPoint
	WHERE CharacterID = @inbCharacterID;
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
