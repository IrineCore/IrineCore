USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetTeacherPupilInfo]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-27
description : ?? ?? ??? ?????. (?? ?, ?? ?, ? ?? ?? ?? ?? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = @sdtNewTeacherBlockDate ?? NULL?? 2000-01-01? ????? ?? at 2010-07-27
3 = ?? ??? ?? ??? ???? ID? ?? ????? ?? at 2010-07-29
4 = ??? @vchOppositeCharacterIDs output param. ? default ?? at 2010-07-29
5 = ??? ? (@intGraduateCount)? ????? ?? at 2010-08-23
**/

CREATE PROCEDURE [dbo].[P_GetTeacherPupilInfo]
	  @inbCharacterID bigint --// ??? ID
	, @intTeacherCount int OUTPUT --// ?? ?
	, @intPupilCount int OUTPUT --// ?? ?
	, @sdtNewTeacherBlockDate smalldatetime OUTPUT --// ? ?? ??? ??? ?? ??? ???? ??
	, @vchOppositeCharacterIDs varchar(500) = NULL OUTPUT --// ,? ?????? ?? ??? ?? ??? ??? ID ???
	, @intGraduateCount int = NULL OUTPUT --// ??? ?
	, @nvcOppositeCharacterNames nvarchar(500) = NULL OUTPUT --// ,? ?????? ?? ??? ?? ??? ??? ?? ???
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
	SET @intTeacherCount = 0;
	SET @intPupilCount = 0;
	SET @intGraduateCount = 0;
	SET @sdtNewTeacherBlockDate = '2000-01-01';

	SELECT @intTeacherCount = TeacherCount FROM dbo.VIX_TeacherCount WITH (NOEXPAND, NOLOCK) WHERE PupilCharacterID = @inbCharacterID;
	SELECT @intPupilCount = PupilCount FROM dbo.VIX_PupilCount WITH (NOEXPAND, NOLOCK) WHERE TeacherCharacterID = @inbCharacterID;
	SELECT @intGraduateCount = GraduateCount FROM dbo.TeacherCharacters WITH (NOLOCK) WHERE TeacherCharacterID = @inbCharacterID;
	SELECT @sdtNewTeacherBlockDate = NewTeacherBlockDate FROM dbo.PupilCharacters WITH (NOLOCK) WHERE PupilCharacterID = @inbCharacterID;

	SET @vchOppositeCharacterIDs = ISNULL(STUFF((
		SELECT ',' + CAST(CharacterID AS varchar(20))
		FROM (
			SELECT PupilCharacterID AS CharacterID
			FROM dbo.TeachersAndPupils WITH (NOLOCK, FORCESEEK)
			WHERE TeacherCharacterID = @inbCharacterID AND GraduateFlag = CAST(0 AS bit)
			UNION ALL
			SELECT TeacherCharacterID
			FROM dbo.TeachersAndPupils WITH (NOLOCK, FORCESEEK)
			WHERE PupilCharacterID = @inbCharacterID AND GraduateFlag = CAST(0 AS bit)
		) S
		FOR XML PATH ('')), 1, 1, ''), '');

	SET @nvcOppositeCharacterNames = ISNULL(STUFF((
		SELECT N',' + CharacterName
		FROM (
			SELECT B.CharacterName
			FROM dbo.TeachersAndPupils A WITH (NOLOCK, FORCESEEK)
				INNER JOIN dbo.Characters B WITH (NOLOCK, FORCESEEK) ON A.PupilCharacterID = B.CharacterID
			WHERE A.TeacherCharacterID = @inbCharacterID AND A.GraduateFlag = CAST(0 AS bit)
			UNION ALL
			SELECT D.CharacterName
			FROM dbo.TeachersAndPupils C WITH (NOLOCK, FORCESEEK)
				INNER JOIN dbo.Characters D WITH (NOLOCK, FORCESEEK) ON C.TeacherCharacterID = D.CharacterID
			WHERE C.PupilCharacterID = @inbCharacterID AND C.GraduateFlag = CAST(0 AS bit)
		) S
		FOR XML PATH (N'')), 1, 1, N''), N'');
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
