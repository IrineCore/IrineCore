USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_IncreaseVIPBasicPoint]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_IncreaseVIPBasicPoint]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_IncreaseVIPBasicPoint] AS' 
END
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-31
description : VIP ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103293 = ?? ??? PurchaseOrderID ????.
history :
3 = VIPPointLogs ???? PK ?? ?? ??? 0? ????? ?? at 2010-12-28
5 = VIPPointLogs ???? PK ?? ?? ??? 103293? ????? ?? at 2010-12-31
6 = Point ? 0 ? ???? ??/??? ?? ??? ??     by ??? at 2012-02-13
7 = 6? ?? ??     by ??? at 2012-02-29
**/

ALTER PROCEDURE [dbo].[P_IncreaseVIPBasicPoint]
	  @inbCharacterID bigint --// ??? ID
	, @intBasicPoint int --// VIP ??? ?? ??? ?? ?? ???
	, @inbPurchaseOrderID bigint --// ??? ?? ?? ID
	, @intVIPPeriod int --// VIP ??. ?? : Day. 30?, 90?, 180?, 360?
	, @bitAutoPayFlag bit --// 0=????, 1=????
	, @intVIPTotalPoint int OUTPUT --// ? VIP ???
	, @dtsVIPStartDate smalldatetime OUTPUT --// VIP ?? ?? ??
	, @dtsVIPEndDate smalldatetime OUTPUT --// VIP ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtmNow datetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	BEGIN TRANSACTION;

	UPDATE dbo.VIPPoints
	SET BasicPoint += @intBasicPoint
		, VIPStartDate = CASE WHEN VIPEndDate < @dtmNow THEN @dtmNow ELSE VIPStartDate END
		, VIPEndDate = CASE WHEN VIPEndDate < @dtmNow THEN DATEADD(DAY, @intVIPPeriod, @dtmNow) ELSE DATEADD(DAY, @intVIPPeriod, VIPEndDate) END
		, AutoPayFlag = @bitAutoPayFlag
	WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
		INSERT dbo.VIPPoints (CharacterID, BasicPoint, IncrementalPoint, DecreasedPoint, VIPStartDate, VIPEndDate, LastCalculatedDate, AutoPayFlag)
		VALUES (@inbCharacterID, @intBasicPoint, 0, 0, @dtmNow, @dtmNow + @intVIPPeriod, @dtmNow, @bitAutoPayFlag);

	INSERT dbo.VIPPointLogs (PurchaseOrderID, CharacterID, BasicPoint)
	VALUES (@inbPurchaseOrderID, @inbCharacterID, @intBasicPoint);

	COMMIT TRANSACTION;

	EXEC dbo.P_GetVIPPoint
		  @inbCharacterID = @inbCharacterID
		, @intVIPTotalPoint = @intVIPTotalPoint OUTPUT
		, @dtsVIPStartDate = @dtsVIPStartDate OUTPUT
		, @dtsVIPEndDate = @dtsVIPEndDate OUTPUT
		, @bitAutoPayFlag = @bitAutoPayFlag OUTPUT;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_VIPPointLogs%'
		SET @intReturnValue = 103293;

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
