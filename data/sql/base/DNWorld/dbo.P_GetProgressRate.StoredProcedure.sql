USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetProgressRate]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-03
description : ?? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = VIX_TargetCharacterDetails ? ?? ?? ?? ?? at 2010-08-19
5 = VIX_TargetCharacterDetails ?? ???? ?? ?? Count? -1? ????? ?? at 2010-08-22
**/

CREATE PROCEDURE [dbo].[P_GetProgressRate]
	  @intCompleteCount int OUTPUT --// ?? ?
	, @intTotalCount int OUTPUT --// ?? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intTemp int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF OBJECT_ID(N'VIX_TargetCharacterDetails', N'V') IS NULL
	BEGIN
		SET @intCompleteCount = -1;
		SET @intTotalCount = -1;

		RETURN 0;
	END

	SELECT @intTemp = ISNULL(MAX(CASE CompleteFlag WHEN 0 THEN Cnt END), 0)
		, @intCompleteCount = ISNULL(MAX(CASE CompleteFlag WHEN 1 THEN Cnt END), 0)
	FROM dbo.VIX_TargetCharacterDetails WITH (NOEXPAND, NOLOCK);

	SET @intTotalCount = @intTemp + @intCompleteCount;
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
