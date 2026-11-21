USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddRestraintReason]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-05
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102101 = ?? ?? ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_AddRestraintReason]
	  @intParentRestraintReasonID int = NULL --// ?? ?? ?? ID / ???? ?? NULL? ?????.
	, @nvcRestraintReasonName nvarchar(50) --// ?? ?? ??
	, @intAdminID int --// ?? ??? ??? ??? ID
	, @intRestraintReasonID int = NULL OUTPUT --// ?? ?? ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @hidParentRestraintReasonNode hierarchyid
	, @hidMaxRestraintReasonNode hierarchyid;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ???? ?? ??? Root ???? INSERT???.*/
	IF NOT EXISTS (SELECT * FROM dbo.RestraintReasons WITH (NOLOCK))
		INSERT dbo.RestraintReasons (RestraintReasonNode, RestraintReasonName, AdminID, RegisterDate)
		VALUES (hierarchyid::GetRoot(), N'Root', @intAdminID, GETDATE());

	/**_# @intParentRestraintReasonID? ?????. ???? ?? @intParentRestraintReasonID? ?? 102101? ?????.*/
	IF @intParentRestraintReasonID IS NULL
		SET @hidParentRestraintReasonNode = hierarchyid::GetRoot();

	ELSE
	BEGIN
		SELECT @hidParentRestraintReasonNode = RestraintReasonNode
		FROM dbo.RestraintReasons WITH (NOLOCK)
		WHERE RestraintReasonID = @intParentRestraintReasonID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 102101;
			GOTO ErrorHandler;
		END
	END

	/**_# [RestraintReasons] ???? INSERT ???.*/
	BEGIN TRANSACTION;

	SELECT @hidMaxRestraintReasonNode = MAX(RestraintReasonNode)
	FROM dbo.RestraintReasons WITH (SERIALIZABLE)
	WHERE RestraintReasonNode.GetAncestor(1) = @hidParentRestraintReasonNode;

	INSERT dbo.RestraintReasons (RestraintReasonNode, RestraintReasonName, AdminID, RegisterDate)
	VALUES (@hidParentRestraintReasonNode.GetDescendant(@hidMaxRestraintReasonNode, NULL), @nvcRestraintReasonName, @intAdminID, GETDATE());

	SET @intRestraintReasonID = SCOPE_IDENTITY();

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
