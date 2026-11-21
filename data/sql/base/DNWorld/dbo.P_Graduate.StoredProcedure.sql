USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_Graduate]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-27
description : ?? ??? ??? ?????.
result set : TeacherCharacterID/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103209 = ??? ???? ?? ??? ??????.
history :
2 = ?? ??? ID? ????? ?? at 2010-07-29
**/

CREATE PROCEDURE [dbo].[P_Graduate]
	  @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inyPupilCharacterJobCode tinyint
	, @inyPupilCharacterLevel tinyint;
DECLARE @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dt2Now);
DECLARE @tblTeachers table (TeacherCharacterID bigint NOT NULL, FavorPoint int NOT NULL);
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# dbo.TeachersAndPupils.GraduateFlag ??? 1? ???????.*/
	UPDATE dbo.TeachersAndPupils
	SET GraduateFlag = 1
	OUTPUT inserted.TeacherCharacterID, inserted.FavorPoint
	INTO @tblTeachers (TeacherCharacterID, FavorPoint)
	WHERE PupilCharacterID = @inbCharacterID AND GraduateFlag = CAST(0 AS bit);

	/**_## ??? ???? ?? ??? ????? 103209? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103209;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ?? +1 ???.*/
	UPDATE dbo.TeacherCharacters
	SET GraduateCount += 1
	FROM dbo.TeacherCharacters TC
		INNER JOIN @tblTeachers TnP ON TC.TeacherCharacterID = TnP.TeacherCharacterID;

	COMMIT TRANSACTION;

	/**_# PupilCharacters ????? DELETE ???.*/
	DELETE dbo.PupilCharacters WHERE PupilCharacterID = @inbCharacterID;

	/**_# ??? ?????.*/
	SELECT @inyPupilCharacterJobCode = JobCode, @inyPupilCharacterLevel = CharacterLevel
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	IF @intLogTableSuffix = 0
		INSERT dbo.TeacherAndPupilLogs0 (LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode)
		SELECT @dt2Now, CS.CharacterID, @inbCharacterID, CS.JobCode, CS.CharacterLevel, @inyPupilCharacterJobCode, @inyPupilCharacterLevel, T.FavorPoint, 0, 0, 4
		FROM @tblTeachers T
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON T.TeacherCharacterID = CS.CharacterID;

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.TeacherAndPupilLogs1 (LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode)
		SELECT @dt2Now, CS.CharacterID, @inbCharacterID, CS.JobCode, CS.CharacterLevel, @inyPupilCharacterJobCode, @inyPupilCharacterLevel, T.FavorPoint, 0, 0, 4
		FROM @tblTeachers T
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON T.TeacherCharacterID = CS.CharacterID;

	ELSE IF @intLogTableSuffix = 2
		INSERT dbo.TeacherAndPupilLogs2 (LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode)
		SELECT @dt2Now, CS.CharacterID, @inbCharacterID, CS.JobCode, CS.CharacterLevel, @inyPupilCharacterJobCode, @inyPupilCharacterLevel, T.FavorPoint, 0, 0, 4
		FROM @tblTeachers T
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON T.TeacherCharacterID = CS.CharacterID;

	/**_# ?? ??? ID? ?????.*/
	SELECT TeacherCharacterID
	FROM @tblTeachers;
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
