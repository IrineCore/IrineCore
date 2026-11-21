USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_GetTreeRestraintReason]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
		
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : "?? ??" ??? ?????.
result set : RestraintReasonID, RestraintReasonLevel, RestraintReasonName, UsedFlag
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102102 = ?? ?? ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetTreeRestraintReason]
	  @intRestraintReasonID int --// ?? ?? ID
	, @inyDirection tinyint --// ?? ?? ?? : 1=root?? branch, 2=branch?? root
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
	IF @inyDirection = 1
		SELECT C.RestraintReasonID, C.RestraintReasonLevel, C.RestraintReasonName, C.UsedFlag
		FROM dbo.RestraintReasons P WITH (NOLOCK)
			INNER JOIN dbo.RestraintReasons C WITH (NOLOCK) ON C.RestraintReasonNode.IsDescendantOf(P.RestraintReasonNode) = 1
		WHERE P.RestraintReasonID = @intRestraintReasonID AND C.RestraintReasonLevel > 0
		ORDER BY C.RestraintReasonNode;

	ELSE IF @inyDirection = 2
		WITH T AS (
			SELECT RestraintReasonNode, RestraintReasonID, RestraintReasonLevel, RestraintReasonName, UsedFlag
			FROM dbo.RestraintReasons WITH (NOLOCK)
			WHERE RestraintReasonID = @intRestraintReasonID AND RestraintReasonLevel > 1
			UNION ALL
			SELECT R.RestraintReasonNode, R.RestraintReasonID, R.RestraintReasonLevel, R.RestraintReasonName, R.UsedFlag
			FROM dbo.RestraintReasons R WITH (NOLOCK)
				INNER JOIN T ON T.RestraintReasonNode.GetAncestor(1) =  R.RestraintReasonNode
			WHERE R.RestraintReasonLevel >= 1
		)
		SELECT RestraintReasonID, RestraintReasonLevel, RestraintReasonName, UsedFlag
		FROM T
		ORDER BY RestraintReasonLevel;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 102102;
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
