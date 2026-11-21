USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListSubRestraintReason]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : ?? "?? ??" ??? ?????.
result set : RestraintReasonID, RestraintReasonLevel, RestraintReasonName, UsedFlag
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102102 = ?? ?? ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListSubRestraintReason]
	  @intRestraintReasonID int --// ?? ?? ID
WITH EXECUTE AS 'SPExecutor'
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
	SELECT C.RestraintReasonID, C.RestraintReasonLevel, C.RestraintReasonName, C.UsedFlag
	FROM dbo.RestraintReasons P WITH (NOLOCK)
		INNER JOIN dbo.RestraintReasons C WITH (NOLOCK) ON C.RestraintReasonNode.GetAncestor(1) = P.RestraintReasonNode
	WHERE P.RestraintReasonID = @intRestraintReasonID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 57011017;
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
