USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_CancelRestraint]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_CancelRestraint]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_CancelRestraint] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-08
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101123 = ??? ??? ???? ????.
101124 = ?? ??? ?? ?????.
**/

ALTER PROCEDURE [dbo].[P_CancelRestraint]
	  @intRestraintID int --// ?? ID
	, @intAdminID int --// ??? ID
	, @nvcMemo nvarchar(300) --// ?? ?? ??
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
	BEGIN TRANSACTION;

	/**_# [Restraints] ???? UPDATE???.*/
	UPDATE dbo.Restraints
	SET CancelFlag = 1
	WHERE RestraintID = @intRestraintID;

	/**_# [CanceledRestraints] ???? INSERT???.*/
	INSERT dbo.CanceledRestraints (RestraintID, AdminID, Memo, CancelDate)
	VALUES (@intRestraintID, @intAdminID, @nvcMemo, GETDATE());

	COMMIT TRANSACTION;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Restraints_CanceledRestraints_RestraintID%'
		SET @intReturnValue = 101123;

	ELSE IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_CanceledRestraints%'
		SET @intReturnValue = 101124;

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
