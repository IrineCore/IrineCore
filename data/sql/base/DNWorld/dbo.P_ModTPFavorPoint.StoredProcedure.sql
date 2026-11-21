USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModTPFavorPoint]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-27
description : ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = @intFavorPoint ?? ???? ????? ?? at 2010-08-13
**/

CREATE PROCEDURE [dbo].[P_ModTPFavorPoint]
	  @inbTeacherCharacterID bigint --// ?? ??? ID
	, @inbPupilCharacterID bigint --// ?? ??? ID
	, @intFavorPoint int --// ??? ???
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
	UPDATE dbo.TeachersAndPupils
	SET FavorPoint += @intFavorPoint
	WHERE TeacherCharacterID = @inbTeacherCharacterID AND PupilCharacterID = @inbPupilCharacterID;
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
