USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ChangeJobCode]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-04

description :
	??? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103102 = ???? ???? ????.
history :
	5 = ?? ??
	6 = 1, 2? ?? ?? ?????? ?? at 2011-10-07
**/

CREATE PROCEDURE [dbo].[P_ChangeJobCode]
	  @inbCharacterID bigint --// ??? ID

	, @iny1stJobCodeBefore tinyint --// 11=?????, 12=????, 13=???, 14=?????, 15=????, 16=??, 17=??????, 18=????, 19=??, 20=???, 21=??, 22=????, 46=???? ,49=?????,
	, @iny1stJobCodeAfter tinyint --// 11=?????, 12=????, 13=???, 14=?????, 15=????, 16=??, 17=??????, 18=????, 19=??, 20=???, 21=??, 22=????, 46=???? ,49=?????,
	, @iny2ndJobCodeBefore tinyint = null --// 23=??????, 24=???, 25=????, 26=??????, 27=?????, 28=?????, 29=????, 30=????, 31=????, 32=????, 33=???, 34=?????, 35=????, 36=?????, 37=???, 38=????, 39=?????, 40=??????, 41=???, 42=?????, 43=???, 44=????, 45=?????, 47=????, 48=?????, 50=???, 51=???
	, @iny2ndJobCodeAfter tinyint = null --// 23=??????, 24=???, 25=????, 26=??????, 27=?????, 28=?????, 29=????, 30=????, 31=????, 32=????, 33=???, 34=?????, 35=????, 36=?????, 37=???, 38=????, 39=?????, 40=??????, 41=???, 42=?????, 43=???, 44=????, 45=?????, 47=????, 48=?????, 50=???, 51=???
	, @inyJobCodeChangeCode tinyint --// 1=??? ??, 2=?? ?? ??? ??

	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @insSkillPoint smallint OUTPUT --// ?? ?? ? 1? ?? ??? ???
	, @insTotalSkillPoint smallint OUTPUT --// ?? ?? ? ??? ? ?? ???
	, @inySkillCode tinyint = 1 --// 1=1? ??, 2=2? ??, 3=1, 2? ??
	, @insSkillPoint2 smallint = 0 OUTPUT --// ?? ?? ? 2? ?? ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2NowDate datetime2(0) = getdate();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	-- 1? ??? ? ??
	IF @iny2ndJobCodeAfter IS NULL
	BEGIN

		/**_# ??? ??? ???. */
		IF @inySkillCode = 1
		BEGIN
			EXEC @intReturnValue = dbo.P_ResetSkill
				  @inbCharacterID = @inbCharacterID
				, @intChannelID = @intChannelID
				, @intMapID = @intMapID
				, @insSkillPoint = @insSkillPoint OUTPUT
				, @insTotalSkillPoint = @insTotalSkillPoint OUTPUT
				, @inySkillCode = 1;

			IF @intReturnValue IS NOT NULL AND (@intReturnValue <> 0 AND @intReturnValue <> 103105)
			BEGIN
				GOTO ErrorHandler;
			END
		END

		ELSE IF @inySkillCode = 2
		BEGIN
			EXEC @intReturnValue = dbo.P_ResetSkill
				  @inbCharacterID = @inbCharacterID
				, @intChannelID = @intChannelID
				, @intMapID = @intMapID
				, @insSkillPoint = @insSkillPoint2 OUTPUT
				, @insTotalSkillPoint = @insTotalSkillPoint OUTPUT
				, @inySkillCode = 2;

			IF @intReturnValue IS NOT NULL AND (@intReturnValue <> 0 AND @intReturnValue <> 103105)
			BEGIN
				GOTO ErrorHandler;
			END
		END

		ELSE IF @inySkillCode = 3
		BEGIN
			EXEC @intReturnValue = dbo.P_ResetSkill
				  @inbCharacterID = @inbCharacterID
				, @intChannelID = @intChannelID
				, @intMapID = @intMapID
				, @insSkillPoint = @insSkillPoint OUTPUT
				, @insTotalSkillPoint = @insTotalSkillPoint OUTPUT
				, @inySkillCode = 1;

			IF @intReturnValue IS NOT NULL AND (@intReturnValue <> 0 AND @intReturnValue <> 103105)
			BEGIN
				GOTO ErrorHandler;
			END

			EXEC @intReturnValue = dbo.P_ResetSkill
				  @inbCharacterID = @inbCharacterID
				, @intChannelID = @intChannelID
				, @intMapID = @intMapID
				, @insSkillPoint = @insSkillPoint2 OUTPUT
				, @insTotalSkillPoint = @insTotalSkillPoint OUTPUT
				, @inySkillCode = 2;

			IF @intReturnValue IS NOT NULL AND (@intReturnValue <> 0 AND @intReturnValue <> 103105)
			BEGIN
				GOTO ErrorHandler;
			END
		END

		BEGIN TRANSACTION

			/**_# ??? ?? ??? JobCode? ?????. */
			UPDATE dbo.CharacterStatus
			   SET JobCode = @iny1stJobCodeAfter
			 WHERE CharacterID = @inbCharacterID
			   AND JobCode = @iny1stJobCodeBefore;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue =103102;
				GOTO ErrorHandler;
			END

			/**_# ?? ??? JobCode? ?????. */
			UPDATE dbo.JobChangeLogs
			   SET JobCode = @iny1stJobCodeAfter
				 , LogDate = @dt2NowDate
			 WHERE CharacterID = @inbCharacterID
			   AND JobCode = @iny1stJobCodeBefore;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue =103102;
				GOTO ErrorHandler;
			END

		COMMIT TRANSACTION

		/**_# ?? ?? ??? JobCode? ?????. */
		UPDATE dbo.TeacherCharacters
		   SET JobCode = @iny1stJobCodeAfter
		 WHERE TeacherCharacterID = @inbCharacterID
		   AND JobCode = @iny1stJobCodeBefore;

		/**_# ?? ?? ??? ?????. */
		INSERT dbo.JobCodeChangeLogs (CharacterID, JobCodeBefore, JobCodeAfter, JobCodeChangeCode, ChangeDate)
		VALUES (@inbCharacterID, @iny1stJobCodeBefore, @iny1stJobCodeAfter, @inyJobCodeChangeCode, @dt2NowDate);

	END
	-- 2? ???? ? ??
	ELSE
	BEGIN

		/**_# ??? ??? ???. */
		IF @inySkillCode = 1
		BEGIN
			EXEC @intReturnValue = dbo.P_ResetSkill
				  @inbCharacterID = @inbCharacterID
				, @intChannelID = @intChannelID
				, @intMapID = @intMapID
				, @insSkillPoint = @insSkillPoint OUTPUT
				, @insTotalSkillPoint = @insTotalSkillPoint OUTPUT
				, @inySkillCode = 1;

			IF @intReturnValue IS NOT NULL AND (@intReturnValue <> 0 AND @intReturnValue <> 103105)
			BEGIN
				GOTO ErrorHandler;
			END
		END

		ELSE IF @inySkillCode = 2
		BEGIN
			EXEC @intReturnValue = dbo.P_ResetSkill
				  @inbCharacterID = @inbCharacterID
				, @intChannelID = @intChannelID
				, @intMapID = @intMapID
				, @insSkillPoint = @insSkillPoint2 OUTPUT
				, @insTotalSkillPoint = @insTotalSkillPoint OUTPUT
				, @inySkillCode = 2;

			IF @intReturnValue IS NOT NULL AND (@intReturnValue <> 0 AND @intReturnValue <> 103105)
			BEGIN
				GOTO ErrorHandler;
			END
		END

		ELSE IF @inySkillCode = 3
		BEGIN
			EXEC @intReturnValue = dbo.P_ResetSkill
				  @inbCharacterID = @inbCharacterID
				, @intChannelID = @intChannelID
				, @intMapID = @intMapID
				, @insSkillPoint = @insSkillPoint OUTPUT
				, @insTotalSkillPoint = @insTotalSkillPoint OUTPUT
				, @inySkillCode = 1;

			IF @intReturnValue IS NOT NULL AND (@intReturnValue <> 0 AND @intReturnValue <> 103105)
			BEGIN
				GOTO ErrorHandler;
			END

			EXEC @intReturnValue = dbo.P_ResetSkill
				  @inbCharacterID = @inbCharacterID
				, @intChannelID = @intChannelID
				, @intMapID = @intMapID
				, @insSkillPoint = @insSkillPoint2 OUTPUT
				, @insTotalSkillPoint = @insTotalSkillPoint OUTPUT
				, @inySkillCode = 2;
			IF @intReturnValue IS NOT NULL AND (@intReturnValue <> 0 AND @intReturnValue <> 103105)
			BEGIN
				GOTO ErrorHandler;
			END
		END

		BEGIN TRANSACTION
			/**_# ??? ?? ??? JobCode? ?????. */
			UPDATE dbo.CharacterStatus
			   SET JobCode = @iny2ndJobCodeAfter
			 WHERE CharacterID = @inbCharacterID
			   AND JobCode = @iny2ndJobCodeBefore;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue =103102;
				GOTO ErrorHandler;
			END

			/**_# ?? ??? JobCode? ?????. */
			UPDATE dbo.JobChangeLogs
			   SET JobCode = @iny1stJobCodeAfter
				 , LogDate = @dt2NowDate
			 WHERE CharacterID = @inbCharacterID
			   AND JobCode = @iny1stJobCodeBefore;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue =103102;
				GOTO ErrorHandler;
			END

			UPDATE dbo.JobChangeLogs
			   SET JobCode = @iny2ndJobCodeAfter
				 , LogDate = @dt2NowDate
			 WHERE CharacterID = @inbCharacterID
			   AND JobCode = @iny2ndJobCodeBefore;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue =103102;
				GOTO ErrorHandler;
			END

		COMMIT TRANSACTION
		/**_# ?? ?? ??? JobCode? ?????. */
		UPDATE dbo.TeacherCharacters
		   SET JobCode = @iny2ndJobCodeAfter
		 WHERE TeacherCharacterID = @inbCharacterID
		   AND JobCode = @iny2ndJobCodeBefore;

		/**_# ?? ?? ??? ?????. */
		INSERT dbo.JobCodeChangeLogs (CharacterID, JobCodeBefore, JobCodeAfter, JobCodeChangeCode, ChangeDate)
		VALUES (@inbCharacterID, @iny1stJobCodeBefore, @iny1stJobCodeAfter, @inyJobCodeChangeCode, @dt2NowDate);

		INSERT dbo.JobCodeChangeLogs (CharacterID, JobCodeBefore, JobCodeAfter, JobCodeChangeCode, ChangeDate)
		VALUES (@inbCharacterID, @iny2ndJobCodeBefore, @iny2ndJobCodeAfter, @inyJobCodeChangeCode, @dt2NowDate);

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
