USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFarm]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-13
description : ?? ??? ?????.
result set : 
	@intVersion = 2 : FarmID, FarmName, MapID, MaxUserCount, EnableFlag
	@intVersion = 3 : FarmID, FarmName, MapID, MaxUserCount, EnableFlag, ActivationFlag
	@intVersion >= 4 : FarmID, FarmName, MapID, MaxUserCount, EnableFlag, ActivationFlag, FarmTypeCode
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103279 = ??? ???? ????.
history :
3 = input param. @intVersion ?? at 2010-12-16
4 = @intVersion : 4 ?? at 2011-05-17
**/

CREATE PROCEDURE [dbo].[P_GetListFarm]
	  @bitEnableFlag bit --// 0=Disable, 1=Enable. NULL? ???? ?? ??? ??? ?????.
	, @intVersion int = 2 --// ??? SP? ??.
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
	/**_# [Farms] ???? SELECT???.*/
	IF @intVersion = 2
		SELECT FarmID, FarmName, MapID, MaxUserCount, EnableFlag
		FROM dbo.Farms WITH (NOLOCK)
		WHERE EnableFlag = COALESCE(@bitEnableFlag, EnableFlag)
		ORDER BY MapID, FarmID
		OPTION (KEEPFIXED PLAN);

	ELSE IF @intVersion = 3
		SELECT FarmID, FarmName, MapID, MaxUserCount, EnableFlag, ActivationFlag
		FROM dbo.Farms WITH (NOLOCK)
		WHERE EnableFlag = COALESCE(@bitEnableFlag, EnableFlag)
		ORDER BY MapID, FarmID
		OPTION (KEEPFIXED PLAN);

	ELSE
		SELECT FarmID, FarmName, MapID, MaxUserCount, EnableFlag, ActivationFlag, FarmTypeCode
		FROM dbo.Farms WITH (NOLOCK)
		WHERE EnableFlag = COALESCE(@bitEnableFlag, EnableFlag)
		ORDER BY MapID, FarmID
		OPTION (KEEPFIXED PLAN);

	/**_## ??? ???? ??? 103279? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103279;
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
