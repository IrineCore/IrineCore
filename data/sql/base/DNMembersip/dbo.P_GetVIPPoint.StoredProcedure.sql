USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetVIPPoint]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetVIPPoint]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetVIPPoint] AS' 
END
GO
/**
version : 10
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-31
description : VIP ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
10 = AdjustedPoint ?? ??? ?? ?? at 2010-07-07
**/

ALTER PROCEDURE [dbo].[P_GetVIPPoint]
	  @inbCharacterID bigint --// ??? ID
	, @intVIPTotalPoint int OUTPUT --// ? VIP ???
	, @dtsVIPStartDate smalldatetime OUTPUT --// VIP ?? ?? ??
	, @dtsVIPEndDate smalldatetime OUTPUT --// VIP ?? ? ??
	, @bitAutoPayFlag bit OUTPUT --// 0=????, 1=????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtmNow datetime = GETDATE(), @intBasicPoint int, @intIncrementalPoint int, @intDecreasedPoint int, @intAdjustedPoint int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	UPDATE dbo.VIPPoints
	SET @intBasicPoint = BasicPoint
		, @intAdjustedPoint = AdjustedPoint
		, @dtsVIPStartDate = VIPStartDate
		, @dtsVIPEndDate = VIPEndDate
		, @bitAutoPayFlag = AutoPayFlag
		, @intIncrementalPoint = IncrementalPoint += 
		CASE
			WHEN LastCalculatedDate >= CAST(@dtmNow AS date) OR LastCalculatedDate >= CAST(VIPEndDate AS date) OR DATEDIFF(DAY, LastCalculatedDate, CASE WHEN CAST(@dtmNow AS date) > CAST(VIPEndDate AS date) THEN VIPEndDate ELSE @dtmNow END) < 0 THEN 0
			ELSE
				CASE
					WHEN DATEDIFF(DAY, LastCalculatedDate, CASE WHEN CAST(@dtmNow AS date) > CAST(VIPEndDate AS date) THEN VIPEndDate ELSE @dtmNow END) * 10 > (10000 - BasicPoint - IncrementalPoint + DecreasedPoint) THEN (10000 - BasicPoint - IncrementalPoint + DecreasedPoint)
					ELSE DATEDIFF(DAY, LastCalculatedDate, CASE WHEN CAST(@dtmNow AS date) > CAST(VIPEndDate AS date) THEN VIPEndDate ELSE @dtmNow END) * 10
				END
		END
	WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		COMMIT TRANSACTION;

		SET @intVIPTotalPoint = 0;
		SET @dtsVIPStartDate = NULL;
		SET @dtsVIPEndDate = NULL;
		SET @bitAutoPayFlag = 0;

		RETURN 0;
	END

	UPDATE dbo.VIPPoints
	SET LastCalculatedDate = @dtmNow
		, @intDecreasedPoint = DecreasedPoint +=
		CASE
			WHEN LastCalculatedDate >= CAST(@dtmNow AS date) OR DATEDIFF(DAY, CASE WHEN LastCalculatedDate > VIPEndDate THEN LastCalculatedDate ELSE VIPEndDate END, @dtmNow) < 0 THEN 0
			ELSE
				CASE
					WHEN DATEDIFF(DAY, CASE WHEN LastCalculatedDate > CAST(VIPEndDate AS date) THEN LastCalculatedDate ELSE DATEADD(DAY, 1, VIPEndDate) END, @dtmNow) * 15 > (BasicPoint + IncrementalPoint - DecreasedPoint) THEN (BasicPoint + IncrementalPoint - DecreasedPoint)
					ELSE DATEDIFF(DAY, CASE WHEN LastCalculatedDate > CAST(VIPEndDate AS date) THEN LastCalculatedDate ELSE DATEADD(DAY, 1, VIPEndDate) END, @dtmNow) * 15
				END
		END
	WHERE CharacterID = @inbCharacterID;

	SET @intVIPTotalPoint = @intAdjustedPoint + @intBasicPoint + @intIncrementalPoint - @intDecreasedPoint;

	COMMIT TRANSACTION;
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
