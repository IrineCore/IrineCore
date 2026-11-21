USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ResetSkill]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 14
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-12
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103105 = ?? ?? ?????. ??? ??? ????.
history :
11 = ??? ?? ?? at 2011-03-21
12 = ??? ??? ?? ?? at 2011-03-29
13 = input param. @inySkillCode ?? at 2011-04-06
**/

CREATE PROCEDURE [dbo].[P_ResetSkill]
	  @inbCharacterID bigint --// ??? ID
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @insSkillPoint smallint OUTPUT --// ?? ?? ? ?? ??? ???
	, @insTotalSkillPoint smallint OUTPUT --// ?? ?? ? ??? ? ?? ???
	, @inySkillCode tinyint = 1 --// 1=1? ??, 2=2? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @bitSkillResetFlag bit
	, @i int
	, @j int
	, @intOldSkillID int
	, @inyNewSkillLevel tinyint
	, @inyOldSkillLevel tinyint
	, @dt2Now datetime2(0) = GETDATE()

DECLARE @tblOldSkills table (
	seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	OldSkillID int NOT NULL,
	OldSkillLevel tinyint NOT NULL,
	NewSkillLevel tinyint NOT NULL	
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @insSkillPoint = 0;

	BEGIN TRANSACTION;

	/**_# ?? ??? ??????.*/
	UPDATE dbo.Skills
	SET @insSkillPoint += UsedSkillPoint
		, SkillLevel = DefaultSkillLevel
		, UsedSkillPoint = 0
	OUTPUT deleted.SkillID, deleted.SkillLevel, inserted.SkillLevel INTO @tblOldSkills (OldSkillID, OldSkillLevel, NewSkillLevel)
	WHERE CharacterID = @inbCharacterID AND SkillCode = @inySkillCode;

	SELECT @i = 1, @j = @@ROWCOUNT;

	/**_## ?? ???? ??? ???? ???? 103105? ?????.*/
	IF @insSkillPoint = 0
	BEGIN
		SET @intReturnValue = 103105;
		GOTO ErrorHandler;
	END

	/**_# ???? SkillPoint? ?????.*/
	IF @inySkillCode = 1
		UPDATE dbo.CharacterStatus
		SET @insTotalSkillPoint = SkillPoint += @insSkillPoint
		WHERE CharacterID = @inbCharacterID;

	ELSE IF @inySkillCode = 2
		UPDATE dbo.CharacterStatus
		SET @insTotalSkillPoint = SkillPoint2 += @insSkillPoint
		WHERE CharacterID = @inbCharacterID;

	COMMIT TRANSACTION;

	/**_# [P_AddSkillLog]? ???? ?? ??? ?????.*/
	WHILE @i <= @j
	BEGIN
		SELECT @intOldSkillID = OldSkillID, @inyOldSkillLevel = OldSkillLevel, @inyNewSkillLevel = NewSkillLevel FROM @tblOldSkills WHERE seq = @i;

		EXEC @intReturnValue = dbo.P_AddSkillLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inySkillChangeCode = 3
			, @intSkillID = @intOldSkillID
			, @inySkillLevelBefore = @inyOldSkillLevel
			, @inySkillLevelAfter = @inyNewSkillLevel
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @inySkillCode = @inySkillCode;

		SET @i += 1;
	END

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
