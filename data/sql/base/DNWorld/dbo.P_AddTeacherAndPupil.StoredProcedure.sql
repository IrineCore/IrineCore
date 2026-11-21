USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTeacherAndPupil]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 20
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-26
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ?? ?? ?? ???? ???? ????.
103206 = ??? ? ?? ?? ?? ?? ???????.
103207 = ??? ? ?? ?? ?? ?? ???????.
103208 = ? ??? ??? ? ?? ?????.
103210 = ?? ?? ?????.
103213 = ?? ??? ???? ?? ??????.
103215 = ??? ?? ??? ?? ?? ?? ?? ??? ?????.
history :
9 = ?? ?? ?? ???? ?? ?? 103102? ????? ?? at 2010-07-29
12 = ??? ?? ??? ??? ?? ?? ?? ??? ?? at 2010-08-03
13 = input param. @inyTransactorCode / output param. @intOppositeAccountID ?? at 2010-08-04
15 = ?? ?? ??? ???? ?? ??? ??? ? ?? input param. @bitSkipCheckFlag ?? at 2010-09-02
16 = @intOppositeAccountID ?? ?? ????? ?? at 2010-10-18
17 = ??? ?? ?? at 2011-03-21
18 = ??? ??? ?? ?? at 2011-03-29
19 = ?? ???? ???? ?? at 2011-06-16
20 = DormantCharacterStatus ??? ?? ?? by ??? at 2011-11-30
**/

CREATE PROCEDURE [dbo].[P_AddTeacherAndPupil]
	  @inbTeacherCharacterID bigint --// ?? ??? ID
	, @inbPupilCharacterID bigint --// ?? ??? ID
	, @intMaxTeacherCount int --// 1?? ??? ??? ? ?? ?? ?? ?
	, @intMaxPupilCount int --// 1?? ??? ??? ? ?? ?? ?? ?
	, @bitCheckTeacherListShowFlag bit --// 0=TeacherListShowFlag? ???? ??, 1=TeacherListShowFlag? ???
	, @inyCharacterLevelGap tinyint = 10 --// ??? ?? ??? ?? ?? ?? ?? ?? ?
	, @inyTransactorCode tinyint --// 1=??? ???, 2=??? ???
	, @intOppositeAccountID int OUTPUT --// ???? ??? ???? ??? ?? ID
	, @bitSkipCheckFlag bit = 0 --// ?? ??? ??? ? ?? ??? ?? ??? ???? ??. 0=???? ??, 1=???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intTeacherCount int = 0
	, @intPupilCount int = 0
	, @inyTeacherCharacterJobCode tinyint
	, @inyTeacherCharacterLevel tinyint
	, @inyPupilCharacterJobCode tinyint
	, @inyPupilCharacterLevel tinyint
	, @inyTeacherAndPupilLogCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inyTeacherCharacterJobCode = CHS.JobCode, @inyTeacherCharacterLevel = CHS.CharacterLevel
		, @intOppositeAccountID = 
			CASE @inyTransactorCode 
				WHEN 2 THEN CH.AccountID 
				ELSE @intOppositeAccountID 
			END
	FROM dbo.Characters CH WITH (NOLOCK)
		INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
	WHERE CH.CharacterID = @inbTeacherCharacterID;

	SELECT @inyPupilCharacterJobCode = CHS.JobCode, @inyPupilCharacterLevel = CHS.CharacterLevel
		, @intOppositeAccountID = 
			CASE @inyTransactorCode 
				WHEN 1 THEN CH.AccountID 
				ELSE @intOppositeAccountID 
			END
	FROM dbo.Characters CH WITH (NOLOCK)
		INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
	WHERE CH.CharacterID = @inbPupilCharacterID;

	SET @inyTeacherAndPupilLogCode = CASE @inyTransactorCode WHEN 1 THEN 1 WHEN 2 THEN 5 END;

	/**_# ?? ??? ??? ? ?? ???? 103208? ?????.*/
	IF @bitSkipCheckFlag = 0 AND EXISTS (SELECT * FROM dbo.PupilCharacters WITH (NOLOCK) WHERE PupilCharacterID = @inbPupilCharacterID AND NewTeacherBlockDate >= @dt2Now)
	BEGIN
		SET @intReturnValue = 103208;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ???? ????? ???? ?? ????, ?? ??? ???? ?? ????? 103213? ?????.*/
	IF @bitCheckTeacherListShowFlag = 1 AND (SELECT TeacherListShowFlag FROM dbo.TeacherCharacters WITH (NOLOCK) WHERE TeacherCharacterID = @inbTeacherCharacterID) = CAST(0 AS bit)
	BEGIN
		SET @intReturnValue = 103213;
		GOTO ErrorHandler;
	END

	/**_# ??? ?? ??? ?? ?? ?? ?? ??? ???? ??? 103215? ?????.*/
	IF (@inyTeacherCharacterLevel - @inyPupilCharacterLevel) < @inyCharacterLevelGap
	BEGIN
		SET @intReturnValue = 103215;
		GOTO ErrorHandler;
	END

	/**_# [TeacherCharacters] ???? ???? ?? ?? ????? ???? INSERT ???.*/
	IF NOT EXISTS (
		SELECT * FROM dbo.TeacherCharacters WITH (NOLOCK)
		WHERE TeacherCharacterID = @inbTeacherCharacterID
	)
	BEGIN
		INSERT dbo.TeacherCharacters (TeacherCharacterID, JobCode, GenderCode, TeacherListShowFlag, RespectPoint, SelfIntroduction, GraduateCount, RegisterDate)
		SELECT CH.CharacterID, CHS.JobCode, PR.GenderCode, 0, 0, N'', 0, @dt2Now
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
			LEFT OUTER JOIN dbo.Profiles PR WITH (NOLOCK) ON CH.CharacterID = PR.CharacterID
		WHERE CH.CharacterID = @inbTeacherCharacterID AND CH.DeleteFlag = CAST(0 AS bit);
	END

	BEGIN TRANSACTION;

	/**_# [TeachersAndPupils] ???? INSERT???.*/
	INSERT dbo.TeachersAndPupils (TeacherCharacterID, PupilCharacterID, FavorPoint, GraduateFlag, RegisterDate)
	VALUES (@inbTeacherCharacterID, @inbPupilCharacterID, 0, 0, @dt2Now);

	SELECT @intTeacherCount = TeacherCount FROM dbo.VIX_TeacherCount WITH (NOEXPAND) WHERE PupilCharacterID = @inbPupilCharacterID;
	SELECT @intPupilCount = PupilCount FROM dbo.VIX_PupilCount WITH (NOEXPAND) WHERE TeacherCharacterID = @inbTeacherCharacterID;

	/**_# ??? ? ?? ?? ?? ?? ???? 103206? ?????.*/
	IF @intTeacherCount > @intMaxTeacherCount
	BEGIN
		SET @intReturnValue = 103206;
		GOTO ErrorHandler;
	END

	/**_# ??? ? ?? ?? ?? ?? ???? 103207? ?????.*/
	ELSE IF @intPupilCount > @intMaxPupilCount
	BEGIN
		SET @intReturnValue = 103207;
		GOTO ErrorHandler;
	END

	/**_# ??? ? ?? ?? ?? ?? ???? ?? ?? ???? ??? ?????.*/
	IF @intPupilCount = @intMaxPupilCount
	BEGIN
		EXEC @intReturnValue = dbo.P_DelTeacherCharacter @inbTeacherCharacterID = @inbTeacherCharacterID;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	/**_# ??? ?????.*/
	EXEC dbo.P_AddTeacherAndPupilLog
		  @dt2LogDate = @dt2Now
		, @inbTeacherCharacterID = @inbTeacherCharacterID
		, @inbPupilCharacterID = @inbPupilCharacterID
		, @inyTeacherCharacterJobCode = @inyTeacherCharacterJobCode
		, @inyTeacherCharacterLevel = @inyTeacherCharacterLevel
		, @inyPupilCharacterJobCode = @inyPupilCharacterJobCode
		, @inyPupilCharacterLevel = @inyPupilCharacterLevel
		, @intFavorPoint = 0
		, @insNewTeacherBlockPeriod = 0
		, @intPenaltyRespectPoint = 0
		, @inyTeacherAndPupilLogCode = @inyTeacherAndPupilLogCode;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_TeacherCharacters_TeachersAndPupils_TeacherCharacterID%'
		SET @intReturnValue = 103102;

	ELSE IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_TeachersAndPupils_PupilCharacterID%'
		SET @intReturnValue = 103102;

	ELSE IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_TeachersAndPupils%'
		SET @intReturnValue = 103210;

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
