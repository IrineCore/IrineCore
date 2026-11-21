USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_DelRestraintReason]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
	
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102103 = ??? ?? ??? ???? ????.
102104 = ?? ?? ??? ???? ??? ? ????.
102107 = ?? ??? ?? ?????. ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DelRestraintReason]
	  @intRestraintReasonID int --// ?? ?? ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @hidRestraintReasonNode hierarchyid
	, @bitUsedFlag bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @hidRestraintReasonNode = RestraintReasonNode, @bitUsedFlag = UsedFlag FROM dbo.RestraintReasons WITH (NOLOCK) WHERE RestraintReasonID = @intRestraintReasonID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 102103;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ?? ???? 102107? ?????.*/
	IF @bitUsedFlag = 1
	BEGIN
		SET @intReturnValue = 102107;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ??? ???? 102104? ?????.*/
	IF EXISTS (
		SELECT * FROM dbo.RestraintReasons WITH (NOLOCK) WHERE RestraintReasonNode.GetAncestor(1) = @hidRestraintReasonNode
	)
	BEGIN
		SET @intReturnValue = 102104;
		GOTO ErrorHandler;
	END

	/**_# [RestraintReasons] ????? DELETE ???.*/
	DELETE dbo.RestraintReasons WHERE RestraintReasonID = @intRestraintReasonID AND UsedFlag = CAST(0 AS bit);

	/**_# ??? ?? ??? 102103? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 102103;
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
