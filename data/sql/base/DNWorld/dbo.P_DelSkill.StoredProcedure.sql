USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelSkill]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-19
description : ??? ?????. (??? Lock???.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103144 = ???? ?? ?????.
history :
4 = ??? ?? ?? at 2011-03-21
5 = ??? ??? ?? ?? at 2011-03-29
6 = input param. @inySkillCode ?? at 2011-04-06
**/

CREATE PROCEDURE [dbo].[P_DelSkill]
	  @inbCharacterID bigint --// ??? ID
	, @intSkillID int --// ?? ID
	, @inySkillChangeCode tinyint --// 9=?? ?? (??), 10=?? ?? (???), 11=?? ?? (??? ??), 12=?? ?? (by ???)
	, @bitSkillPointBackFlag bit --// ?? ?? ? ?? ???? ?? ?? ??. 0=????? ???? ??, 1=?? ??? ???
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @inySkillCode tinyint = 1 --// 1=1? ??, 2=2? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inySkillLevelBefore tinyint
	, @insUsedSkillPoint smallint;

DECLARE @tblTemp table (SkillLevel tinyint NOT NULL, UsedSkillPoint smallint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Skills]????? DELETE???.*/
	DELETE dbo.Skills
	OUTPUT deleted.SkillLevel, deleted.UsedSkillPoint INTO @tblTemp (SkillLevel, UsedSkillPoint)
	WHERE CharacterID = @inbCharacterID AND SkillID = @intSkillID AND SkillCode = @inySkillCode;

	/**_# ??? ??? ??? 103144? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103144;
		GOTO ErrorHandler;
	END

	SELECT @inySkillLevelBefore = SkillLevel, @insUsedSkillPoint = UsedSkillPoint
	FROM @tblTemp;

	/**_# ?? ???? ????? CharacterStatus.SkillPoint? UPDATE???.*/
	IF @bitSkillPointBackFlag = 1
		UPDATE dbo.CharacterStatus
		SET SkillPoint += CASE @inySkillCode WHEN 1 THEN @insUsedSkillPoint WHEN 2 THEN 0 END 
			, SkillPoint2 += CASE @inySkillCode WHEN 2 THEN @insUsedSkillPoint WHEN 1 THEN 0 END 
		WHERE CharacterID = @inbCharacterID;

	/**_# [P_DelSkillLog]? ???? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_AddSkillLog
		  @dt2LogDate = @dt2Now
		, @inbCharacterID = @inbCharacterID
		, @inySkillChangeCode = @inySkillChangeCode
		, @intSkillID = @intSkillID
		, @inySkillLevelBefore = @inySkillLevelBefore
		, @inySkillLevelAfter = 0
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID
		, @inySkillCode = @inySkillCode;

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
