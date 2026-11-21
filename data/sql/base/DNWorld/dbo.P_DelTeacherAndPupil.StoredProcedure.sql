USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelTeacherAndPupil]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-27
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103218 = ?? ??? ??? 24?? ???? ??? ??? ? ????.
103228 = ?? ??? ????.
history :
3 = ?? ??? ??? 24?? ???? ??? ?? ? ??? ?? at 2010-08-18
4 = ?? ??? ??? ?? ??? ??? ? ?? input param. @bitSkipCheckFlag ?? at 2010-09-02
5 = ?? ?? ??? ?? ??? ?? ?? ?? ?? at 2010-09-03
6 = ??? ?? ?? at 2011-03-21
7 = ??? ??? ?? ?? at 2011-03-29
8 = ?? ???? ???? ?? at 2011-06-16
9 = DormantCharacterStatus ??? ?? ?? by ??? at 2011-11-30
**/

CREATE PROCEDURE [dbo].[P_DelTeacherAndPupil]
	  @inbTeacherCharacterID bigint --// ?? ??? ID
	, @inbPupilCharacterID bigint --// ?? ??? ID
	, @inyTransactorCode tinyint --// 1=??? ???, 2=??? ???
	, @insNewTeacherBlockPeriod smallint --// ??? ??? ?? ? ???? ?? ??. ? ??? ??? ? ?? ?? (?? : day)
	, @intPenaltyRespectPoint int --// ??? ??? ??? ? ???? ?? ??. ???? ?? ?? (?? ??? ?????.)
	, @bitSkipCheckFlag bit = 0 --// ??? ?? ??? ???? ??. 0=???? ??, 1=???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inyTeacherCharacterJobCode tinyint
	, @inyTeacherCharacterLevel tinyint
	, @inyPupilCharacterJobCode tinyint
	, @inyPupilCharacterLevel tinyint
	, @intFavorPoint int
	, @inyTeacherAndPupilLogCode tinyint
	, @dt2RegisterDate datetime2(0);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @inyTeacherAndPupilLogCode = CASE @inyTransactorCode WHEN 1 THEN 2 WHEN 2 THEN 3 END;
	
	BEGIN TRANSACTION;

	SELECT @intFavorPoint = FavorPoint, @dt2RegisterDate = RegisterDate
	FROM dbo.TeachersAndPupils WITH (UPDLOCK)
	WHERE TeacherCharacterID = @inbTeacherCharacterID AND PupilCharacterID = @inbPupilCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103228;
		GOTO ErrorHandler;
	END

	IF @bitSkipCheckFlag = 0 AND DATEDIFF(SECOND, @dt2RegisterDate, GETDATE()) < 86400
	BEGIN
		SET @intReturnValue = 103218;
		GOTO ErrorHandler;
	END

	DELETE dbo.TeachersAndPupils
	WHERE TeacherCharacterID = @inbTeacherCharacterID AND PupilCharacterID = @inbPupilCharacterID;

	COMMIT TRANSACTION;

	IF @intPenaltyRespectPoint > 0
		UPDATE dbo.TeacherCharacters
		SET RespectPoint -= @intPenaltyRespectPoint
		WHERE TeacherCharacterID = @inbTeacherCharacterID;

	ELSE IF @insNewTeacherBlockPeriod > 0
	BEGIN
		UPDATE dbo.PupilCharacters
		SET NewTeacherBlockDate = DATEADD(DAY, @insNewTeacherBlockPeriod, @dt2Now)
		WHERE PupilCharacterID = @inbPupilCharacterID;

		IF @@ROWCOUNT = 0
			INSERT dbo.PupilCharacters (PupilCharacterID, NewTeacherBlockDate)
			VALUES (@inbPupilCharacterID, DATEADD(DAY, @insNewTeacherBlockPeriod, @dt2Now));
	END

	SELECT @inyTeacherCharacterJobCode = JobCode, @inyTeacherCharacterLevel = CharacterLevel
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbTeacherCharacterID;

	SELECT @inyPupilCharacterJobCode = JobCode, @inyPupilCharacterLevel = CharacterLevel
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbPupilCharacterID;

	EXEC dbo.P_AddTeacherAndPupilLog
		  @dt2LogDate = @dt2Now
		, @inbTeacherCharacterID = @inbTeacherCharacterID
		, @inbPupilCharacterID = @inbPupilCharacterID
		, @inyTeacherCharacterJobCode = @inyTeacherCharacterJobCode
		, @inyTeacherCharacterLevel = @inyTeacherCharacterLevel
		, @inyPupilCharacterJobCode = @inyPupilCharacterJobCode
		, @inyPupilCharacterLevel = @inyPupilCharacterLevel
		, @intFavorPoint = @intFavorPoint
		, @insNewTeacherBlockPeriod = @insNewTeacherBlockPeriod
		, @intPenaltyRespectPoint = @intPenaltyRespectPoint
		, @inyTeacherAndPupilLogCode = @inyTeacherAndPupilLogCode;
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
