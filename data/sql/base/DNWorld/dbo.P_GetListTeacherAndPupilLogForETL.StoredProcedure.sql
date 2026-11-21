USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTeacherAndPupilLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-14
description : ?? ?? ??? ?????.
result set : LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListTeacherAndPupilLogForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2BasisDate datetime2(0) = CAST(CAST(@dtmBasisDate AS date) AS datetime2(0)), @intLogTableSuffix int;;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2BasisDate);

	IF @intLogTableSuffix = 0
		SELECT LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode
		FROM dbo.TeacherAndPupilLogs0
		WHERE LogDate >= @dt2BasisDate AND LogDate < DATEADD(DAY, 1, @dt2BasisDate)
		OPTION (MAXDOP 1);

	ELSE IF @intLogTableSuffix = 1
		SELECT LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode
		FROM dbo.TeacherAndPupilLogs1
		WHERE LogDate >= @dt2BasisDate AND LogDate < DATEADD(DAY, 1, @dt2BasisDate)
		OPTION (MAXDOP 1);

	ELSE
		SELECT LogDate, TeacherCharacterID, PupilCharacterID, TeacherCharacterJobCode, TeacherCharacterLevel, PupilCharacterJobCode, PupilCharacterLevel, FavorPoint, NewTeacherBlockPeriod, PenaltyRespectPoint, TeacherAndPupilLogCode
		FROM dbo.TeacherAndPupilLogs2
		WHERE LogDate >= @dt2BasisDate AND LogDate < DATEADD(DAY, 1, @dt2BasisDate)
		OPTION (MAXDOP 1);

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
