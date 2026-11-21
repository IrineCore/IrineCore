USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ResetSkillBySkillIDsFromGSM]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-15
description : ??? ???? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ??? ?? ?? at 2011-03-21
3 = ??? ??? ?? ?? at 2011-03-29
4 = input param. @inySkillCode ?? at 2011-04-06
5 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_ResetSkillBySkillIDsFromGSM]
	  @inbCharacterID bigint --// ??? ID
	, @vchSkillIDs varchar(8000) --// ,? ?????? "?? ?? ?? ID" ???
	, @inySkillCode tinyint = 1 --// 1=1? ??, 2=2? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @insSkillPoint smallint = 0
	, @insTotalSkillPoint smallint
	, @i int
	, @j int
	, @intOldSkillID int
	, @inyNewSkillLevel tinyint
	, @inyOldSkillLevel tinyint
	, @dt2Now datetime2(0) = GETDATE();

DECLARE @tblTargetSkills table (
	SkillID int NOT NULL PRIMARY KEY
);
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
	INSERT @tblTargetSkills (SkillID)
	SELECT CAST(string AS int)
	FROM dbo.TF_Split(@vchSkillIDs, ',')
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);

	BEGIN TRANSACTION;

	/**_# ?? ??? ??????.*/
	UPDATE dbo.Skills
	SET @insSkillPoint += UsedSkillPoint
		, SkillLevel = DefaultSkillLevel
		, UsedSkillPoint = 0
	OUTPUT deleted.SkillID, deleted.SkillLevel, inserted.SkillLevel INTO @tblOldSkills (OldSkillID, OldSkillLevel, NewSkillLevel)
	FROM @tblTargetSkills T
		INNER JOIN dbo.Skills SK WITH (INDEX(PK_Skills), FORCESEEK) ON SK.SkillID = T.SkillID
	WHERE SK.CharacterID = @inbCharacterID AND SK.SkillCode = @inySkillCode
	OPTION (KEEPFIXED PLAN);

	SELECT @i = 1, @j = @@ROWCOUNT;

	IF @insSkillPoint = 0
	BEGIN
		ROLLBACK TRANSACTION;
		RETURN 0;
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
			, @inySkillChangeCode = 15
			, @intSkillID = @intOldSkillID
			, @inySkillLevelBefore = @inyOldSkillLevel
			, @inySkillLevelAfter = @inyNewSkillLevel
			, @intChannelID = NULL
			, @intMapID = NULL
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
