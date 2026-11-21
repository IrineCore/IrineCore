USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetMyClassmate]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-27
description : ? ??? ?? ??? ?????.
result set : CharacterID/bigint, CharacterName/nvarchar, JobCode/tinyint, CharacterLevel/tinyint, TeacherCharacterNames/nvarchar
return value : 
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ???? ?? ?? ?? at 2010-07-28
**/

CREATE PROCEDURE [dbo].[P_GetMyClassmate]
	  @inbCharacterID bigint --// ??? ID
	, @inbClassmateCharacterID bigint --// ?? ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcTeacherCharacterNames nvarchar(4000);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @nvcTeacherCharacterNames = ISNULL(STUFF((
		SELECT N',' + CH.CharacterName
		FROM dbo.TeachersAndPupils TnP1 WITH (NOLOCK)
			INNER JOIN dbo.TeachersAndPupils TnP2 WITH (NOLOCK) ON TnP1.TeacherCharacterID = TnP2.TeacherCharacterID
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON TnP2.TeacherCharacterID = CH.CharacterID
		WHERE TnP1.PupilCharacterID = @inbCharacterID AND TnP2.PupilCharacterID = @inbClassmateCharacterID AND TnP2.GraduateFlag = CAST(0 AS bit)
		FOR XML PATH (N'')), 1, 1, N''), N'');

	SELECT CharacterID, CharacterName, JobCode, CharacterLevel, @nvcTeacherCharacterNames AS TeacherCharacterNames
	FROM dbo.V_Characters WITH (NOLOCK)
	WHERE CharacterID = @inbClassmateCharacterID;
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
