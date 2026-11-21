USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMyTeacherAndClassmate]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-27
description : ? ?? & ?? ??? ?????.
result set : Kind/tinyint, CharacterID/bigint, CharacterName/nvarchar, LastConnectDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = ?? ??? ???? "????? ??"?? ?? ??? ?? ??? ????, ?? ??? ??? ? ??? ?? ??? ?????? ?? by ??? at 2011-11-01
5 = ?? ??? ?? ??? ?? ??? ?? LEFT OUTER JOIN ???? ?? by ??? at 2011-11-14
6 = DormantCharacterStatus ??? ?? ?? by ??? at 2011-11-30
7 = ??? ???? ?? by ??? at 2012-07-18
**/

CREATE PROCEDURE [dbo].[P_GetListMyTeacherAndClassmate]
	  @inbCharacterID bigint --// ??? ID
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
	;WITH C1 (Kind, CharacterID, CharacterName, RegisterDate, LastConnectDate) AS (
		SELECT CAST(1 AS tinyint) AS Kind, CH.CharacterID, CH.CharacterName, TnP.RegisterDate, CASE WHEN CS.LastLogoutDate > CS.LastLoginDate THEN CS.LastLogoutDate ELSE CS.LastLoginDate END
		FROM dbo.TeachersAndPupils TnP WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON TnP.TeacherCharacterID = CH.CharacterID
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
		WHERE TnP.PupilCharacterID = @inbCharacterID AND TnP.GraduateFlag = CAST(0 AS bit)
		UNION ALL
		SELECT CAST(2 AS tinyint), CH.CharacterID, CH.CharacterName, S.RegisterDate, CASE WHEN CS.LastLogoutDate > CS.LastLoginDate THEN CS.LastLogoutDate ELSE CS.LastLoginDate END
		FROM dbo.Characters CH WITH (NOLOCK)
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
			INNER JOIN (
				SELECT TnP2.PupilCharacterID, MIN(TnP2.RegisterDate) AS RegisterDate
				FROM dbo.TeachersAndPupils TnP1 WITH (NOLOCK)
					INNER JOIN dbo.TeachersAndPupils TnP2 WITH (NOLOCK) ON TnP1.TeacherCharacterID = TnP2.TeacherCharacterID
				WHERE TnP1.PupilCharacterID = @inbCharacterID
					AND TnP2.PupilCharacterID <> @inbCharacterID
					AND TnP2.GraduateFlag = CAST(0 AS bit)
					AND TnP1.GraduateFlag = CAST(0 AS bit)
				GROUP BY TnP2.PupilCharacterID
			) S ON S.PupilCharacterID = CH.CharacterID
	)
	SELECT Kind, CharacterID, CharacterName, LastConnectDate
	FROM C1
	ORDER BY Kind, RegisterDate;
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
