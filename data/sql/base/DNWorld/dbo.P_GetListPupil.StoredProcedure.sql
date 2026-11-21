USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPupil]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-26
description : ?? ??? ?????. (? ?? or ?? ??? ??)
result set : CharacterID/bigint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, FavorPoint/int, LastConnectDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5~6 = ?? ??? ?? ??? ?? ??? ?? UNION ALL ???? LEFT OUTER JOIN ???? ?? by ??? at 2011-11-14
**/

CREATE PROCEDURE [dbo].[P_GetListPupil]
	  @inbTeacherCharacterID bigint --// ??? ??? ID
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
	--SELECT CH.CharacterID, CH.CharacterName, CS.CharacterLevel, CS.JobCode, TnP.FavorPoint
	--	, CASE WHEN CS.LastLogoutDate > CS.LastLoginDate THEN CS.LastLogoutDate ELSE CS.LastLoginDate END AS LastConnectDate
	--FROM dbo.TeachersAndPupils TnP WITH (NOLOCK)
	--	INNER JOIN dbo.Characters CH WITH (NOLOCK) ON TnP.PupilCharacterID = CH.CharacterID
	--	INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
	--WHERE TnP.TeacherCharacterID = @inbTeacherCharacterID AND TnP.GraduateFlag = CAST(0 AS bit)
	--UNION ALL
	--SELECT CH.CharacterID, CH.CharacterName, CS.CharacterLevel, CS.JobCode, TnP.FavorPoint
	--	, CASE WHEN CS.LastLogoutDate > CS.LastLoginDate THEN CS.LastLogoutDate ELSE CS.LastLoginDate END AS LastConnectDate
	--FROM dbo.TeachersAndPupils TnP WITH (NOLOCK)
	--	INNER JOIN dbo.Characters CH WITH (NOLOCK) ON TnP.PupilCharacterID = CH.CharacterID
	--	INNER JOIN dbo.DormantCharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
	--WHERE TnP.TeacherCharacterID = @inbTeacherCharacterID AND TnP.GraduateFlag = CAST(0 AS bit);

	SELECT CH.CharacterID, CH.CharacterName, CS.CharacterLevel, CS.JobCode, TnP.FavorPoint, CASE WHEN CS.LastLogoutDate > CS.LastLoginDate THEN CS.LastLogoutDate ELSE CS.LastLoginDate END
	FROM dbo.TeachersAndPupils TnP WITH (NOLOCK)
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON TnP.PupilCharacterID = CH.CharacterID
		INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
	WHERE TnP.TeacherCharacterID = @inbTeacherCharacterID AND TnP.GraduateFlag = CAST(0 AS bit)

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
