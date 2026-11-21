USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetTeacherCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-26
description : ?? ?? ??? ?????. (??, ? ??, ?? ??)
result set : 
	@inyGetType = 1 : TeacherListShowFlag/bit, SelfIntroduction/nvarchar, GraduateCount/int, RespectPoint/int
	@inyGetType = 2 : CharacterID/bigint, CharacterName/nvarchar, JobCode/tinyint, CharacterLevel/tinyint, RespectPoint/int, FavorPoint/int
	@inyGetType = 3 : CharacterID/bigint, CharacterName/nvarchar, SelfIntroduction/nvarchar, GenderCode/tinyint, CharacterLevel/tinyint, JobCode/tinyint, GraduateCount/int, RespectPoint/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetTeacherCharacter]
	  @inyGetType tinyint --// 1=??? ?? ??, 2=? ?? ??, 3=??? ?? ?? ??
	, @inbTeacherCharacterID bigint --// ?? ??? ID
	, @inbPupilCharacterID bigint = NULL --// ?? ??? ID. ? ?? ?? ??? ??? ? ?????. ? ?? ?? ???? ????.
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
	IF @inyGetType = 1
		SELECT TS.TeacherListShowFlag, TS.SelfIntroduction, TS.GraduateCount, TS.RespectPoint
		FROM dbo.TeacherCharacters TS WITH (NOLOCK)
		WHERE TS.TeacherCharacterID = @inbTeacherCharacterID;

	ELSE IF @inyGetType = 2
		SELECT CS.CharacterID, CH.CharacterName, CS.JobCode, CS.CharacterLevel, TS.RespectPoint, TnP.FavorPoint
		FROM dbo.TeacherCharacters TS WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON TS.TeacherCharacterID = CH.CharacterID
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON TS.TeacherCharacterID = CS.CharacterID
			INNER JOIN dbo.TeachersAndPupils TnP WITH (NOLOCK) ON TS.TeacherCharacterID = TnP.TeacherCharacterID
		WHERE TS.TeacherCharacterID = @inbTeacherCharacterID AND TnP.PupilCharacterID = @inbPupilCharacterID;

	ELSE IF @inyGetType = 3
		SELECT CS.CharacterID, CH.CharacterName, TS.SelfIntroduction, TS.GenderCode, CS.CharacterLevel, CS.JobCode, TS.GraduateCount, TS.RespectPoint
		FROM dbo.TeacherCharacters TS WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON TS.TeacherCharacterID = CH.CharacterID
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON TS.TeacherCharacterID = CS.CharacterID
		WHERE TS.TeacherCharacterID = @inbTeacherCharacterID;
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
