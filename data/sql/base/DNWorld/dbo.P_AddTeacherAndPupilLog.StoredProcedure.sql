USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTeacherAndPupilLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-27
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddTeacherAndPupilLog]
	  @dt2LogDate datetime2(0) --// ?? ??
	, @inbTeacherCharacterID bigint --// ?? ??? ID
	, @inbPupilCharacterID bigint --// ?? ??? ID
	, @inyTeacherCharacterJobCode tinyint --// 
	, @inyTeacherCharacterLevel tinyint --// ?? ??? ??
	, @inyPupilCharacterJobCode tinyint --// 
	, @inyPupilCharacterLevel tinyint --// ?? ??? ??
	, @intFavorPoint int --// ???
	, @insNewTeacherBlockPeriod smallint --// ? ??? ??? ? ?? ??, ??:day
	, @intPenaltyRespectPoint int --// ???? ??? ?? ??
	, @inyTeacherAndPupilLogCode tinyint --// 1=?? ?? ??, 2=??? ??, 3=??? ??, 4=??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();
DECLARE @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dt2Now);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [TeacherAndPupilLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.TeacherAndPupilLogs0 (LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode)
		VALUES (@dt2LogDate, @inbTeacherCharacterID, @inbPupilCharacterID, @inyTeacherCharacterJobCode, @inyTeacherCharacterLevel, @inyPupilCharacterJobCode, @inyPupilCharacterLevel, @intFavorPoint, @insNewTeacherBlockPeriod, @intPenaltyRespectPoint, @inyTeacherAndPupilLogCode);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.TeacherAndPupilLogs1 (LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode)
		VALUES (@dt2LogDate, @inbTeacherCharacterID, @inbPupilCharacterID, @inyTeacherCharacterJobCode, @inyTeacherCharacterLevel, @inyPupilCharacterJobCode, @inyPupilCharacterLevel, @intFavorPoint, @insNewTeacherBlockPeriod, @intPenaltyRespectPoint, @inyTeacherAndPupilLogCode);

	ELSE IF @intLogTableSuffix = 2
		INSERT dbo.TeacherAndPupilLogs2 (LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode)
		VALUES (@dt2LogDate, @inbTeacherCharacterID, @inbPupilCharacterID, @inyTeacherCharacterJobCode, @inyTeacherCharacterLevel, @inyPupilCharacterJobCode, @inyPupilCharacterLevel, @intFavorPoint, @insNewTeacherBlockPeriod, @intPenaltyRespectPoint, @inyTeacherAndPupilLogCode);
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
