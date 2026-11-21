USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMyTeacher]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-10
description : ? ?? ??? ?????.
result set : TeacherCharacterID/bigint, FavorPoint/int, TeacherCharacterName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ??? ??? ??? ?? ?? at 2010-08-16
**/

CREATE PROCEDURE [dbo].[P_GetListMyTeacher]
	  @inbPupilCharacterID bigint --// ?? ??? ID
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
	SELECT TnP.TeacherCharacterID, TnP.FavorPoint, CH.CharacterName AS TeacherCharacterName
	FROM dbo.TeachersAndPupils TnP WITH (NOLOCK)
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON TnP.TeacherCharacterID = CH.CharacterID
	WHERE TnP.PupilCharacterID = @inbPupilCharacterID AND TnP.GraduateFlag = CAST(0 AS bit);
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
