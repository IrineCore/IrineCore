USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTeacherCharacter]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-26
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = Profiles.GenderCode? ????? ?? at 2010-07-29
**/

CREATE PROCEDURE [dbo].[P_AddTeacherCharacter]
	  @inbTeacherCharacterID bigint --// ??? ID
	, @nvcSelfIntroduction nvarchar(50) --// ?? ??
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
	/**_# [TeacherCharacters] ???? UPDATE???.*/
	UPDATE dbo.TeacherCharacters
	SET TeacherListShowFlag = 1, SelfIntroduction = @nvcSelfIntroduction
	WHERE TeacherCharacterID = @inbTeacherCharacterID;

	/**_# ???? ???? ????, [TeacherCharacters] ???? INSERT???.*/
	IF @@ROWCOUNT = 0
		INSERT dbo.TeacherCharacters (TeacherCharacterID, JobCode, GenderCode, TeacherListShowFlag, RespectPoint, SelfIntroduction, GraduateCount, RegisterDate)
		SELECT @inbTeacherCharacterID, CS.JobCode, PR.GenderCode, 1, 0, @nvcSelfIntroduction, 0, GETDATE()
		FROM dbo.CharacterStatus CS WITH (NOLOCK)
			LEFT OUTER JOIN dbo.Profiles PR WITH (NOLOCK) ON CS.CharacterID = PR.CharacterID
		WHERE CS.CharacterID = @inbTeacherCharacterID;
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
